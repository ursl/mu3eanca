//

#include "Mu3eFibreSmbMuTrigSD.h"

#include "Mu3eEvent.h"

#include <TFile.h>
#include <TH1.h>
#include <TMath.h>
#include <TString.h>
#include <TTree.h>

#include "G4SDManager.hh"

using namespace CLHEP;
using namespace std;

// ----------------------------------------------------------------------
Mu3eFibreSmbMuTrigSD::Mu3eFibreSmbMuTrigSD(const G4String& name)
    : G4VSensitiveDetector(name)
{

    int nbins(1000);
    double zmin(100.), zmax(200.);

    if(fFillDbxHist) {
        fMuTrigEdep = new TH1F("MuTrigEdep", "MuTrigEdep", 100, 0., 1.);
        fMuTrigEdepCombined = new TH1F("MuTrigEdepCombined", "MuTrigEdepCombined", 100, 0., 1.);

        fSmbMuTrigPosZ = new TH1F("SmbMuTrigPosZ", "SMB MuTrig hits, |z| global position for positive z", 3000, 120., 150.);
        fSmbMuTrigNegZ = new TH1F("SmbMuTrigNegZ", "SMB MuTrighits, |z| global position for negative z", 3000, 120., 150.);

        fSmbMuTrigPosZ_xy = new TH2F("fSmbMuTrigPosZ_xy", "SMB MuTrighits, xy global position for positive z", 200, -100., 100., 200, -100., 100.);
        fSmbMuTrigNegZ_xy = new TH2F("fSmbMuTrigNegZ_xy", "SMB MuTrighits, xy global position for negative z", 200, -100., 100., 200, -100., 100.);

        fSmbMuTrigPlanePosZ = new TH2F("SmbMuTrigPlanePosZ", "Smb MuTrig ID for z > 0 (DS)", 400, -200., 200., 400, -200., 200.);
        fSmbMuTrigPlaneNegZ = new TH2F("SmbMuTrigPlaneNegZ", "Smb MuTrig ID for z < 0 (US)", 400, -200., 200., 400, -200., 200.);

        for(int ix = 1; ix <= fSmbMuTrigPlanePosZ->GetNbinsX(); ++ix) {
            for(int iy = 1; iy <= fSmbMuTrigPlanePosZ->GetNbinsY(); ++iy) {
                fSmbMuTrigPlanePosZ->SetBinContent(ix, iy, -2.);
                fSmbMuTrigPlaneNegZ->SetBinContent(ix, iy, -2.);
            }
        }
    }

    // -- per-ASIC dose counter and  histograms
    for(unsigned int iz = 0; iz < 2; ++iz) {
        string sz("US");
        if(1 == iz) sz = string("DS");
        for(unsigned int ismb = 0; ismb < 12; ++ismb) {
            for(unsigned int iasic = 0; iasic < 4; ++iasic) {
                // -- names for ASIC histograms
                string sn = Form("%s_SMB%d_ASIC%d", sz.c_str(), ismb, iasic);

                // -- counter for deposited energy
                fDose.insert(make_pair(sn, 0.));
                fDose2.insert(make_pair(sn, 0.));

                if(fFillDbxHist) {
                    // -- names for SMB histograms (summing all ASICs)
                    string sh = Form("%s_gz", sn.c_str());
                    string sns = Form("%s_SMB%d", sz.c_str(), ismb);
                    string shs = Form("%s_gz", sns.c_str());
                    cout << "booking sn ->" << sn << "<- with histogram name sh ->" << sh << "<-" << endl;

                    // -- global z
                    fSmbMuTrigGZ.insert(make_pair(sn, new TH1F(sh.c_str(),
                        Form("MuTrig hits, global z for %s %s", sz.c_str(), sn.c_str()),
                        nbins, +zmin, +zmax
                    )));
                    if(0 == iasic) {
                        fSmbMuTrigGZ.insert(make_pair(sns, new TH1F(shs.c_str(),
                            Form("MuTrig hits, global z for %s %s", sz.c_str(), sns.c_str()),
                            nbins, +zmin, +zmax
                        )));
                    }

                    // -- edep
                    sh = Form("%s_edep", sn.c_str());
                    fSmbMuTrigEdep.insert(make_pair(sn, new TH1F(sh.c_str(),
                        Form("MuTrig hits, Edep for %s %s", sz.c_str(), sn.c_str()),
                        100, 0., 0.5
                    )));
                    shs = Form("%s_edep", sns.c_str());
                    if(0 == iasic) {
                        fSmbMuTrigEdep.insert(make_pair(sns, new TH1F(shs.c_str(),
                            Form("MuTrig hits, Edep for %s %s", sz.c_str(), sns.c_str()),
                            100, 0., 1.0
                        )));
                    }

                    // -- lz
                    sh = Form("%s_lz", sn.c_str());
                    fSmbMuTrigLZ.insert(make_pair(sn, new TH1F(sh.c_str(),
                        Form("MuTrig hits, local z for %s %s", sz.c_str(), sn.c_str()),
                        nbins, -1., +1.
                    )));
                    shs = Form("%s_lz", sns.c_str());
                    if(0 == iasic) {
                        fSmbMuTrigLZ.insert(make_pair(sns, new TH1F(shs.c_str(),
                            Form("MuTrig hits, local z for %s %s", sz.c_str(), sns.c_str()),
                            nbins, -1., +1.
                        )));
                    }

                    // -- lxy
                    sh = Form("%s_lxy", sn.c_str());
                    fSmbMuTrigLXY.insert(make_pair(sn, new TH2F(sh.c_str(),
                        Form("MuTrig hits, local xy for %s %s", sz.c_str(), sn.c_str()),
                        100, -5., +5., 100, -5., +5.
                    )));

                    shs = Form("%s_lxy", sns.c_str());
                    if(0 == iasic) {
                        fSmbMuTrigLXY.insert(make_pair(sns, new TH2F(shs.c_str(),
                            Form("MuTrig hits, local xy for %s %s", sz.c_str(), sns.c_str()),
                            100, -5., +5., 100, -5., +5.
                        )));
                    }

                    // -- gxy
                    sh = Form("%s_gxy", sn.c_str());
                    fSmbMuTrigGXY.insert(make_pair(sn, new TH2F(sh.c_str(),
                        Form("MuTrig hits, global xy for %s %s", sz.c_str(), sn.c_str()),
                        200, -100., +100., 200, -100., +100.
                    )));

                    shs = Form("%s_gxy", sns.c_str());
                    if(0 == iasic) {
                        fSmbMuTrigGXY.insert(make_pair(sns, new TH2F(shs.c_str(),
                            Form("MuTrig hits, global xy for %s %s", sz.c_str(), sns.c_str()),
                            200, -100., +100., 200, -100., +100.
                        )));
                    }
                }
            }
        }
    }
}

