//

#include "FbDet.h"

#include "Segment.h"

#include "Conf.h"
#include "Util.h"

#include "Mu3eConditions.hh"
#include "calFibreAlignment.hh"
#include "calMppcAlignment.hh"

#include <algorithm>

#include <boost/range/adaptors.hpp>

float FbDet::R = 0;

FbDet::Hit::Hit(const Mppc& mppc_, uint32_t id_, uint32_t timestamp_)
    : mppc(mppc_)
    , id(id_)
    , timestamp{timestamp_ - Conf::inst.fb.delay, Conf::inst.fb.n_fine_bits}
{
    int readoutType = mppc.getReadoutType();

    if(readoutType == READOUT_SINGLE) {
        x = mppc.v.x;
        y = mppc.v.y;
    }
    else if(readoutType == READOUT_MPPC) {
        x = mppc.x(id.column); // assume not rotated/tilted fibres
        y = mppc.y(id.column);
    }
    else {
        // TODO: error
        exit(EXIT_FAILURE);
    }

    r = float2::rt();
    phi = float2::phi();
}

void FbDet::Hit::print() {
    printf(
        "FbHit: id = %X, ribbon = %d, col = %d, side = %d, timestamp = %u, tid = %d\n",
        id.value(), id.mppcId.ribbon, id.column, id.mppcId.side, timestamp.fine(), mcs.front().tid
    );
}

bool FbDet::Cluster::contains(Cluster* cluster) {
    if(hits.empty() || cluster->hits.empty()) return false;

    const float c_fibre = mu3e::units::c_light / Conf::inst.fb.refidx;
    const float l_fibre = Conf::inst.fb.length;

    // contains if at least one hit on the same side whould have been merged
    const float dt_min_ = std::abs(tmin() - cluster->tmin());
    const float dt_max_ = std::abs(tmax() - cluster->tmax());
    const float dt_ = std::min(dt_min_, dt_max_);
    if(dt_ > Conf::inst.fb.cluster_dt_max + l_fibre / c_fibre) return false;

    // contains if new cluster is still below cutoff
    //float tmin_ = std::min(tmin(), cluster->tmin());
    //float tmax_ = std::max(tmax(), cluster->tmax());
    //if((tmax_ - tmin_) > Conf::inst.fb.cluster_dt_max) return false;

    const float d_max = Conf::inst.fb.link_sides_tolerance;
    if(-d_max < mu3e::util::dphi(hit_phimin->phi, cluster->hit_phimax->phi) &&
                mu3e::util::dphi(hit_phimax->phi, cluster->hit_phimin->phi) < +d_max)
                return true;

    // instead use cluster center
    //if(-d_max < Util::dphi(hit_phimin->phi, cluster->phi) &&
    //            Util::dphi(hit_phimax->phi, cluster->phi) < +d_max)
    //            return true;

    if(hits.size() == 1 && std::abs(mu3e::util::dphi(cluster->phi, phi)) < 3 * Conf::inst.fb.link_sides_tolerance) { // Nicolas: tested, concluded to use 3
        return true;
    }

    return false;
}

void FbDet::Cluster::add(Hit* hit) {
    hits.push_back(hit);
    hit->cluster = this;

    const size_t n = hits.size();

    if(n == 1) side = hit->id.mppcId.side;

    if(side != hit->id.mppcId.side) {
        mu3e::log::fatal("[FbDet::Cluster::add] BUG: cluster.side != hit.side");
        exit(EXIT_FAILURE);
    }

    auto& s = hit->id.mppcId.side > 0 ? right : left;

    float t = hit->t();
    if(t < s.t_min) {
        s.t_min = t;
        s.t_mc = hit->mcs.front().time;
    }
    if(t > s.t_max) {
        s.t_max = t;
    }
    s.size += 1;

    tm1 += t;
    t = tm1 / n;

    xm1 += hit->x;
    ym1 += hit->y;
    x = xm1 / n;
    y = ym1 / n;
    r = float2::rt();
    phi = float2::phi();

    if(n == 1) tid = hit->mcs.front().tid;
    if(hit->mcs.front().tid < 0) tid = -1; // tid is -1 if one of the hits is -1, even if correct one is in cluster
    if(tid != hit->mcs.front().tid && tid >= 0) tid = 0;

    if(!hit_phimin || mu3e::util::dphi(hit_phimin->phi, hit->phi) < 0) hit_phimin = hit;
    if(!hit_phimax || mu3e::util::dphi(hit_phimax->phi, hit->phi) > 0) hit_phimax = hit;
}

