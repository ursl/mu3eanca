//

#include "SiDet.h"

#include <mu3e/tools/idmap.h>

#include <mu3e/util/file.hpp>
#include <mu3e/util/rand.hpp>

#include "Mu3eConditions.hh"
#include "calPixelAlignment.hh"

#include <algorithm>
#include <fstream>

#include <boost/range/adaptors.hpp>

float SiDet::LAYER_L[SiDet::LAYER_N] {};
float SiDet::LAYER_R[SiDet::LAYER_N] {};

SiDet::Sensor::Sensor(uint32_t id_, const float3& v_, const float3& drow_, const float3& dcol_)
    : id(id_),  v(v_), drow(drow_), dcol(dcol_)
{
    float3 dir = drow / drow.rt(); dir.z = 0;
    float3 pca = v - dir * dot(dir, v);
    r = pca.rt();
    phi = pca.phi();
}

watson::Plane SiDet::Sensor::plane() const {
    return {
        toVector3(drow),
        toVector3(dcol),
        toVector3(v + drow * nrow / 2 + dcol * ncol / 2)
    };
}

SiDet::Hit::Hit(const Sensor& sensor, int row, int col, uint32_t timestamp_)
    : float3 ( sensor.v + sensor.drow * (0.5f + row) + sensor.dcol * (0.5f + col) )
    , sensor(sensor), row(row), col(col), timestamp{timestamp_, Conf::inst.si.n_fine_bits}
{
    // if surface deformations were read in different calculations are necessary
    if(sensor.deform.surface.maxCoeff() > FLT_EPSILON ||
       std::abs(sensor.deform.dTu - 1) > FLT_EPSILON ||
       std::abs(sensor.deform.dTv - 1) > FLT_EPSILON
    ) {
        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // TODO: Check this!!
        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

     
        // scale lengths by temperature parameters
        float scaled_width = sensor.width * sensor.deform.dTu;
        float scaled_length = sensor.length * sensor.deform.dTv;

        auto l0 = toVector3(sensor.drow);
        l0.normalize();
        auto l1 = toVector3(sensor.dcol);
        l1.normalize();
        watson::Vector3 origin = toVector3(sensor.v) + l0 * scaled_width/2 + l1 * scaled_length/2;

        watson::Vector2 local(sensor.pixelSize * (0.5f+row) - sensor.width/2, sensor.pixelSize * (0.5f+col) - sensor.length/2);
        // scale local hits by temperature parameters
        local(0) *= sensor.deform.dTu;
        local(1) *= sensor.deform.dTv;

        watson::Vector2 length(sensor.width, sensor.length);
        watson::Vector2 temperature(sensor.deform.dTu, sensor.deform.dTv);
        watson::LegendrePlane legendre_plane(l0, l1, origin, length, sensor.deform.surface, temperature);

        watson::Vector3 global = legendre_plane.to_global(local);

        x = global(0);
        y = global(1);
        z = global(2);
    }

    r = float3::rt();
    assert(r > 0);
    phi = float3::phi();
}

MCHit* SiDet::Hit::mc(int32_t tid_) {
    for(auto& mc : mcs) {
        if(mc.tid == tid_) return &mc;
    }
    return nullptr;
}

