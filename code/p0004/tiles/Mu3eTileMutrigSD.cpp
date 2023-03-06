/*
 * Mu3eTileMutrigSD.cpp
 *
 */

#include "Mu3eTileMutrigSD.h"

#include "Mu3eEvent.h"
#include "Mu3eTrajectory.h"

#include "Mu3eFibreMppcSD.h"


#include "G4SDManager.hh"
#include "G4RunManager.hh"

#include <TFile.h>
#include <TH1F.h>

using namespace CLHEP;

Mu3eTileMutrigSD::Mu3eTileMutrigSD(
    const G4String& name,
    int _writeTiles
)
    : G4VSensitiveDetector(name)
    , write_mode(_writeTiles)
{
    if(!tileasicdose){
        tileasicdose = new TH1F("tileMutrigDose","Tile Mutrig Dose",191,0,191);
        tileasicdose->GetXaxis()->SetTitle("ASICID");
        tileasicdose->GetYaxis()->SetTitle("Dose [Gray]");
    }
    if(!tileasicnieldose){
        tileasicnieldose = new TH1F("tileMutrigNonionizingDose","Tile Mutrig Non-Ionizing Dose",191,0,191);
        tileasicnieldose->GetXaxis()->SetTitle("ASICID");
        tileasicnieldose->GetYaxis()->SetTitle("1 MeV neutron equivalents/cm^{2}");
    }

}

void Mu3eTileMutrigSD::Initialize(G4HCofThisEvent* HCE) {
}


G4bool Mu3eTileMutrigSD::ProcessHits(G4Step* aStep, G4TouchableHistory*) {
  const G4Event *evt = G4RunManager::GetRunManager()->GetCurrentEvent();
  int eventId(0);
  if (evt) {
    eventId = evt->GetEventID();
  } else {
    std::cout << " XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX no evt!!!!!!" << std::endl;
  }

  static int oldEventId(-1); 
  static std::map<int, double> eDepCounter;

  if(write_mode < 0) return false;

    auto track = aStep->GetTrack();
    auto info = Mu3eTrackInfo::trackInfo(track);

    // don't process neutrino hits
    if(Mu3eTrackInfo::getPType(info->pdg) == 7) return false;

    // hit edep
    double edep = aStep->GetTotalEnergyDeposit();
    if(!(edep > 0)) return false;

    auto prePoint = aStep->GetPreStepPoint();
    auto postPoint = aStep->GetPostStepPoint();

    // asic number
    int32_t asicNb = prePoint->GetTouchableHandle()->GetCopyNumber();// + prePoint->GetTouchableHandle()->GetCopyNumber(3);

    // Dose calculation...
    G4double cubicVolume = prePoint->GetPhysicalVolume()->GetLogicalVolume()->GetSolid()->GetCubicVolume();
    G4double density = prePoint->GetMaterial()->GetDensity();
    G4double dose_c    = edep/joule / ( density/kg * cubicVolume );
    dose_c *= prePoint->GetWeight();

    int index = asicNb;

    tileasicdose->Fill(index,dose_c);
    if (aStep->IsFirstStepInVolume()) { //  && abs(pid)==11
        // TODO different for Muons!
        G4double df = Mu3eFibreMppcSD::damageFunction(aStep->GetTrack()->GetKineticEnergy());
        tileasicnieldose->Fill(index, df);
    }

    if (fhmap.find(index) == fhmap.end()) {
      std::string shname = Form("h%d", index);
      std::string sename = Form("e%d", index);
      std::string setotname = Form("etot%d", index);
      std::string stitle("");
      auto hitPos = prePoint->GetPosition();
      stitle = shname + Form(" (%f/%f/%f)", hitPos.x(), hitPos.y(), hitPos.z()); 
      //      std::cout << "creating " << shname  << " for index = " << index << " with title = " << stitle << std::endl;
      fhmap.insert(std::make_pair(index, new TH1F(shname.c_str(), stitle.c_str(), 60, 0., 60.)));
      femap.insert(std::make_pair(index, new TH1F(sename.c_str(), stitle.c_str(), 100, 0., 2.)));
      fetotmap.insert(std::make_pair(index, new TH1F(setotname.c_str(), stitle.c_str(), 100, 0., 5.)));
      eDepCounter.insert(std::make_pair(index, 0.));
    } 
    
    if (prePoint->GetStepStatus() == fGeomBoundary) {
      //      std::cout << "filling index = " << index << std::endl;
      fhmap[index]->Fill(track->GetMomentum().mag());
    }
    femap[index]->Fill(edep);

    // -- write out old eDep accumulators
    if (eventId != oldEventId) {
      //      std::cout << "oldEventId = " << oldEventId << " eventId = " << eventId << std::endl;
      oldEventId = eventId;
      for (auto it: eDepCounter) {
        if (it.second > 0.0001) {
          std::cout << "writing   " << it.first << " with Edep = " << it.second << std::endl;
          fetotmap[it.first]->Fill(it.second);
        }
        eDepCounter[it.first] = 0.0;
      }
      eDepCounter[index] = edep;
      //      std::cout << "setting   " << index << " Edep = " << edep << " total = " << eDepCounter[index] << std::endl;
    }  else {
      eDepCounter[index] += edep;
      //      std::cout << "adding to " << index << " Edep = " << edep << " total = " << eDepCounter[index] << std::endl;
    }
    
    return true;
}

void Mu3eTileMutrigSD::EndOfEvent(G4HCofThisEvent*) {

}


void Mu3eTileMutrigSD::writeStat()
{
    TDirectory * d = (TDirectory*)gDirectory->Get("Tiledose");
    if(!d)
        gDirectory->mkdir("Tiledose");
    gDirectory->cd("Tiledose");
    tileasicdose->Write();
    tileasicnieldose->Write();

    for (auto it: fhmap) {
      //      std::cout << "write " << it.second->GetName() << std::endl;
      it.second->Write();
    }

    for (auto it: femap) {
      //      std::cout << "write " << it.second->GetName() << std::endl;
      it.second->Write();
    }

    for (auto it: fetotmap) {
      //      std::cout << "write " << it.second->GetName() << std::endl;
      it.second->Write();
    }

    gDirectory->cd("..");
}

TH1F * Mu3eTileMutrigSD::tileasicdose = nullptr;
TH1F * Mu3eTileMutrigSD::tileasicnieldose = nullptr;
