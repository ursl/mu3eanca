//

#include "Mu3eTileSD.h"

#include "Mu3eEvent.h"

#include "Mu3eFibreMppcSD.h"

#include <mu3e/util/container.hpp>

#include <TFile.h>
#include <TH1F.h>

Mu3eTileSD::Mu3eTileSD(
    const G4String& name,
    mu3e::sim::Tile* _tile,
    int _writeTiles
)
    : G4VSensitiveDetector(name)
    , tile(_tile)
    , write_mode(_writeTiles)
{
    collectionName.insert("tileCollection");

    tiledose = new TH1F("tileDose","Tile Dose",6000,-0.5,5999.5);
    tiledose->GetXaxis()->SetTitle("SensorID");
    tiledose->GetYaxis()->SetTitle("Dose [Gray]");

    tilenieldose = new TH1F("tileNonionizingDose","Tile Non-Ionizing Dose",6000,-0.5,5999.5);
    tilenieldose->GetXaxis()->SetTitle("SensorID");
    tilenieldose->GetYaxis()->SetTitle("1 MeV neutron equivalents/cm^{2}");
}

Mu3eTileSD::~Mu3eTileSD() {
    for(auto& hits : hitmap | boost::adaptors::map_values) {
        for(auto hit : hits) delete hit;
    }
    hitmap.clear();
    secondary_map.clear();
}

void Mu3eTileSD::Initialize(G4HCofThisEvent* HCE) {
    tileCollection = new Mu3eTileHit::collection_t(GetName(), GetCollectionName(0));

    assert(GetCollectionID(0) >= 0);
    HCE->AddHitsCollection(GetCollectionID(0), tileCollection);

//    hitmap.clear(); // if hitmap is cleared here events are not stored over frame borders!
//    secondary_map.clear();
}