void FbDet::Cluster::merge(Cluster* cluster) {
    assert(cluster != this);
    for(auto& hit : cluster->hits) add(hit);
    cluster->reset();
}

void FbDet::Cluster::print() {
    printf(
        "FbCluster: phi_min = %.3f, phi_max %.3f, tid = %d, t_min = %f\n",
        hit_phimin->phi, hit_phimax->phi, tid, tmin()
    );

    printf("  hits = {\n");
    for(auto& h : hits) { printf("    "); h->print(); }
    printf("  }\n");

    printf("  tids = { ");
    for(auto& tid_ : getTids()) printf("%i , ", tid_);
    printf(" }\n");
}

void FbDet::Cluster::reset() {
    hits.clear();
//    segs.clear();

    left.reset();
    right.reset();

    tm1 = t = 0;
    xm1 = ym1 = x = y = 0;
    r = phi = 0;
    tid = 0;

    hit_phimin = hit_phimax = nullptr;
}

bool FbDet::Cluster::containsTid(int32_t tid_) const {
    if(tid_ <= 0) return false;
    for(const auto& hit : hits) {
        for(auto& mc : hit->mcs) if(mc.tid == tid_) return true;
    }
    return false;
}

bool FbDet::Cluster::containsTidHid(int32_t tid_, int32_t hid_) const {
    if(tid_ <= 0) return false;
    for(const auto& hit : hits) {
        for(auto& mc : hit->mcs) {
            if(mc.tid == tid_ && mc.hid_g == hid_) return true;
        }
    }
    return false;
}

std::tuple<size_t, size_t, bool, bool> FbDet::Cluster::getNumIds(int32_t tid_, int32_t hid_) const {
    size_t ntids = 0;
    bool tid_ok = false, hid_ok = false;
    std::map<int32_t, std::set<int32_t>> tid2hids;
    for(const auto& hit : hits) {
        for(const auto& mc : hit->mcs) {
            auto& hids = tid2hids[mc.tid];
            hids.insert(mc.hid);
            if(hids.size() > ntids) ntids = hids.size();
            if(mc.tid == tid_) {
                tid_ok = true;
                if(mc.hid_g == hid_) hid_ok = true;
            }
        }
    }
    return std::tuple<size_t, size_t, bool, bool> { tid2hids.size(), ntids, tid_ok, hid_ok };
}

std::tuple<size_t, size_t> FbDet::Cluster::getNumIds() const {
    size_t ntid, nhid;
    std::tie(ntid, nhid, std::ignore, std::ignore) = FbDet::Cluster::getNumIds(0, 0);
    return std::tuple<size_t, size_t> { ntid, nhid };
}

