#include "Mu3eFibreSmbMuTrigSD.h"

#include "Mu3eEvent.h"

#include <TString.h>
#include <TH1.h>
#include <TMath.h>
#include <TFile.h>
#include <TTree.h>


#include "G4SDManager.hh"


using namespace CLHEP;
using namespace std;


// ----------------------------------------------------------------------
Mu3eFibreSmbMuTrigSD::Mu3eFibreSmbMuTrigSD(const G4String& name) : G4VSensitiveDetector(name) {

  cout << "Mu3eFibreSmbMuTrigSD::Mu3eFibreSmbMuTrigSD ctor" << endl;
  
  int nbins(1000);
  double zmin(100.), zmax(200.);
  
  fSmbMuTrigPosZ = new TH1F("SmbMuTrigPosZ", "SMB MuTrig hits, |z| global position for positive z", 3000, 120., 150.);
  fSmbMuTrigNegZ = new TH1F("SmbMuTrigNegZ", "SMB MuTrighits, |z| global position for negative z", 3000, 120., 150.);

  fSmbMuTrigPosZ_xy = new TH2F("fSmbMuTrigPosZ_xy", "SMB MuTrighits, xy global position for positive z",
                               200, -100., 100., 200, -100., 100.);
  fSmbMuTrigNegZ_xy = new TH2F("fSmbMuTrigNegZ_xy", "SMB MuTrighits, xy global position for negative z",
                               200, -100., 100., 200, -100., 100.);

  fSmbMuTrigPlanePosZ = new TH2F("SmbMuTrigPlanePosZ", "Smb MuTrig ID for z > 0 (DS)", 400, -200., 200., 400, -200., 200.);
  fSmbMuTrigPlaneNegZ = new TH2F("SmbMuTrigPlaneNegZ", "Smb MuTrig ID for z < 0 (US)", 400, -200., 200., 400, -200., 200.);
  
  for (int ix = 1; ix <= fSmbMuTrigPlanePosZ->GetNbinsX(); ++ix) {
    for (int iy = 1; iy <= fSmbMuTrigPlanePosZ->GetNbinsY(); ++iy) {
      fSmbMuTrigPlanePosZ->SetBinContent(ix, iy, -2.);
      fSmbMuTrigPlaneNegZ->SetBinContent(ix, iy, -2.);
    }
  }

  // -- per-ASIC histograms
  for (unsigned int iz = 0; iz  < 2; ++iz) {
    string sz("US");
    if (1 == iz) sz = string("DS");
    for (unsigned int ismb = 0; ismb < 12; ++ismb) {
      for (unsigned int iasic = 0; iasic < 4; ++iasic) {
        // -- names for ASIC histograms
        string sn = Form("%s_SMB%d_ASIC%d", sz.c_str(), ismb, iasic);
        string sh = Form("%s_gz", sn.c_str());

        // -- counter for deposited energy
        fDose.insert(make_pair(sn, 0.));

        // -- names for SMB histograms (summing all ASICs)
        string sns = Form("%s_SMB%d", sz.c_str(), ismb);
        string shs = Form("%s_gz", sns.c_str());
        cout << "booking sn ->" << sn << "<- with histogram name sh ->" << sh << "<-" << endl;

        // -- global z
        fSmbMuTrigGZ.insert(make_pair(sn, new TH1F(sh.c_str(),
                                                   Form("MuTrig hits, global z for %s %s",
                                                        sz.c_str(), sn.c_str()),
                                                   nbins, +zmin, +zmax)));
        if (0 == iasic) fSmbMuTrigGZ.insert(make_pair(sns, new TH1F(shs.c_str(),
                                                                    Form("MuTrig hits, global z for %s %s",
                                                                         sz.c_str(), sns.c_str()),
                                                                    nbins, +zmin, +zmax)));

        // -- edep
        sh = Form("%s_edep", sn.c_str());
        fSmbMuTrigEdep.insert(make_pair(sn, new TH1F(sh.c_str(), Form("MuTrig hits, Edep for %s %s",
                                                                      sz.c_str(), sn.c_str()),
                                                     100, 0., 0.5)));
        shs = Form("%s_edep", sns.c_str());
        if (0 == iasic) fSmbMuTrigEdep.insert(make_pair(sns, new TH1F(shs.c_str(), Form("MuTrig hits, Edep for %s %s",
                                                                                        sz.c_str(), sns.c_str()),
                                                                      100, 0., 1.0)));

        // -- lz
        sh = Form("%s_lz", sn.c_str());
        fSmbMuTrigLZ.insert(make_pair(sn, new TH1F(sh.c_str(), Form("MuTrig hits, local z for %s %s",
                                                                    sz.c_str(), sn.c_str()),
                                                     nbins, -1., +1.)));
        shs = Form("%s_lz", sns.c_str());
        if (0 == iasic) fSmbMuTrigLZ.insert(make_pair(sns, new TH1F(shs.c_str(), Form("MuTrig hits, local z for %s %s",
                                                                                      sz.c_str(), sns.c_str()),
                                                                    nbins, -1., +1.)));

        // -- lxy
        sh = Form("%s_lxy", sn.c_str());
        fSmbMuTrigLXY.insert(make_pair(sn, new TH2F(sh.c_str(), Form("MuTrig hits, local xy for %s %s", sz.c_str(), sn.c_str()),
                                                    100, -5., +5., 100, -5., +5.)));

        shs = Form("%s_lxy", sns.c_str());
        if (0 == iasic) fSmbMuTrigLXY.insert(make_pair(sns, new TH2F(shs.c_str(), Form("MuTrig hits, local xy for %s %s",
                                                                                       sz.c_str(), sns.c_str()),
                                                                     100, -5., +5., 100, -5., +5.)));

        // -- gxy
        sh = Form("%s_gxy", sn.c_str());
        fSmbMuTrigGXY.insert(make_pair(sn, new TH2F(sh.c_str(), Form("MuTrig hits, global xy for %s %s", sz.c_str(), sn.c_str()),
                                                    200, -100., +100., 200, -100., +100.)));

        shs = Form("%s_gxy", sns.c_str());
        if (0 == iasic) fSmbMuTrigGXY.insert(make_pair(sns, new TH2F(shs.c_str(), Form("MuTrig hits, global xy for %s %s",
                                                                                       sz.c_str(), sns.c_str()),
                                                                     200, -100., +100., 200, -100., +100.)));


      }
    }
  }
  
  fSmbMutrigRadialOutElpz1  = new TH2F("SmbMutrigRadialOutElpz1", "e +z z-position at Smb local coord",
                                       70, -30., 40., 100, -50., 50.);
  fSmbMutrigRadialOutElpz2  = new TH2F("SmbMutrigRadialOutElpz2", "e +z z-position at Smb world coord",
                                       80, 100., 180., 100, -50., 50.);
  fSmbMutrigRadialOutElpz3  = new TH2F("SmbMutrigRadialOutElpz3", "e +z z-position at Smb edep",
                                       80, 100., 180., 100, -50., 50.);

  fSmbMutrigRadialOutElmz1  = new TH2F("SmbMutrigRadialOutElmz1", "e -z z-position at Smb local coord",
                                       70, -30., 40., 100, -50., 50.);
  fSmbMutrigRadialOutElmz2  = new TH2F("SmbMutrigRadialOutElmz2", "e -z z-position at Smb world coord",
                                       80, -180., -100., 100, -50., 50.);
  fSmbMutrigRadialOutElmz3  = new TH2F("SmbMutrigRadialOutElmz3", "e -z z-position at Smb edep",
                                       40, -180., -100., 100, -50., 50.);


  fSmbMutrigRadialOutElx1 = new TH2F("SmbMutrigRadialOutElmx1", "e x-position +z at Smb local coord", 60, -30., 30., 250, 0., 250.);
  fSmbMutrigRadialOutEly1 = new TH2F("SmbMutrigRadialOutElmy1", "e y-position +z at Smb local coord", 60, -30., 30., 250, 0., 250.);

  fSmbMutrigRadialOutElx1a = new TH2F("SmbMutrigRadialOutElmx1a", "e x-position -z at Smb local coord", 60, -30., 30., 250, 0., 250.);
  fSmbMutrigRadialOutEly1a = new TH2F("SmbMutrigRadialOutElmy1a", "e y-position -z at Smb local coord", 60, -30., 30., 250, 0., 250.);

  
  // *fPosZSmbMutrigRadialInMup;
  // fPosZSmbMutrigRadialOutMun, *fPosZSmbMutrigRadialInMun;
  // fPosZSmbMutrigRadialOutElp, *fPosZSmbMutrigRadialInElp;
  // fPosZSmbMutrigRadialOutEln, *fPosZSmbMutrigRadialInEln;
  
}