G4bool Mu3eTileSD::ProcessHits(G4Step* aStep, G4TouchableHistory*) {
    if(!(write_mode > 0)) return false;

    auto track = aStep->GetTrack();
    auto info = Mu3eTrackInfo::trackInfo(track);

    // don't process neutrino hits
    if(Mu3eTrackInfo::getPType(info->pdg) == 7) {
        return false;
    }

    // update track info
    assert(info->tl_hid > 0);

    // hit edep
    double edep = aStep->GetTotalEnergyDeposit();
    if(!(edep > 0 || write_mode > 1)) return false;

    auto prePoint = aStep->GetPreStepPoint();
    auto postPoint = aStep->GetPostStepPoint();

    mu3e::id::tile tileId;
    {
        int copyNo = prePoint->GetTouchableHandle()->GetCopyNumber(0) + prePoint->GetTouchableHandle()->GetCopyNumber(1);
        auto& volume = tile->volumes[copyNo];
        if(volume.mother == nullptr) {
            mu3e::log::fatal("[TileSD::ProcessHits] unknown volume: copyNo = %d\n", copyNo);
            exit(EXIT_FAILURE);
        }
        tileId = volume.id;

        if(volume.copyNo != copyNo) {
            mu3e::log::fatal("[TileSD::ProcessHits] volume.copyNo (%d) != copyNo (%d)\n", volume.copyNo, copyNo);
            exit(EXIT_FAILURE);
        }
    }
    auto& tileHits = hitmap[tileId];

    int32_t hid = info->tl_hid;
    int32_t hid_g = info->si_hid;
    int32_t tid = info->tid;
    int32_t primary = tid;
    int32_t mid = info->mid;

    if(write_mode > 1) {
        // fill secondary map if first hit from a given tid
        if(track->GetLogicalVolumeAtVertex() == tile->logicalVolume && mid > 0) {
            auto parent = secondary_map.find(tid);
            if(parent == secondary_map.end()) {
                auto it = std::find_if(tileHits.begin(), tileHits.end(), [mid] (Mu3eTileHit* hit) {
                    return hit->mc0().tid == mid;
                });
                if(it == tileHits.end()) {
                    std::cout << "WARNING: Mu3eTileSD::ProcessHits: couldn't find primary (" << mid << ") of secondary " << tid << " in tile " << tileId.value() << std::endl;
                    if(Mu3eDigiCfg::Instance().verbose >= 3) info->print();
                }
                else {
                    secondary_map[tid] = *it;
                    //std::cout << "NOTE:    Mu3eTileSD::ProcessHits: found primary (" << mid << ") of secondary " << tid << " in tile " << tileId << std::endl;
                }
            }
            parent = secondary_map.find(tid);
            if(parent != secondary_map.end()) {
                hid += parent->second->mc0().hid - 1; // only fails if above WARNING is printed
            }
        }

        // find primary id, resolve all recursive

        while(1) {
            const auto& it = secondary_map.find(primary);
            if(it == secondary_map.end() || !it->second || primary == it->second->mc0().tid) {
                break; // no secondaries present
            }
            primary = it->second->mc0().tid;
        }

    }

    // multiple mc hits in one tile -> merge
    Mu3eTileHit* hit = nullptr;
    for(auto& hit_ : tileHits) {
        if(hit_->mc0().tid == tid && hit_->mc0().hid == hid) {
            hit = hit_;
            hit->mc0().edep += edep;
        }
    }

    if(!hit) { // new hit (tid, hid) combination
        hit = new Mu3eTileHit();
        hit->tileId = tileId.value();

        auto& mc = hit->mc0();
        mc.init(*aStep);
        mc.hid = hid;
        mc.hid_g = hid_g;
        hit->primary = primary;
        //mc.p_in = { prePoint->GetTotalEnergy(), prePoint->GetMomentum() };

        tileHits.push_back(hit);
    }

    // last tile hit
    if(postPoint->GetPhysicalVolume()->GetLogicalVolume() != tile->logicalVolume) {
        info->tl_hid += 1;
    }

    // Dose calculation...
    G4double cubicVolume = prePoint->GetPhysicalVolume()->GetLogicalVolume()->GetSolid()->GetCubicVolume();
    G4double density = prePoint->GetMaterial()->GetDensity();
    G4double dose_c    = edep/CLHEP::joule / ( density/CLHEP::kg * cubicVolume );
    dose_c *= prePoint->GetWeight();

    if (0) G4cout << "Tile   DEBUG density " << density
           << " rho/kg = " << density/CLHEP::kg
           << " cubicVol = " << cubicVolume
           << " w8 = " << (density/CLHEP::kg * cubicVolume)
           << " edep = " << edep << " edep/joule = " << edep/CLHEP::joule
           << " dose_c = " << dose_c
           << " w8' = " << prePoint->GetWeight()
           << G4endl;

    int index = 3000 * (tileId.station - 1) + tileId.slice + 56 * tileId.ring;

    tiledose->Fill(index, dose_c);
    if (aStep->IsFirstStepInVolume()) { //  && abs(pid)==11
        // TODO different for Muons!
        G4double df = Mu3eFibreMppcSD::damageFunction(aStep->GetTrack()->GetKineticEnergy());
        tilenieldose->Fill(index, df);
    }

    return true;
}

