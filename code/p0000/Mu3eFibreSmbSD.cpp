#include "Mu3eFibreSmbSD.h"

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
Mu3eFibreSmbSD::Mu3eFibreSmbSD(const G4String& name) : G4VSensitiveDetector(name) {

  cout << "Mu3eFibreSmbSD::Mu3eFibreSmbSD ctor" << endl;
  
  int nbins(1000);
  double zmin(100.), zmax(200.);
  
  fSmbPosZ = new TH1F("SmbPosZ", "SMB hits, global position for positive z", 1000, 100., 200.);
  fSmbNegZ = new TH1F("SmbNegZ", "SMB hits, global position for negative z", 1000, -200., -100.);

  for (int i = 0; i < 12; ++i) {
    fSmbZ.insert(make_pair(+100+i, new TH1F(Form("SmbPosZ_%d", i), Form("SMB hits, global pos for +z, iSMB=%d", i), nbins, +zmin, +zmax)));
    fSmbZ.insert(make_pair(-100-i, new TH1F(Form("SmbNegZ_%d", i), Form("SMB hits, global pos for -z, iSMB=%d", i), nbins, -zmax, -zmin)));
  }

  fPlanePosZ = new TH2F("PlanePosZ", "Smb ID for z > 0 (DS)", 200, -200., 200., 200, -200., 200.);
  fPlaneNegZ = new TH2F("PlaneNegZ", "Smb ID for z < 0 (US)", 200, -200., 200., 200, -200., 200.);
  
  for (int ix = 1; ix <= fPlanePosZ->GetNbinsX(); ++ix) {
    for (int iy = 1; iy <= fPlanePosZ->GetNbinsY(); ++iy) {
      fPlanePosZ->SetBinContent(ix, iy, -2.);
      fPlaneNegZ->SetBinContent(ix, iy, -2.);
    }
  }

  
  fRadialOutElpz1  = new TH2F("RadialOutElpz1", "e +z z-position at Smb local coord",
			     70, -30., 40., 250, 0., 250.);
  fRadialOutElpz2  = new TH2F("RadialOutElpz2", "e +z z-position at Smb world coord",
			     80, 100., 180., 250, 0., 250.);
  fRadialOutElpz3  = new TH2F("RadialOutElpz3", "e +z z-position at Smb edep",
			     80, 100., 180., 250, 0., 250.);

  fRadialOutElmz1  = new TH2F("RadialOutElmz1", "e -z z-position at Smb local coord",
			     70, -30., 40., 250, 0., 250.);
  fRadialOutElmz2  = new TH2F("RadialOutElmz2", "e -z z-position at Smb world coord",
			     40, -160., -140., 250, 0., 250.);
  fRadialOutElmz3  = new TH2F("RadialOutElmz3", "e -z z-position at Smb edep",
			     40, -160., -140., 250, 0., 250.);


  fRadialOutElx1 = new TH2F("RadialOutElmx1", "e x-position +z at Smb local coord", 60, -30., 30., 250, 0., 250.);
  fRadialOutEly1 = new TH2F("RadialOutElmy1", "e y-position +z at Smb local coord", 60, -30., 30., 250, 0., 250.);

  fRadialOutElx1a = new TH2F("RadialOutElmx1a", "e x-position -z at Smb local coord", 60, -30., 30., 250, 0., 250.);
  fRadialOutEly1a = new TH2F("RadialOutElmy1a", "e y-position -z at Smb local coord", 60, -30., 30., 250, 0., 250.);

  for (unsigned int i = 0; i < 200; ++i) {
    fDose[i] += 0.;
  }
  
  // *fPosZRadialInMup;
  // fPosZRadialOutMun, *fPosZRadialInMun;
  // fPosZRadialOutElp, *fPosZRadialInElp;
  // fPosZRadialOutEln, *fPosZRadialInEln;
  
}


// ----------------------------------------------------------------------
Mu3eFibreSmbSD::~Mu3eFibreSmbSD() {

}


// ----------------------------------------------------------------------
void Mu3eFibreSmbSD::Initialize(G4HCofThisEvent*) {

}