// ----------------------------------------------------------------------
Mu3eFibreSmbMuTrigSD::~Mu3eFibreSmbMuTrigSD() {
}

// ----------------------------------------------------------------------
void Mu3eFibreSmbMuTrigSD::Initialize(G4HCofThisEvent*) {
}

// ----------------------------------------------------------------------
G4bool Mu3eFibreSmbMuTrigSD::ProcessHits(G4Step* aStep, G4TouchableHistory*) {
  const G4Event *evt = G4RunManager::GetRunManager()->GetCurrentEvent();
  int eventId(0);
  if (evt) {
    eventId = evt->GetEventID();
  } else {
    cout << " XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX no evt!!!!!!" << endl;
  }
 
 
    static int oldEventId(-1); 
    static std::map<std::string, double> eDepCounter;

    int DBX(0);
    G4double edep = aStep->GetTotalEnergyDeposit();
    if(edep <= 0) return false;
    auto prePoint = aStep->GetPreStepPoint();
    auto postPoint = aStep->GetPostStepPoint();

    // prePoint->GetTouchable()->GetCopyNumber(1) =
    //   0: downstream z > 0
    //   1: upstream   z < 0

    auto feePos = prePoint->GetTouchable()->GetTranslation();
    auto hitPos = prePoint->GetPosition();
    auto localPos = prePoint->GetTouchable()->GetHistory()->GetTopTransform().TransformPoint(hitPos);

    // -- this should correspond to the specbook
    string smbName = prePoint->GetTouchable()->GetVolume()->GetName();
    int smbNumber(-1), muTrigNumber(-1);
    sscanf(smbName.c_str(), "SMB_%d_ASIC_%d", &smbNumber, &muTrigNumber);

    // -- calculate from smbNumber and muTrigNumber
    int sign = (feePos.z() > 0. ? 1. : -1.);
    int feeId = -50;
    if(sign < 0) {
        feeId = sign * (smbNumber + 1) * 4 + sign * muTrigNumber;
    }
    else {
        feeId = sign * smbNumber * 4 + sign * muTrigNumber;
    }

    if(smbNumber < 0) return false;
    if(muTrigNumber < 0) return false;

    string sn = Form("%s_SMB%d_ASIC%d", (feePos.z() > 0 ? "DS" : "US"), smbNumber, muTrigNumber);

    if (fhmap.find(sn) == fhmap.end()) {
      std::string shname = Form("h%s", sn.c_str());
      std::string sename = Form("e%s", sn.c_str());
      std::string setotname = Form("etot%s", sn.c_str());
      std::string stitle("");
      auto hitPos = prePoint->GetPosition();
      stitle = shname + Form(" (%f/%f/%f)", hitPos.x(), hitPos.y(), hitPos.z()); 
      //      std::cout << "creating " << shname  << " with title = " << stitle << std::endl;
      fhmap.insert(std::make_pair(sn, new TH1F(shname.c_str(), stitle.c_str(), 60, 0., 60.)));
      femap.insert(std::make_pair(sn, new TH1F(sename.c_str(), stitle.c_str(), 100, 0., 2.)));
      fetotmap.insert(std::make_pair(sn, new TH1F(setotname.c_str(), stitle.c_str(), 100, 0., 5.)));
      eDepCounter.insert(std::make_pair(sn, 0.));
    } 
    
    if (prePoint->GetStepStatus() == fGeomBoundary) {
      auto track = aStep->GetTrack();
      //      std::cout << "filling sn = " << sn << std::endl;
      fhmap[sn]->Fill(track->GetMomentum().mag());
    }
    femap[sn]->Fill(edep);

    // -- write out old eDep accumulators
    if (eventId != oldEventId) {
      cout << "oldEventId = " << oldEventId << " eventId = " << eventId << endl;
      oldEventId = eventId;
      for (auto it: eDepCounter) {
        if (it.second > 0.0001) {
          //          std::cout << "writing   " << it.first << " with Edep = " << it.second << std::endl;
          fetotmap[it.first]->Fill(it.second);
        }
        eDepCounter[it.first] = 0.0;
      }
      eDepCounter[sn] = edep;
      //      std::cout << "setting   " << sn << " Edep = " << edep << " total = " << eDepCounter[sn] << std::endl;
    }  else {
      eDepCounter[sn] += edep;
      //      std::cout << "adding to " << sn << " Edep = " << edep << " total = " << eDepCounter[sn] << std::endl;
    }
        
    if(fFillDbxHist) {
        int ix = fSmbMuTrigPlanePosZ->GetXaxis()->FindBin(feePos.x());
        int iy = fSmbMuTrigPlanePosZ->GetYaxis()->FindBin(feePos.y());

        bool entry = (prePoint->GetStepStatus() == fGeomBoundary);
        bool exit = (postPoint->GetStepStatus() == fGeomBoundary);

        if(DBX > 0)
            std::cout << "SmbMuTrigSD> " << smbName
                << " "
                // << prePoint->GetTouchable()->GetCopyNumber(0) << "/"
                // << prePoint->GetTouchable()->GetCopyNqumber(1) << " "
                // << prePoint->GetTouchable()->GetVolume()->GetName()
                // << " feeID = " << feeId
                // << " smbID = " << smbId
                << " r = " << TMath::Sqrt(feePos.x() * feePos.x() + feePos.y() * feePos.y())
                << " z = " << feePos.z() << " muTrigNumber = " << muTrigNumber << " edep = " << edep
                << " (r) = " << feePos.x() << "/" << feePos.y() << "/" << feePos.z() << " ix/iy = " << ix << "/"
                << iy << " W(r) = " << hitPos.x() << "/" << hitPos.y() << "/" << hitPos.z() << " e/e = " << entry
                << "/" << exit << std::endl;

        // -- ASIC level histograms
        if(DBX > 1) cout << "filling sn = " << sn << endl;
        fSmbMuTrigGZ[sn]->Fill(TMath::Abs(hitPos.z()));
        fSmbMuTrigEdep[sn]->Fill(edep);
        fSmbMuTrigLZ[sn]->Fill(localPos.z());
        fSmbMuTrigLXY[sn]->Fill(localPos.x(), localPos.y());
        fSmbMuTrigGXY[sn]->Fill(hitPos.x(), hitPos.y());

        // -- SMB integrated
        string sns = Form("%s_SMB%d", (feePos.z() > 0 ? "DS" : "US"), smbNumber);
        if(DBX > 1) cout << "filling sns = " << sns << endl;
        fSmbMuTrigGZ[sns]->Fill(TMath::Abs(hitPos.z()));
        fSmbMuTrigEdep[sns]->Fill(edep);
        fSmbMuTrigLZ[sns]->Fill(localPos.z());
        fSmbMuTrigLXY[sns]->Fill(localPos.x(), localPos.y());
        fSmbMuTrigGXY[sns]->Fill(hitPos.x(), hitPos.y());

        // -- keep a record which fedID is where
        if(feePos.z() > 0) {
            fSmbMuTrigPosZ->Fill(TMath::Abs(hitPos.z()));
            fSmbMuTrigPosZ_xy->Fill(hitPos.x(), hitPos.y());

            fSmbMuTrigPlanePosZ->SetBinContent(ix, iy, muTrigNumber);
        }
        else {
            fSmbMuTrigNegZ->Fill(TMath::Abs(hitPos.z()));
            fSmbMuTrigNegZ_xy->Fill(hitPos.x(), hitPos.y());
            fSmbMuTrigPlaneNegZ->SetBinContent(ix, iy, muTrigNumber);
        }
    }

    if(edep > 0) {
        G4double dose_c = 0;
        G4double cubicVolume = prePoint->GetPhysicalVolume()->GetLogicalVolume()->GetSolid()->GetCubicVolume();
        G4double density = prePoint->GetMaterial()->GetDensity(); // 2.3; /*DBX FIXME */
        dose_c = edep / joule / (density / kg * cubicVolume);
        dose_c *= prePoint->GetWeight();
        fDose[sn] += dose_c;
        fDose2[sn] += (dose_c * dose_c);

        if(fFillDbxHist) fMuTrigEdep->Fill(edep);
    }

    return true;
}