void Mu3eTileSD::EndOfEvent(G4HCofThisEvent*) {
    auto event = Mu3eEvent::GetInstance();

    for(auto& hits : hitmap | boost::adaptors::map_values) {
        hits.sort([] (Mu3eTileHit* l, Mu3eTileHit* r) {
//            if(l->mc0().hid < r->mc0().hid) return true;
            return l->mc0().time < r->mc0().time;
        });

        // merge hits
        Mu3eTileHit* hit = nullptr;
        for(auto it = hits.begin(); it != hits.end();) {
            auto hit_next = *it;

            if(hit == nullptr) {
                hit = hit_next;
                ++it;
                continue;
            }

            double mc_t = hit->mc0().time; // TODO: use mcs.back()
            double mc_edep = hit->mc_edep();
            double e_thr = tile->e_thresh;
            double t_prompt = tile->prompttime;
            double t_dead = tile->getDeadtime(hit->mc_edep());

            // merge within t_prompt
            // or within t_dead if edep > e_thr
            if((hit_next->mc0().time < mc_t + t_prompt) ||
               (mc_edep > e_thr && hit_next->mc0().time < mc_t + t_dead)
            ) {
                hit->merge(*hit_next);
                mu3e::util::erase_if(secondary_map, [hit_next] (auto it_) { return it_.second == hit_next; });
                delete hit_next;
                it = hits.erase(it);
                continue;
            }
            else {
                hit = hit_next;
                ++it;
            }
        }

        // process hits
        for(auto it = hits.begin(); it != hits.end();) {
            hit = *it;

            double mc_t = hit->mc0().time;
            double mc_edep = hit->mc_edep();
            double e_thr = tile->e_thresh;
            double t_dead = tile->getDeadtime(hit->mc_edep());

            // sensor response (digi)
            if(mc_edep > e_thr) {
                hit->edep = tile->getDigiEdep(mc_edep);
                hit->time = tile->getDigiTime(mc_t, mc_edep);
                hit->timestamp = tile->mutrig->getTimestamp(hit->time);

                if(mc_t + t_dead < event->startTime + event->frameLength) {
                    mu3e::util::erase_if(secondary_map, [hit] (auto it_) { return it_.second == hit; });
                    tileCollection->insert(hit);
                    it = hits.erase(it);
                    continue;
                }
            }

            // remove old hits
            if(mc_t < event->startTime - 16 * event->frameLength) {
                mu3e::util::erase_if(secondary_map, [hit] (auto it_) { return it_.second == hit; });
                delete hit;
                it = hits.erase(it);
                continue;
            }

            ++it;
        }
    }

    // remove empty tiles
    for(auto hits = hitmap.begin(); hits != hitmap.end();) {
        if(hits->second.empty()) {
            hits = hitmap.erase(hits);
        }
        else ++hits;
    }

    event->FillTileHits(*tileCollection);

    if(verboseLevel > 0) {
        size_t NbHits = tileCollection->entries();
        G4cout << "\n-------->Hits Collection: in this event there are " << NbHits
               << " hits in the tile sensors: " << G4endl;
        for(size_t i = 0; i < NbHits; i++) (*tileCollection)[i]->Print();
    }
}

void Mu3eTileSD::WriteAlignment(Mu3eTileSD* detector, TDirectory* file, const char* name) {
    if(!detector) {
        printf("WARN: Tile alignments '%s' not written (detector == NULL in Mu3eTileSD::WriteAlignment)\n", name);
        return;
    }

    if(!file) return;

    auto tree = file->Get<TTree>(name);
    if(!tree) {
        tree = new TTree(name, name);
        tree->SetDirectory(file);
    }

    auto tile = detector->tile;

    // TODO: remove
    int sensor;

    uint32_t id;
    double posx, posy, posz;
    double dirx, diry, dirz;

    tree->Branch("sensor", &sensor);
    tree->Branch("id", &id);
    tree->Branch("posx", &posx);
    tree->Branch("posy", &posy);
    tree->Branch("posz", &posz);

    tree->Branch("dirx", &dirx);
    tree->Branch("diry", &diry);
    tree->Branch("dirz", &dirz);

    const G4ThreeVector center_(0, 0, 0);
    const G4ThreeVector normal_(0, 0, 1);

    for(auto& volume : tile->volumes | boost::adaptors::map_values) {
        const auto& transform = volume.transform;

        const G4ThreeVector center = transform.TransformPoint(center_);
        const G4ThreeVector normal = transform.TransformPoint(normal_);
        const G4ThreeVector dir = normal - center;

        sensor = volume.id.value();
        id = volume.id.value();
        posx = center.x();// + tile->width / 2. * dir.x();
        posy = center.y();// + tile->width / 2. * dir.y();
        posz = center.z();// + tile->width / 2. * dir.z();

        dirx = dir.x();
        diry = dir.y();
        dirz = dir.z();

        tree->Fill();
    }
}

void Mu3eTileSD::writeStat() {
    gDirectory->mkdir("Tiledose");
    gDirectory->cd("Tiledose");
    tiledose->Write();
    tilenieldose->Write();
    gDirectory->cd("..");
}