// ----------------------------------------------------------------------
Mu3eFibreSmbMuTrigSD::~Mu3eFibreSmbMuTrigSD() {

}


// ----------------------------------------------------------------------
void Mu3eFibreSmbMuTrigSD::Initialize(G4HCofThisEvent*) {

}


// ----------------------------------------------------------------------
G4bool Mu3eFibreSmbMuTrigSD::ProcessHits(G4Step* aStep, G4TouchableHistory*) {
  int DBX(0);
  G4double edep = aStep->GetTotalEnergyDeposit();
  if (edep <= 0) return false;
  auto prePoint  = aStep->GetPreStepPoint();
  auto postPoint = aStep->GetPostStepPoint();

  // prePoint->GetTouchable()->GetCopyNumber(1) =
  //   0: downstream z > 0
  //   1: upstream   z < 0

  G4int apdgid = TMath::Abs(aStep->GetTrack()->GetDynamicParticle()->GetPDGcode());
  
  auto feePos   = prePoint->GetTouchable()->GetTranslation();
  auto hitPos   = prePoint->GetPosition();
  auto localPos = prePoint->GetTouchable()->GetHistory()->GetTopTransform().TransformPoint(hitPos);

  // -- this should correspond to the specbook
  string smbName = prePoint->GetTouchable()->GetVolume()->GetName();
  int smbNumber(-1), muTrigNumber(-1);
  sscanf(smbName.c_str(), "SMB_%d_ASIC_%d", &smbNumber, &muTrigNumber);

  // -- calculate from smbNumber and muTrigNumber
  int sign = (feePos.z() > 0.? 1.: -1.);
  int feeId(-50);
  if (sign < 0) {
    feeId = sign*(smbNumber+1)*4 + sign*muTrigNumber;
  } else {
    feeId = sign*smbNumber*4 + sign*muTrigNumber;
  }
  
  bool entry = (prePoint->GetStepStatus() == fGeomBoundary);
  bool exit  = (postPoint->GetStepStatus() == fGeomBoundary);
    
  int ix = fSmbMuTrigPlanePosZ->GetXaxis()->FindBin(feePos.x());
  int iy = fSmbMuTrigPlanePosZ->GetYaxis()->FindBin(feePos.y());

  if (DBX > 0) std::cout << "SmbMuTrigSD> " << smbName << " " 
                 //                   << prePoint->GetTouchable()->GetCopyNumber(0) << "/"
                 // << prePoint->GetTouchable()->GetCopyNumber(1) << " "
                 // << prePoint->GetTouchable()->GetVolume()->GetName()
                 // << " feeID = " << feeId
                 // << " smbID = " << smbId
                         << " r = " << TMath::Sqrt(feePos.x()*feePos.x() + feePos.y()*feePos.y())
                         << " z = " << feePos.z()
                         << " muTrigNumber = " << muTrigNumber
                         << " edep = " << edep
                         << " (r) = " << feePos.x() << "/" << feePos.y() << "/" << feePos.z()
                         << " ix/iy = " << ix << "/" << iy
                         << " W(r) = " << hitPos.x() << "/" << hitPos.y() << "/" << hitPos.z()
                         << " e/e = " << entry << "/" << exit
                         << std::endl;

  if (smbNumber < 0) return false; 
  if (muTrigNumber < 0) return false; 
  
  // -- ASIC level histograms
  string sn = Form("%s_SMB%d_ASIC%d", (feePos.z()>0?"DS":"US"), smbNumber, muTrigNumber);
  if (DBX > 1) cout << "filling sn = " << sn << endl;
  fSmbMuTrigGZ[sn]->Fill(TMath::Abs(hitPos.z()));
  fSmbMuTrigEdep[sn]->Fill(edep);
  fSmbMuTrigLZ[sn]->Fill(localPos.z());
  fSmbMuTrigLXY[sn]->Fill(localPos.x(), localPos.y());
  fSmbMuTrigGXY[sn]->Fill(hitPos.x(), hitPos.y());

  // -- SMB integrated
  string sns = Form("%s_SMB%d", (feePos.z()>0?"DS":"US"), smbNumber);
  if (DBX > 1) cout << "filling sns = " << sns << endl;
  fSmbMuTrigGZ[sns]->Fill(TMath::Abs(hitPos.z()));
  fSmbMuTrigEdep[sns]->Fill(edep);
  fSmbMuTrigLZ[sns]->Fill(localPos.z());
  fSmbMuTrigLXY[sns]->Fill(localPos.x(), localPos.y());
  fSmbMuTrigGXY[sns]->Fill(hitPos.x(), hitPos.y());

  
  // -- keep a record which fedID is where
  if (feePos.z() > 0) {
    fSmbMuTrigPosZ->Fill(TMath::Abs(hitPos.z()));
    fSmbMuTrigPosZ_xy->Fill(hitPos.x(), hitPos.y());
   
    fSmbMuTrigPlanePosZ->SetBinContent(ix, iy, muTrigNumber);
    if (11 == apdgid) {
      fSmbMutrigRadialOutElpz1->Fill(localPos.z(), feeId);
      fSmbMutrigRadialOutElx1->Fill(localPos.x(), feeId);
      fSmbMutrigRadialOutEly1->Fill(localPos.y(), feeId);
	
      fSmbMutrigRadialOutElpz2->Fill(TMath::Abs(hitPos.z()), feeId);
      fSmbMutrigRadialOutElpz3->Fill(TMath::Abs(hitPos.z()), feeId, edep);
    }
  } else {
    fSmbMuTrigNegZ->Fill(TMath::Abs(hitPos.z()));
    fSmbMuTrigNegZ_xy->Fill(hitPos.x(), hitPos.y());
    fSmbMuTrigPlaneNegZ->SetBinContent(ix, iy, muTrigNumber);
    if (11 == apdgid) {
      fSmbMutrigRadialOutElmz1->Fill(localPos.z(), feeId);
      fSmbMutrigRadialOutElx1a->Fill(localPos.x(), feeId);
      fSmbMutrigRadialOutEly1a->Fill(localPos.y(), feeId);

      fSmbMutrigRadialOutElmz2->Fill(TMath::Abs(hitPos.z()), feeId);
      fSmbMutrigRadialOutElmz3->Fill(TMath::Abs(hitPos.z()), feeId, edep);
    }
  }

  if (edep > 0) {
    G4double dose_c = 0;
    //  int pid = aStep->GetTrack()->GetParticleDefinition()->GetPDGEncoding();
    G4double cubicVolume = prePoint->GetPhysicalVolume()->GetLogicalVolume()->GetSolid()->GetCubicVolume();
    G4double density = prePoint->GetMaterial()->GetDensity();
    //G4cout << "DEBUG density " << density << " " << density/kg << " " << (density/kg * cubicVolume) << G4endl;
    dose_c    = edep/joule / ( density/kg * cubicVolume );
    dose_c *= prePoint->GetWeight();
    fDose[sn] += dose_c;
  }

  return true;
}


