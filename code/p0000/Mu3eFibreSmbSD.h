/// \file Mu3eFibreSmbSD.h

#ifndef Mu3eFibreSmbSD_h
#define Mu3eFibreSmbSD_h

#include <G4AffineTransform.hh>
#include <G4VSensitiveDetector.hh>

#include "TH1F.h"
#include "TH2F.h"

class Mu3eFibreSmbSD : public G4VSensitiveDetector {
public:
  Mu3eFibreSmbSD(const G4String& name);
  virtual ~Mu3eFibreSmbSD();

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

  TH1F *fSmbPosZ, *fSmbNegZ;

  TH2F *fPlanePosZ, *fPlaneNegZ;
  TH2F *fRadialOutElpz1, *fRadialOutElpz2, *fRadialOutElpz3;
  TH2F *fRadialOutElmz1, *fRadialOutElmz2, *fRadialOutElmz3;

  TH2F *fRadialOutElx1, *fRadialOutEly1;
  TH2F *fRadialOutElx1a, *fRadialOutEly1a;


};

#endif