FbDet::FbDet(const Fibre::map_t& fibres_, const Mppc::map_t& mppcs_)
    : fibres(fibres_)
    , mppcs(mppcs_)
    , surface(R)
{
    hits.reserve(512);

    // get readout type
    if(!mppcs.empty()) {
        READOUT = mppcs.begin()->second.getReadoutType();
    }

    switch(READOUT) {
    case READOUT_MPPC : {
        for(const Mppc& mppc : mppcs | boost::adaptors::map_values ) {
            // TODO: make ribbons from fibres
            if(mppc.id.side > 0) {
                float3 dirz = { 0, 0, -1 };
                auto norm = cross(mppc.dcol, dirz);
                norm = norm / norm.r();
                auto pos = norm * dot(norm, mppc.v);
                ribbons.emplace(mppc.id.ribbon, watson::Plane::Curvilinear(toVector3(norm), toVector3(pos)));
            }
        }
        break;
    }
    case READOUT_SINGLE : {
        std::map<int, float3> posf3; // <ribbon N, pos> pos fibre 3 of layer 0 in ribbon N
        std::map<int, float3> posf4; // <ribbon N, pos> pos fibre 4 of layer 0 in ribbon N
        for(const Mppc& mppc : mppcs | boost::adaptors::map_values ) {
            if(mppc.id.side > 0) {
                int ribbon = mppc.id.ribbon;
                int layer = mppc.id.layer;
                int fibre = mppc.id.fb;
                if((fibre == 3) && (layer == 0)) posf3[ribbon] = mppc.v;
                if((fibre == 4) && (layer == 0)) posf4[ribbon] = mppc.v;
            }
        }
        for(const auto& posm : posf3) {
            auto ribbon = posm.first;
            auto pos = posm.second;
            auto dir = posf4[ribbon] - pos;
            auto norm = cross(dir, {0, 0, -1});
            ribbons.emplace(ribbon, watson::Plane::Curvilinear(toVector3(norm), toVector3(pos)));
        }
        break;
    }
    default:
        abort();
    }
}

void FbDet::makeFibreClusters() {
    if(hits.empty()) return;

    std::sort(hits.begin(), hits.end(),
        [] (Hit* hi, Hit* hj)
        { return hi->time < hj->time; }
    );

    for(auto iti = hits.cbegin(), end = hits.cend(); iti != end; ++iti) {
        const auto& hi = *iti;
    for(auto itj = std::next(iti); itj != end; ++itj) {
        const auto& hj = *itj;

        float dt = hj->time - hi->time; // hj->time > hi->time (sorted)
        if(dt > 2.0f) break;

        if(std::abs(hi->id.mppcId.layer - hj->id.mppcId.layer) > 1) continue;
        if(std::abs(mu3e::util::dphi(hi->phi, hj->phi)) > 2 * Conf::inst.fb.diameter / R) continue;

        if(!hi->cluster && !hj->cluster) {
            auto cluster = new Cluster();
            cluster->add(hi);
            cluster->add(hj);
            clusters.push_back(cluster);
            continue;
        }
        if(hi->cluster == hj->cluster) continue;
        if(hi->cluster && !hj->cluster) {
            hi->cluster->add(hj);
            continue;
        }
        if(!hi->cluster && hj->cluster) {
            hj->cluster->add(hi);
            continue;
        }
        if(hi->cluster && hj->cluster) {
            hi->cluster->merge(hj->cluster);
            continue;
        }
    } }

    for(auto it = clusters.begin(); it != clusters.end();) {
        if((*it)->hits.empty()) {
            delete *it;
            it = clusters.erase(it);
            continue;
        }
        ++it;
    }

    for(auto& h : hits) if(!h->cluster) {
        auto cluster = new Cluster();
        cluster->add(h);
        clusters.push_back(cluster);
    }
}

void FbDet::makeClusters() {
    if(hits.empty()) return;

    // sort hits in order of ribbon, then side, then col, then time
    std::sort(hits.begin(), hits.end(), [] (Hit* hi, Hit* hj) {
        if(hi->id.mppcId.ribbon != hj->id.mppcId.ribbon) return hi->id.mppcId.ribbon < hj->id.mppcId.ribbon;
        if(hi->id.mppcId.side != hj->id.mppcId.side) return hi->id.mppcId.side < hj->id.mppcId.side;
        if(hi->id.column != hj->id.column) return hi->id.column < hj->id.column;
        return hi->timestamp.fine() < hj->timestamp.fine();
    });

    // add first hit to a new Cluster
    auto cluster = new Cluster();
    cluster->add(hits.front());
    clusters.push_back(cluster);

    // now go through all hits either add new clusters or add them to the previous one
    for(auto iti = hits.begin(), itj = iti + 1, end = hits.end(); itj != end; ++itj) {
        Hit* hi = *iti;
        Hit* hj = *itj;

        float dt = std::max(std::abs(hj->t() - hi->cluster->tmin()),
                            std::abs(hj->t() - hi->cluster->tmax()));

        if(hi->id.mppcId.value() == hj->id.mppcId.value() &&
           dt <= Conf::inst.fb.cluster_dt_max &&
           hj->id.column - hi->id.column == 1
        ) {
            hi->cluster->add(hj);
            iti = itj;
        }
        else {
            auto cluster = new Cluster();
            cluster->add(hj);
            clusters.push_back(cluster);
            iti = itj;
        }
    }
}