// ----------------------------------------------------------------------
void Mu3eFibreSmbMuTrigSD::writeStat() {
  cout << "Mu3eFibreSmbMuTrigSD::writeStat()" << endl;
  gDirectory->mkdir("FibreSmbMuTrig");
  gDirectory->cd("FibreSmbMuTrig");

  double evendtime = (Mu3eEvent::GetInstance()->startTime + Mu3eEvent::GetInstance()->frameLength) /s;
  TH1F * hFibreSmbDose = new TH1F("hFibreSmbDose", TString::Format("Fibre Smb Dose (time = %f)", evendtime), 100, 0, 100);
  hFibreSmbDose->GetXaxis()->SetTitle("fibre Smb MuTrig");
  hFibreSmbDose->GetYaxis()->SetTitle("dose/time [Gy/s]");

  TH1F * hFibreSmbDose2 = new TH1F("hFibreSmbDose2", TString::Format("Fibre Smb Dose "), 100, 0, 100);
  hFibreSmbDose2->GetXaxis()->SetTitle("fibre Smb MuTrig");
  hFibreSmbDose2->GetYaxis()->SetTitle("dose/time [Gy]");


  int ibin(1); 
  map<string, double>::iterator i0end = fDose.end();
  for (map<string, double>::iterator ih = fDose.begin(); ih != i0end; ++ih) {
    hFibreSmbDose->SetBinContent(ibin, ih->second/evendtime);
    hFibreSmbDose->GetXaxis()->SetBinLabel(ibin, ih->first.c_str());

    hFibreSmbDose2->SetBinContent(ibin, ih->second);
    hFibreSmbDose2->GetXaxis()->SetBinLabel(ibin, ih->first.c_str());

    ++ibin;
  }
  hFibreSmbDose->Write();
  hFibreSmbDose2->Write();


  map<string, TH1F*>::iterator i1end = fSmbMuTrigGZ.end();
  for (map<string, TH1F*>::iterator ih = fSmbMuTrigGZ.begin(); ih != i1end; ++ih) ih->second->Write();

  i1end = fSmbMuTrigEdep.end();
  for (map<string, TH1F*>::iterator ih = fSmbMuTrigEdep.begin(); ih != i1end; ++ih) ih->second->Write();

  i1end = fSmbMuTrigLZ.end();
  for (map<string, TH1F*>::iterator ih = fSmbMuTrigLZ.begin(); ih != i1end; ++ih) ih->second->Write();

  map<string, TH2F*>::iterator i2end = fSmbMuTrigLXY.end();
  for (map<string, TH2F*>::iterator ih = fSmbMuTrigLXY.begin(); ih != i2end; ++ih) ih->second->Write();

  i2end = fSmbMuTrigGXY.end();
  for (map<string, TH2F*>::iterator ih = fSmbMuTrigGXY.begin(); ih != i2end; ++ih) ih->second->Write();

  
  fSmbMuTrigPlanePosZ->Write();
  fSmbMuTrigPlaneNegZ->Write();

  fSmbMuTrigPosZ->Write();
  fSmbMuTrigNegZ->Write();
  
  fSmbMutrigRadialOutElpz1->Write();
  fSmbMutrigRadialOutElpz2->Write();
  fSmbMutrigRadialOutElpz3->Write();

  fSmbMutrigRadialOutElmz1->Write();
  fSmbMutrigRadialOutElmz2->Write();
  fSmbMutrigRadialOutElmz3->Write();

  fSmbMutrigRadialOutElx1->Write();
  fSmbMutrigRadialOutEly1->Write();

  fSmbMutrigRadialOutElx1a->Write();
  fSmbMutrigRadialOutEly1a->Write();
  
  gDirectory->cd("..");
}

