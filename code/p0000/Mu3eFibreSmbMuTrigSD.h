/// \file Mu3eFibreSmbMuTrigSD.h

#ifndef Mu3eFibreSmbMuTrigSD_h
#define Mu3eFibreSmbMuTrigSD_h

#include <G4AffineTransform.hh>
#include <G4VSensitiveDetector.hh>

#include "TH1F.h"
#include "TH2F.h"

class Mu3eFibreSmbMuTrigSD : public G4VSensitiveDetector {
public:
  Mu3eFibreSmbMuTrigSD(const G4String& name);
  virtual ~Mu3eFibreSmbMuTrigSD();

  void Initialize(G4HCofThisEvent*);

  void EndOfEvent(G4HCofThisEvent*);
  void writeStat();

  static double damageFunction(double e);

protected:
  G4bool ProcessHits(G4Step*, G4TouchableHistory*);

private:

  std::map<uint32_t, double> fDose;
  // damage function for electrons / 95 MeVmb (neutron equivalent)
  // http://www.sr-niel.org/Simulation/516481niel_e.html
  static const std::vector<double> df_e;
  static const std::vector<double> df_n;

  TH1F *fSmbMuTrigPosZ, *fSmbMuTrigNegZ;
  std::map<int, TH1F*> fSmbMuTrigZ; 
  
  TH2F *fSmbMuTrigPlanePosZ, *fSmbMuTrigPlaneNegZ;

  TH2F *fRadialOutElpz1, *fRadialOutElpz2, *fRadialOutElpz3;
  TH2F *fRadialOutElmz1, *fRadialOutElmz2, *fRadialOutElmz3;

  TH2F *fRadialOutElx1, *fRadialOutEly1;
  TH2F *fRadialOutElx1a, *fRadialOutEly1a;


};

#endif