// ----------------------------------------------------------------------
void Mu3eFibreSmbMuTrigSD::writeStat() {
    cout << "Mu3eFibreSmbMuTrigSD::writeStat()" << endl;
    gDirectory->mkdir("FibreSmbMuTrig");
    gDirectory->cd("FibreSmbMuTrig");

    double evendtime = (Mu3eEvent::GetInstance()->startTime + Mu3eEvent::GetInstance()->frameLength) / s;
    TH1F* hFibreSmbDose = new TH1F("hFibreSmbDose", TString::Format("Fibre Smb Dose (time = %f)", evendtime), 100, 0, 100);
    hFibreSmbDose->GetXaxis()->SetTitle("fibre Smb MuTrig");
    hFibreSmbDose->GetYaxis()->SetTitle("dose/time [Gy/s]");

    TH1F* hFibreSmbDose2 = new TH1F("hFibreSmbDose2", TString::Format("Fibre Smb Dose "), 100, 0, 100);
    hFibreSmbDose2->GetXaxis()->SetTitle("fibre Smb MuTrig");
    hFibreSmbDose2->GetYaxis()->SetTitle("dose [Gy]");

    int ibin(1);
    double error(0.);
    map<string, double>::iterator i0end = fDose.end();
    for(map<string, double>::iterator ih = fDose.begin(); ih != i0end; ++ih) {
        hFibreSmbDose->SetBinContent(ibin, ih->second / evendtime);
        hFibreSmbDose->GetXaxis()->SetBinLabel(ibin, ih->first.c_str());

        hFibreSmbDose2->SetBinContent(ibin, ih->second);
        error = TMath::Sqrt(fDose2[ih->first]);
        hFibreSmbDose2->SetBinError(ibin, error);
        hFibreSmbDose2->GetXaxis()->SetBinLabel(ibin, ih->first.c_str());

        ++ibin;
    }
    hFibreSmbDose->Write();
    hFibreSmbDose2->Write();

    if(fFillDbxHist) {
        map<string, TH1F*>::iterator i1end = fSmbMuTrigGZ.end();
        for(map<string, TH1F*>::iterator ih = fSmbMuTrigGZ.begin(); ih != i1end; ++ih) ih->second->Write();

        i1end = fSmbMuTrigEdep.end();
        for(map<string, TH1F*>::iterator ih = fSmbMuTrigEdep.begin(); ih != i1end; ++ih) ih->second->Write();

        i1end = fSmbMuTrigLZ.end();
        for(map<string, TH1F*>::iterator ih = fSmbMuTrigLZ.begin(); ih != i1end; ++ih) ih->second->Write();

        map<string, TH2F*>::iterator i2end = fSmbMuTrigLXY.end();
        for(map<string, TH2F*>::iterator ih = fSmbMuTrigLXY.begin(); ih != i2end; ++ih) ih->second->Write();

        i2end = fSmbMuTrigGXY.end();
        for(map<string, TH2F*>::iterator ih = fSmbMuTrigGXY.begin(); ih != i2end; ++ih) ih->second->Write();

        fSmbMuTrigPlanePosZ->Write();
        fSmbMuTrigPlaneNegZ->Write();

        fSmbMuTrigPosZ->Write();
        fSmbMuTrigNegZ->Write();

        fMuTrigEdep->Write();
        fMuTrigEdepCombined->Write();
    }


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

// ----------------------------------------------------------------------
void Mu3eFibreSmbMuTrigSD::EndOfEvent(G4HCofThisEvent*) {
}