// ----------------------------------------------------------------------
void Mu3eFibreSmbMuTrigSD::EndOfEvent(G4HCofThisEvent*) {

}

// ----------------------------------------------------------------------
// http://ieeexplore.ieee.org/document/273529
// data from http://www.sr-niel.org/Simulation/516481niel_e.html
double Mu3eFibreSmbMuTrigSD::damageFunction(double energy) {
  if(energy <= df_e.front()) return df_n.front();
  if(energy >= df_e.back())  return df_n.back();

  auto index = std::distance(df_e.begin(), std::upper_bound (df_e.begin(), df_e.end(), energy));
  if (index > 1) {
    double frac = (energy - df_e.at(index-1)) / (df_e.at(index) - df_e.at(index-1));
    return df_n.at(index-1) + frac * (df_n.at(index) - df_n.at(index-1));

  } else return df_n.front(); // we should never get here
  //double niel2df = std:pow(energy, 0.98390033838489543) * 1.8e-7;
}


// ----------------------------------------------------------------------
// damage function for electrons / 95 MeVmb (neutron equivalent)
// http://www.sr-niel.org/Simulation/516481niel_e.html
const std::vector<double> Mu3eFibreSmbMuTrigSD::df_e{
  00000.10,  00000.15,  00000.20,  00000.25,  00000.30,  00000.35,  00000.40,
    00000.45,  00000.50,  00000.55,  00000.60,  00000.65,  00000.70,  00000.75,
    00000.80,  00000.85,  00000.90,  00000.95,  00001.00,  00001.50,  00002.00,
    00002.50,  00003.00,  00003.50,  00004.00,  00004.50,  00005.00,  00005.50,
    00006.00,  00006.50,  00007.00,  00007.50,  00008.00,  00008.50,  00009.00,
    00009.50,  00010.00,  00015.00,  00020.00,  00025.00,  00030.00,  00035.00,
    00040.00,  00045.00,  00050.00,  00055.00,  00060.00,  00065.00,  00070.00,
    00075.00,  00080.00,  00085.00,  00090.00,  00095.00,  00100.00,  00150.00,
    00200.00,  00250.00,  00300.00,  00350.00,  00400.00,  00450.00,  00500.00,
    00550.00,  00600.00,  00650.00,  00700.00,  00750.00,  00800.00,  00850.00,
    00900.00,  00950.00,  01000.00,  01500.00,  02000.00,  02500.00,  03000.00,
    03500.00,  04000.00,  04500.00,  05000.00,  05500.00,  06000.00,  06500.00,
    07000.00,  07500.00,  08000.00,  08500.00,  09000.00,  09500.00,  10000.00
    }; // MEV