void SiDet::makeClusters() {
    // sort hits by sensor.id
    // to allows to break from hits loop
    // when 'hi->sensor.id != hj->sensor.id'
    std::sort(hits.begin(), hits.end(), [] (Hit* hi, Hit* hj) {
        assert(hi != nullptr && hj != nullptr);
        return hi->sensor.id < hj->sensor.id;
    });

    for(auto iti = hits.cbegin(), end = hits.cend(); iti != end; ++iti) {
        const auto& hi = *iti;
    for(auto itj = std::next(iti); itj != end; ++itj) {
        const auto& hj = *itj;

        if (hi->sensor.id.value() == 1) {
          std::cout << "sensor.id.value() = " << hi->sensor.id.value()
                    << " sensor.v.x = " << hi->sensor.v.x
                    << std::endl;
        }

        
        if(hi->sensor.id.value() != hj->sensor.id.value()) break;

        int drow = std::abs(hi->row - hj->row);
        int dcol = std::abs(hi->col - hj->col);
        // allow diagonally adjacent hits
        if(drow > 1 or dcol > 1) continue;

        if(!hi->cluster && !hj->cluster) {
            auto cluster = new Cluster;
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
        if(hi->cluster != hj->cluster) {
            hi->cluster->merge(hj->cluster);
            continue;
        }
    } }

    // remove empty clusters
    mu3e::util::erase_if(clusters, [] (Cluster* cluster) {
        if(cluster->hits.empty()) {
            delete cluster;
            return true;
        }
        return false;
    });

    // report number of clusters with same number of hits
    if(Conf::inst.verbose >= 1) {
        // map: n_hit => n_cluster
        std::map<size_t, int> nh2nc;
        for(auto& cluster : clusters) {
            nh2nc[cluster->hits.size()]++;
        }
        mu3e::log::debug("[SiDet::init] clusters:\n");
        for(auto& p : nh2nc) {
            mu3e::log::debug("  n_cluster(n_hit = %lu) = %d\n", p.first, p.second);
        }
    }

    if(Conf::inst.verbose >= 1) for(auto& cluster : clusters) {
        size_t nhit = 0;
        for(auto& hit : hits) if(hit->tid == cluster->tid && hit->hid == cluster->hid) nhit++;
        if(nhit != cluster->hits.size() || cluster->tid == 0 || cluster->hid == 0) {
            mu3e::log::warn(
                "[SiDet::makeClusters] cluster: nhit = %lu/%lu, tid = %d, hid = %d\n",
                cluster->hits.size(), nhit, cluster->tid, cluster->hid
            );
        }
    }

    // remove hits that are assigned to clusters
    mu3e::util::erase_if(hits, [] (Hit* hit) {
        if(hit->cluster) {
            return true;
        }
        return false;
    });

    // for each cluster create 'average' hit
    for(auto& c : clusters) {
        assert(c->hits.empty() == false);

        auto hit = new Hit(c->hits[0]->sensor, -1, -1, 0);
        hits.push_back(hit);
        // this reference to cluster will be used to delete hit
        hit->cluster = c;

        // calculate average position
        hit->x = hit->y = hit->z = 0;
        // and time
        float mc_time = 0;
        float mc_edep = 0;
        for(auto& h : c->hits) {
            hit->x += h->x;
            hit->y += h->y;
            hit->z += h->z;
            mc_time += h->mcs.front().time;
            mc_edep += h->mcs.front().edep;
        }
        hit->x /= c->hits.size();
        hit->y /= c->hits.size();
        hit->z /= c->hits.size();
        mc_time /= c->hits.size();
        hit->r = hit->rt();
        hit->phi = float3_phi(*hit);

        if(c->hits.size() > 3) {
            // assume that large clusters (more than 3 hits) are noise
            // TODO: add Conf parameter
            //continue;
        }

        hit->tid = c->tid;
        hit->hid = c->hid;
        // TODO: merge all mc hits
        hit->mcs.emplace_back(hit->tid, hit->hid, mc_time, mc_edep);
    }
}

//#define DEBUG_LO_HI

void SiDet::Layer::initLUTs() {
    /**
     * Example 1:
     * N = 8
     * hits_phi[] = { 5.0 } => idx = { 6 }
     *
     *             0  1  2  3  4  5  6  7
     * phi_lo[] = | 0| 0| 0| 0| 0| 0| 0|-1|
     * phi_hi[] = |-1|-1|-1|-1|-1|-1| 0| 0|
     *
     *
     *
     * Example 2:
     * N = 8
     * hits_phi[] = { 1.0, 2.0, 3.0, 5.0 } => idx = { 1, 2, 3, 6 }
     *
     *             0  1  2  3  4  5  6  7
     * phi_lo[] = | 0| 0| 1| 2| 3| 3| 3|-1|
     * phi_hi[] = |-1| 0| 1| 2| 2| 2| 3| 3|
     *
     *
     * phi_min = 0.5; phi_max = 2.5
     * idx_min = 0; idx_max = 3
     * phi_lo[0] = 0; phi_hi[3] = 2
     */
    lut_phi.init(hits, [] (Hit* hit) { return hit->phi; });

    lut_z.init(hits, [] (Hit* hit) { return hit->z; });
}

SiDet::Layer::lut_hits_t::range_t SiDet::Layer::range_phi(float phi_min, float phi_max) const {
    return lut_phi.range(phi_min, phi_max);
}

SiDet::Layer::lut_hits_t::range_t SiDet::Layer::range_z(float z_min, float z_max) const {
    return lut_z.range(z_min, z_max, false);
}

void SiDet::write_hits(std::ostream& file) const {
    for(auto& layer : layers) {
        uint32_t n = layer.hits.size();
        mu3e::file::write(file, &n);
        for(auto& hit: layer.hits) {
            float3& xyz = *hit;
            mu3e::file::write(file, &xyz);
        }
    }
}

SiDet::Hit* SiDet::add(uint32_t id, uint32_t timestamp) {
    mu3e::id::sensor::hit hitId(id);

    auto s_it = sensors.find(hitId.sensorId);
    if(s_it == sensors.end()) {
        mu3e::log::fatal("[SiDet::add] missing alignment for sensor 0x%04X\n", hitId.sensorId.value());
        exit(EXIT_FAILURE);
    }
    const Sensor& sensor = s_it->second;

    if(sensor.v.rt() < FLT_EPSILON) {
        mu3e::log::error("[SiDet::add] sensor_%u -> vx = %.3f, vy = %.3f, vz = %.3f\n", hitId.sensorId.value(), sensor.v.x, sensor.v.y, sensor.v.z);
        return nullptr;
    }

    Hit* hit = new Hit(sensor, hitId.row, hitId.col, timestamp);
    hits.push_back(hit);
    return hit;
}

void SiDet::generateNoise(float pixelNoiseFreq, float frameLength) {
    if(!(pixelNoiseFreq > 0)) return;

    for(const Sensor& sensor : sensors | boost::adaptors::map_values) {
        int n = CLHEP::RandPoisson::shoot(sensor.nrow * sensor.ncol * frameLength * pixelNoiseFreq / mu3e::units::second);
        for(int i = 0; i < n ; i++) {
            auto hit = new Hit(
                sensor,
                int(CLHEP::RandFlat::shootInt(sensor.nrow)),
                int(CLHEP::RandFlat::shootInt(sensor.ncol)),
                0
            );
            hit->mcs.emplace_back(-1, 0, frameLength * CLHEP::RandFlat::shoot(), 0);
            hit->tid = hit->mcs.front().tid;
            hit->hid = hit->mcs.front().hid;
            hits.push_back(hit);
        }
    }
}

void SiDet::init(const Sensor::map_t& sensors) {
    mu3e::log::info("[SiDet::init] init global vars\n");
    for(int i = 0; i < LAYER_N; i++) LAYER_R[i] =  LAYER_L[i] = 0;
    int LADDER_N[LAYER_N] {};

    for(const Sensor& sensor : sensors | boost::adaptors::map_values) {
        int layer = sensor.id.layer;
        assert(layer < LAYER_N);
        int ladder = sensor.id.ladder;
        assert(ladder < 32);

        LADDER_N[layer] = std::max(LADDER_N[layer], ladder + 1);
        LAYER_R[layer] = sensor.r;
        LAYER_L[layer] = std::max(LAYER_L[layer], sensor.v.z + sensor.drow.z * sensor.nrow + sensor.dcol.z * sensor.ncol);
    }

    for(int i = 0; i < LAYER_N; i++) {
        printf("  layer %d : %d ladders, r = %.1f, l = %.1f\n", i, LADDER_N[i], LAYER_R[i], LAYER_L[i]);
    }
}

#include "util/root.hpp"

void SiDet::readSensors(Sensor::map_t& sensors, TTree* tree) {
    if(!tree) {
        mu3e::log::warn("[SiDet::readSensors] No sensors alignment tree found (tree == NULL).\n");
        return;
    }

    if (1) {
      Mu3eConditions *pDC = Mu3eConditions::instance();
      std::cout << "Mu3eConditions with gt =  " << pDC->getGlobalTag()
                << " and db = " << pDC->getDB()->getName()
                << std::endl;
      pDC->printCalibrations();          
      calAbs *cal = pDC->getCalibration("pixelalignment_");
      std::cout << "cal = " << cal << std::endl;
      
      calPixelAlignment *cpa = dynamic_cast<calPixelAlignment*>(cal);
      uint32_t i(99999);
      std::cout << "HALLO SiDet::readSensors(...) cpa->getName() = " << cpa->getName() << std::endl;
      mu3e::root::alignment_sensors_t entry;
      int cnt(0); 
      while (cpa->getNextID(i)) {
        std::cout << "ID = " << i << " (cnt = " << cnt++ << ")" << std::endl;
        
        entry.id = cpa->id(i); 
        entry.vx = cpa->vx(i); 
        entry.vy = cpa->vy(i); 
        entry.vz = cpa->vz(i); 
        entry.rowx = cpa->rowx(i); 
        entry.rowy = cpa->rowy(i); 
        entry.rowz = cpa->rowz(i); 
        entry.colx = cpa->colx(i); 
        entry.coly = cpa->coly(i); 
        entry.colz = cpa->colz(i); 
        entry.nrow = cpa->nrow(i); 
        entry.ncol = cpa->ncol(i); 
        entry.width = cpa->width(i); 
        entry.length = cpa->length(i); 
        entry.thickness = cpa->thickness(i); 
        entry.pixelSize = cpa->pixelSize(i); 
        
        std::cout << " entry.id = " << entry.id << std::endl;

        auto& sensor = sensors.emplace(std::piecewise_construct,
                                       std::forward_as_tuple(entry.id),
                                       std::forward_as_tuple(entry.id,
                                                             make_float3(entry.vx, entry.vy, entry.vz),
                                                             make_float3(entry.rowx, entry.rowy, entry.rowz),
                                                             make_float3(entry.colx, entry.coly, entry.colz)
                                                             )
                                       ).first->second;
        
        sensor.nrow = entry.nrow; sensor.ncol = entry.ncol;
        sensor.width = entry.width; sensor.length = entry.length; sensor.thickness = entry.thickness;
        sensor.pixelSize = entry.pixelSize;
        
        watson::Matrix4 surface = watson::Matrix4::Zero();
        surface(2,0) = entry.c20; surface(2,1) = entry.c21; surface(2,2) = entry.c22;
        surface(3,0) = entry.c30; surface(3,1) = entry.c31; surface(3,2) = entry.c32; surface(3,3) = entry.c33;
        sensor.deform.surface = surface;
        sensor.deform.dTu = entry.dTu;
        sensor.deform.dTv = entry.dTv;
      }
    }
    
    if (0) {
      mu3e::root::alignment_sensors_t entry(tree);
      entry.set_branch_address();
      
      for(Long64_t i = 0, n = tree->GetEntries(); i < n; i++) {
        tree->GetEntry(i);

        std::cout << " entry.id = " << entry.id << std::endl;
        auto& sensor = sensors.emplace(std::piecewise_construct,
                                       std::forward_as_tuple(entry.id),
                                       std::forward_as_tuple(entry.id,
                                                             make_float3(entry.vx, entry.vy, entry.vz),
                                                             make_float3(entry.rowx, entry.rowy, entry.rowz),
                                                             make_float3(entry.colx, entry.coly, entry.colz)
                                                             )
                                       ).first->second;
        
        sensor.nrow = entry.nrow; sensor.ncol = entry.ncol;
        sensor.width = entry.width; sensor.length = entry.length; sensor.thickness = entry.thickness;
        sensor.pixelSize = entry.pixelSize;
        
        watson::Matrix4 surface = watson::Matrix4::Zero();
        surface(2,0) = entry.c20; surface(2,1) = entry.c21; surface(2,2) = entry.c22;
        surface(3,0) = entry.c30; surface(3,1) = entry.c31; surface(3,2) = entry.c32; surface(3,3) = entry.c33;
        sensor.deform.surface = surface;
        sensor.deform.dTu = entry.dTu;
        sensor.deform.dTv = entry.dTv;
      }
    }

}