void FbDet::matchClusterSides() {
    for(auto iti = clusters.begin(), itj_start = clusters.begin(); iti != clusters.end(); ++iti) {
        auto& ci = *iti;
        if((*itj_start)->hits[0]->id.mppcId.ribbon != ci->hits[0]->id.mppcId.ribbon) itj_start = iti;
        if(ci->side == 1) continue;
    for(auto itj = itj_start; itj != clusters.end(); ++itj) {
        if(iti == itj) continue;

        auto& cj = *itj;
        if(cj->hits[0]->id.mppcId.ribbon != ci->hits[0]->id.mppcId.ribbon) break;
        if(cj->side == 0) continue;

        if(ci->contains(cj) || cj->contains(ci)) ci->merge(cj);
    } } // for(iti) for(itj)

    // delete empty (merged) clusters
    for(auto it = clusters.begin(); it != clusters.end();) {
        if((*it)->hits.empty()) {
            delete *it;
            it = clusters.erase(it);
            continue;
        }
        ++it;
    }
}

/*
void FbDet::makeReclustering() {
    Cluster::list_t clusters_;

    typedef typename FbDet::Hit::vector_t::const_reference hit_ref_t;
    typedef typename FbDet::Hit::vector_t::iterator Hit_it;

    int debugCounter=0;
    // needs to be here because clusters.end() gets modified in loop
    const auto end_clusters = clusters.cend();
    for(auto clusterIt = clusters.begin();clusterIt!=end_clusters;clusterIt++) {
        auto& cluster = *clusterIt;

        float tmin = cluster->tmin_min();
        float tmax = cluster->tmax_max();

        //printf("\ndebugCounter: %i \n ",debugCounter);
        //just consider cluster having a time span bigger than recl_dtime, all others we leave as they are
        if(tmax - tmin < Conf::inst.fb.cluster_recl_dt) continue;

        //just conside cluster matched to at least one track
        if(cluster->segs.empty()) continue;

        // small clusters do often have bad timing.. we skip clusters being to smalle (1 1 match)
        if(cluster->hits.size() <= 2 && cluster->side == 2) continue;

        //sort all the clusters to have smallest cols first
        std::sort(cluster->hits.begin(), cluster->hits.end(), [] (Hit* l, Hit* r) {
            return l->col < r->col;
        });

        //(*clusterIt)->print();

        // define two borderlists (they contain beginning and end of new clusters)
        std::list<Hit_it> hitListBegin;
        std::list<Hit_it> hitListEnd;

        // find initial borders (of and add them to the borderlists
        hitListBegin.push_back((*clusterIt)->hits.begin());
        hitListEnd.push_back((*clusterIt)->hits.end());


        auto it_beg = hitListBegin.begin();
        auto it_end = hitListEnd.begin();
        //printf("\nfirst element: ");
        //(**it_beg)->print();
        //printf("\nlast element: ");
        //(*((*it_end)-1))->print();
        // find min element then search on left and right,
        // if counter is bigger than borderlist, break;
        while(it_beg!=hitListBegin.end()) {
            Hit_it it_minElement = std::min_element(*it_beg,*it_end,
                    [] (hit_ref_t hi, hit_ref_t hj)
                    {return hi->time < hj->time;}
            );
            //printf("-------------------------------------------------------------------------------");
            //printf("\nmin_element %d, ",(*it_minElement)->tid);
            //(*it_minElement)->print();

            //search for the left side border &save border, ignore single clusters
            for(auto it = it_minElement;(it+1)!=(*it_beg);it--) {
                if(std::abs((*it_minElement)->time-(*it)->time)<Conf::inst.fb.cluster_recl_dt) continue;
                // check on single clusters and ignore
                if(it!=(*it_beg)&&std::abs((*it_minElement)->time-(*(it-1))->time)<Conf::inst.fb.cluster_recl_dt) continue;

                hitListBegin.push_back(*it_beg);
                hitListEnd.push_back(it+1);

                //printf("\n left first element: ");
                //(**it_beg)->print();
                //printf("\n left last element: ");
                //(*it)->print();

                (*it_beg)=it+1;
                break;

            }
            //search for the right side border & save borders, ignore single clusters
            for(auto it = it_minElement;it!=(*(it_end));it++) {

                if(std::abs((*it_minElement)->time-(*it)->time)<Conf::inst.fb.cluster_recl_dt) continue;
                //check on single clusters and ignore
                if((it+1)!=(*it_end)&&std::abs((*it_minElement)->time-(*(it+1))->time)<Conf::inst.fb.cluster_recl_dt) continue;

                hitListBegin.push_back(it);
                hitListEnd.push_back(*it_end);

                //printf("\n right first element: ");
                //(*it)->print();
                //printf("\n right last element: ");
                //(*(*it_end-1))->print();

                (*it_end)=it;
                break;

            }
            // TODO: Throw out single Elements
            // go to next element in list
            it_beg++;it_end++;
        }

        //printf("-------------------------------------------------------------------------------");
        //printf("\n s1: %u, s2: %u\n ",hitListBegin.size(),hitListEnd.size());
        //now go though hitLists and add new Clusters and delete the old one
        it_beg =hitListBegin.begin();it_end =hitListEnd.begin();

        int i = 0;



        while(it_beg!=hitListBegin.end()) {

            FbDet::Cluster* cluster= new FbDet::Cluster();
            for(auto it=(*it_beg);it!=(*it_end);it++) {
                cluster->add((*it));
            }
            // add cluster to clusters list

            it_beg++;it_end++;i++;
            // single hit clusters are thrown out
            if(it_beg==it_end)continue;
            //right now, only cluster being matched together are counted
            if(cluster->side!=2) continue;
            // add cluster to clusters list
            clusters_.push_back(cluster);
            //printf("->->");
            // cluster->print();


        }
        //printf(" end new clusters-------------------------------------------------------------------------------");

        //match every segment matched to old cluster to one new one
        for(auto s:(*clusterIt)->segs) {
            //printf("\ns->fbs.size() 1, %i",s->fbs.size());
            //erase reference to old cluster in segment
            for(auto fb_clusterIt = s->fbs.begin();fb_clusterIt !=s->fbs.end();fb_clusterIt++)
                if(((*fb_clusterIt)->cluster)==*clusterIt) fb_clusterIt =s->fbs.erase(fb_clusterIt);

            //printf("\ns->fbs.size() 2, %i",s->fbs.size());

            //TODO: add path..
            if(clusters_.size()==0)continue;

            //find new cluster working for it
            float d_min = FLT_MAX;
            float z_min = FLT_MAX;
            float phi_min = FLT_MAX;
            FbDet::Cluster::list_t::iterator newClusterMinIt;
            for(auto newClusterIt = clusters_.begin();newClusterIt!=clusters_.cend();newClusterIt++) {

                float phi_, z_;
                std::tie(phi_, z_, std::ignore) = s->est12(1, +s->hits[1][1]->r, +(*newClusterIt)->r);
                phi_ += s->hits[1][1]->phi; z_ += s->hits[1][1]->z;

                const float d_ = Util::dphi(phi_, (*newClusterIt)->phi) * (*newClusterIt)->r;
                if(std::abs(d_)<std::abs(d_min)) {
                    d_min=d_;
                    z_min=z_;
                    phi_min=phi_;
                    newClusterMinIt=newClusterIt;
                }
            }
            auto fb_clust = std::make_shared<Segment::fb_t>();
            fb_clust->cluster = (*newClusterMinIt);
            fb_clust->d = d_min;
            fb_clust->z = z_min;
            fb_clust->phi = phi_min;
            s->fbs.push_back(fb_clust);
            //printf("\ns->fbs.size() 3, %i",s->fbs.size());
            // if(s->isMC()&&s->tid()==s->getMinFbCluster()->cluster->tid) {
            //   printf("\n OKKK\n");
            //   printf("\ns->tid %u\n",s->tid());
            //   printf("\ncluster->tid %u\n",s->getMinFbCluster()->cluster->tid);
            //
            // }
            // else{
            //   printf("\n WRONG: \n");
            //   printf("\ns->tid %u\n",s->tid());
            //   printf("\ncluster->tid %u\n",s->getMinFbCluster()->cluster->tid);
            // }
            (*newClusterMinIt)->segs.push_back(s);
            // go through every cluster
            // calculate norm
        }
        // get rid of old cluster
        if(i>0) clusterIt = clusters.erase(clusterIt);
        debugCounter++;
    }

    clusters.merge(clusters_);

    for(auto& cluster : clusters) {
        std::sort(cluster->tids.begin(), cluster->tids.end(),
        [] (const Cluster::tid2time_t& ti, const Cluster::tid2time_t& tj) {
            return ti.second < tj.second;
        });
    }
}*/