// ----------------------------------------------------------------------
const std::vector<double> Mu3eFibreSmbMuTrigSD::df_n {
  0.        ,  0.        ,  0.        ,  0.00121725,  0.00287333,
    0.00419408,  0.00531324,  0.00629959,  0.00719216,  0.00801404,
    0.00878142,  0.00950412,  0.01018902,  0.01084152,  0.01146603,
    0.01206549,  0.01264189,  0.01319815,  0.01373576,  0.01831697,
    0.02190249,  0.02484485,  0.02733258,  0.02948252,  0.03137078,
    0.03305136,  0.03456304,  0.0359348 ,  0.03718824,  0.03834103,
    0.03940691,  0.0403967 ,  0.04132021,  0.0421848 ,  0.04299686,
    0.04376178,  0.04448399,  0.05002946,  0.05370679,  0.05633837,
    0.05830715,  0.05981441,  0.06099273,  0.06192557,  0.06266693,
    0.06326591,  0.06374705,  0.06413983,  0.06445896,  0.06471917,
    0.06493519,  0.06511685,  0.06526414,  0.06538197,  0.06548507,
    0.06592203,  0.06598586,  0.06597113,  0.06594167,  0.06590731,
    0.06587785,  0.06584839,  0.06582384,  0.0658042 ,  0.06578456,
    0.06576984,  0.06575511,  0.06574038,  0.06573056,  0.06572074,
    0.06571092,  0.0657011 ,  0.06569128,  0.06564218,  0.06561273,
    0.065598  ,  0.06558327,  0.06557836,  0.06556854,  0.06556363,
    0.06555872,  0.06555872,  0.06555381,  0.06555381,  0.0655489 ,
    0.0655489 ,  0.0655489 ,  0.06554399,  0.06554399,  0.06554399,
    0.06554399
    }; //  niel / 95 MeVmb = 1 MeV neutron equivalent
