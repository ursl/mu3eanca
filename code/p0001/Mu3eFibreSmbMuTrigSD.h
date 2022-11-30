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

  std::map<std::string, double> fDose;
  // damage function for electrons / 95 MeVmb (neutron equivalent)
  // http://www.sr-niel.org/Simulation/516481niel_e.html
  static const std::vector<double> df_e;
  static const std::vector<double> df_n;

  const bool fFillDbxHist = false;
  
  TH1F *fMuTrigEdep, *fMuTrigEdepCombined;

  TH1F *fSmbMuTrigPosZ, *fSmbMuTrigNegZ;
  TH2F *fSmbMuTrigNegZ_xy, *fSmbMuTrigPosZ_xy;
  std::map<std::string, TH1F*> fSmbMuTrigGZ, fSmbMuTrigEdep, fSmbMuTrigLZ; 
  std::map<std::string, TH2F*> fSmbMuTrigLXY, fSmbMuTrigGXY;
  
  TH2F *fSmbMuTrigPlanePosZ, *fSmbMuTrigPlaneNegZ;

};

#endif