FbDet::Hit* FbDet::add(uint32_t id, uint32_t timestamp) {
    // array: mppc (5bits), col (7bits), side (1bit)
    // single fibre: mppc (14) [ribbon(5), layer(2), fibre(7)] col (7bits), side (1bit)
    mu3e::id::mppc::hit hitId(id);

    auto mppc_it = mppcs.find(hitId.mppcId.value());
    if(mppc_it == mppcs.end()) {
        mu3e::log::fatal("[FbDet::add] missing alignment for mppc 0x%08X\n", hitId.mppcId.value());
        exit(EXIT_FAILURE);
    }
    const Mppc& mppc = mppc_it->second;

    Hit* hit = new Hit(mppc, id, timestamp);
    hits.push_back(hit);
    return hit;
}

float2 FbDet::getMppcsNeighbourPos(Hit* hit, bool isLeft) {
    const Mppc& mppc = hit->mppc;
    float tol = Conf::inst.fb.link_col_tollerance;
    float x = isLeft ? mppc.x(hit->id.column + tol) : mppc.x(hit->id.column - tol); // 0.75, was empirically Tested, and seemed to be the best result
    float y = isLeft ? mppc.y(hit->id.column + tol) : mppc.y(hit->id.column - tol); // 0.75, was empirically Tested, and seemed to be the best result
    return { x, y };
}