// ----------------------------------------------------------------------
G4bool Mu3eFibreSmbSD::ProcessHits(G4Step* aStep, G4TouchableHistory*) {
  G4double edep = aStep->GetTotalEnergyDeposit();
  int DBX(0);
  if (edep <= 0) return false;
  auto prePoint  = aStep->GetPreStepPoint();
  auto postPoint = aStep->GetPostStepPoint();

  // prePoint->GetTouchable()->GetCopyNumber(1) =
  //   0: downstream z > 0
  //   1: upstream   z < 0

  G4int apdgid = TMath::Abs(aStep->GetTrack()->GetDynamicParticle()->GetPDGcode());
  
  // -- this is not very meaningful
  uint32_t feeId = prePoint->GetTouchable()->GetCopyNumber(0) 
    + 100*prePoint->GetTouchable()->GetCopyNumber(1);

  auto feePos   = prePoint->GetTouchable()->GetTranslation();
  auto hitPos   = prePoint->GetPosition();
  auto localPos = prePoint->GetTouchable()->GetHistory()->GetTopTransform().TransformPoint(hitPos);

  // -- this should correspond to the specbook
  string smbName = prePoint->GetTouchable()->GetVolume()->GetName();
  int smbNumber(-1);
  sscanf(smbName.c_str(), "SMB_%d", &smbNumber);
  int32_t smbId = smbNumber;
  if (DBX > 1) cout << "smbNumber = " << smbNumber << " smbName = " << smbName << endl;
  
  bool entry = (prePoint->GetStepStatus() == fGeomBoundary);
  bool exit  = (postPoint->GetStepStatus() == fGeomBoundary);
    
  int ix = fPlanePosZ->GetXaxis()->FindBin(feePos.x());
  int iy = fPlanePosZ->GetYaxis()->FindBin(feePos.y());

  if (DBX > 0) std::cout << "SmbSD> " 
                     << prePoint->GetTouchable()->GetCopyNumber(0) << "/"
                     << prePoint->GetTouchable()->GetCopyNumber(1) << "  ->"
                     << prePoint->GetTouchable()->GetVolume()->GetName()
                     << "<- feeID = " << feeId
                     << "<- smbID = " << smbId
                     << " edep = " << edep
                     << " (r) = " << feePos.x() << "/" << feePos.y() << "/" << feePos.z()
                     << " ix/iy = " << ix << "/" << iy
                     << " W(r) = " << hitPos.x() << "/" << hitPos.y() << "/" << hitPos.z()
                     << " e/e = " << entry << "/" << exit
                     << std::endl;

  
  // -- keep a record which fedID is where
  if (feePos.z() > 0) {
    fSmbPosZ->Fill(hitPos.z());
    fSmbZ[100+smbNumber]->Fill(hitPos.z());
   
    fPlanePosZ->SetBinContent(ix, iy, smbId);
    if (11 == apdgid) {
      fRadialOutElpz1->Fill(localPos.y(), feeId);
      fRadialOutElx1->Fill(localPos.x(), feeId);
      fRadialOutEly1->Fill(localPos.z(), feeId);
	
      fRadialOutElpz2->Fill(hitPos.z(), feeId);
      fRadialOutElpz3->Fill(hitPos.z(), feeId, edep);
    }
  } else {
    fSmbNegZ->Fill(hitPos.z());
    fSmbZ[-100-smbNumber]->Fill(hitPos.z());
    fPlaneNegZ->SetBinContent(ix, iy, smbId);
    if (11 == apdgid) {
      fRadialOutElmz1->Fill(localPos.y(), feeId);
      fRadialOutElx1a->Fill(localPos.x(), feeId);
      fRadialOutEly1a->Fill(localPos.z(), feeId);

      fRadialOutElmz2->Fill(hitPos.z(), feeId);
      fRadialOutElmz3->Fill(hitPos.z(), feeId, edep);
    }
    // -- TODO: normalize to bin volume!
  }

  if (edep > 0) {
    G4double dose_c = 0;
    //  int pid = aStep->GetTrack()->GetParticleDefinition()->GetPDGEncoding();
    G4double cubicVolume = prePoint->GetPhysicalVolume()->GetLogicalVolume()->GetSolid()->GetCubicVolume();
    G4double density = prePoint->GetMaterial()->GetDensity();
    //G4cout << "DEBUG density " << density << " " << density/kg << " " << (density/kg * cubicVolume) << G4endl;
    dose_c    = edep/joule / ( density/kg * cubicVolume );
    dose_c *= prePoint->GetWeight();
    fDose[feeId] += dose_c;
  }

  return true;
}


// ----------------------------------------------------------------------
void Mu3eFibreSmbSD::writeStat() {
  gDirectory->mkdir("FibreSmb");
  gDirectory->cd("FibreSmb");

  double evendtime = (Mu3eEvent::GetInstance()->startTime + Mu3eEvent::GetInstance()->frameLength) /s;
  TH1F * hFibreSmbDose = new TH1F("hFibreSmbDose", TString::Format("Fibre Smb Dose (time = %f)", evendtime), 200, 0, 200);
  hFibreSmbDose->GetXaxis()->SetTitle("fibre Smb");
  hFibreSmbDose->GetYaxis()->SetTitle("dose/time [Gy/s]");
  for(unsigned int i = 0; i < 200; ++i) {
    hFibreSmbDose->SetBinContent(i+1, fDose[i]/evendtime);
  }
  hFibreSmbDose->Write();

  fPlanePosZ->Write();
  fPlaneNegZ->Write();

  for (int i = 0; i < 12; ++i) {
    fSmbZ[+100 + i]->Write();
    fSmbZ[-100 - i]->Write();
  }    
  
  fSmbPosZ->Write();
  fSmbNegZ->Write();
  
  fRadialOutElpz1->Write();
  fRadialOutElpz2->Write();
  fRadialOutElpz3->Write();

  fRadialOutElmz1->Write();
  fRadialOutElmz2->Write();
  fRadialOutElmz3->Write();

  fRadialOutElx1->Write();
  fRadialOutEly1->Write();

  fRadialOutElx1a->Write();
  fRadialOutEly1a->Write();
  
  gDirectory->cd("..");
}

// ----------------------------------------------------------------------
void Mu3eFibreSmbSD::EndOfEvent(G4HCofThisEvent*) {

}

// ----------------------------------------------------------------------
// http://ieeexplore.ieee.org/document/273529
// data from http://www.sr-niel.org/Simulation/516481niel_e.html
double Mu3eFibreSmbSD::damageFunction(double energy) {
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
const std::vector<double> Mu3eFibreSmbSD::df_e{
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
const std::vector<double> Mu3eFibreSmbSD::df_n {
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