void FbDet::init(const Mppc::map_t& mppcs) {
    mu3e::log::info("[FbDet::init] init global vars\n");

    // get readout type
    auto readoutType = READOUT_MPPC;
    if(!mppcs.empty()) {
        readoutType = mppcs.begin()->second.getReadoutType();
    }
    if(readoutType == READOUT_SINGLE) {
        printf("  Single fibre readout\n");
        printf("  %lu fibres per side, ", mppcs.size()/2);
    }
    else {
        printf("  Array readout\n");
        printf("  %lu mppcs per side", mppcs.size()/2);
    }

    // FIXME: in single fibre readout some branches are missing in the tree
    for(const Mppc& mppc : mppcs | boost::adaptors::map_values) {
        assert(mppc.getReadoutType() == readoutType);
        R += mppc.v.rt();
    }
    R /= mppcs.size();

    if(Conf::inst.rec_fb == 2) printf(", waveforms");
    printf("\n");

    printf("  r = %.1f mm\n", R);
}

#include <TTree.h>

void FbDet::readFibres(Fibre::map_t& fibres, TTree* tree) {
  if (!mu3e::rec::conf.conddb.useCDB) {
    if(!tree) {
      mu3e::log::warn("[FbDet::readFibres] No fibres alignment tree found (tree == NULL)\n");
      return;
    }
      
    uint32_t id;
    double3 v, f;
      
    tree->SetBranchAddress("fibre", &id);
    tree->SetBranchAddress("cx", &v.x);
    tree->SetBranchAddress("cy", &v.y);
    tree->SetBranchAddress("cz", &v.z);
    tree->SetBranchAddress("fx", &f.x);
    tree->SetBranchAddress("fy", &f.y);
    tree->SetBranchAddress("fz", &f.z);
      
    for(Long64_t n = tree->GetEntries(), entry = 0; entry < n; entry++) {
      tree->GetEntry(entry);
      auto& fibre = fibres[id];
      fibre.id = mu3e::id::fibre(id);
      fibre.v = make_float3(v);
      fibre.f = make_float3(f);
    }
  } else {
    Mu3eConditions *pDC = Mu3eConditions::instance();
    if (pDC->getDB()) {
      calAbs *cal = pDC->getCalibration("fibrealignment_");
      std::cout << "filling FbDet from CDB with fibrealignment_ cal = " << cal << std::endl;
      
      calFibreAlignment *cfa = dynamic_cast<calFibreAlignment*>(cal);
      uint32_t i(99999);
      int cnt(0); 
      unsigned int id;
      double cx, cy, cz, fx, fy, fz;
      cfa->resetIterator();
      while (cfa->getNextID(i)) {
        // std::cout << "FbDet ID = " << i << " (cnt = " << cnt++ << ")" << std::endl;
        id = cfa->id(i); 
        cx = cfa->cx(i); 
        cy = cfa->cy(i); 
        cz = cfa->cz(i); 
        fx = cfa->fx(i); 
        fy = cfa->fy(i); 
        fz = cfa->fz(i); 
        auto& fibre = fibres[id];
        fibre.id = mu3e::id::fibre(id);
        fibre.v = make_float3(cx, cy, cz);
        fibre.f = make_float3(fx, fy, fz);
      }
    } else {
      std::cout << "You are lost: running with CDB, but no database instance defined. There will be a crash"
                << std::endl;
    }
  }
}


void FbDet::readMppcs(Mppc::map_t& mppcs, TTree* tree) {
  if (!mu3e::rec::conf.conddb.useCDB) {
    if(!tree) {
        mu3e::log::warn("[FbDet::readMppcs] No mppcs alignment tree found (tree == NULL)\n");
        return;
    }

    uint32_t id;
    double3 v, dcol;

    tree->SetBranchAddress("mppc", &id);
    tree->SetBranchAddress("vx",   &v.x);
    tree->SetBranchAddress("vy",   &v.y);
    tree->SetBranchAddress("vz",   &v.z);
    tree->SetBranchAddress("colx", &dcol.x);
    tree->SetBranchAddress("coly", &dcol.y);
    tree->SetBranchAddress("colz", &dcol.z);

    for(Long64_t n = tree->GetEntries(), entry = 0; entry < n; entry++) {
        tree->GetEntry(entry);
        auto& mppc = mppcs[id];
        mppc.id = mu3e::id::mppc(id);
        mppc.v = make_float3(v);
        mppc.dcol = make_float3(dcol);
    }
  } else {
    Mu3eConditions *pDC = Mu3eConditions::instance();
    if (pDC->getDB()) {
      calAbs *cal = pDC->getCalibration("mppcalignment_");
      std::cout << "filling MPPC in FbDet from CDB with mppcalignment_ cal = " << cal << std::endl;
      
      calMppcAlignment *cma = dynamic_cast<calMppcAlignment*>(cal);
      uint32_t i(99999);
      unsigned int id;
      double3 v, dcol;
      cma->resetIterator();
      while (cma->getNextID(i)) {
        id = cma->mppc(i); 
        v.x = cma->vx(id); 
        v.y = cma->vy(id); 
        v.z = cma->vz(id); 
        dcol.x = cma->colx(id); 
        dcol.y = cma->coly(id); 
        dcol.z = cma->colz(id); 
        // std::cout << "FbDet::Mppcs db ID = " << id << " (cnt = " << cnt++ << ")"
        // << " v = " << v.x << "/" << v.y << "/" << v.z
        // << " col = " << dcol.x << "/" << dcol.y << "/" << dcol.z
        // << std::endl;
        auto& mppc = mppcs[id];
        mppc.id = mu3e::id::mppc(id);
        mppc.v = make_float3(v);
        mppc.dcol = make_float3(dcol);
      }
    } else {
      std::cout << "You are lost: running with CDB, but no database instance defined. There will be a crash"
                << std::endl;
    }
  }
}
