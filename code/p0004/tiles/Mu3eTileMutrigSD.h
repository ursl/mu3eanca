/// \file Mu3eTileMutrigSD.h

/* Niklaus Berger, niberger@uni-mainz.de
 * December 2019
 */

#ifndef mu3eTileMutrigSD_h
#define mu3eTileMutrigSD_h

#include <G4AffineTransform.hh>
#include <G4VSensitiveDetector.hh>

class G4Step;
class TH1F;

/// Class representing the scintillating tile detector
class Mu3eTileMutrigSD : public G4VSensitiveDetector {
public:
    Mu3eTileMutrigSD(
        const G4String& name,
        int writeTiles
    );

    virtual ~Mu3eTileMutrigSD() {
    }

    /// Initialize hit collection
    void Initialize(G4HCofThisEvent*);
    /// Process a hit
    G4bool ProcessHits(G4Step*, G4TouchableHistory*);

    /// Dummy
    void EndOfEvent(G4HCofThisEvent*);

    /// Write histograms
    void writeStat();
private:
    static TH1F * tileasicdose;
    static TH1F * tileasicnieldose;

  std::map<int, TH1F*> fhmap;
  std::map<int, TH1F*> femap;
  std::map<int, TH1F*> fetotmap;

protected:
    /// Write mode
    int write_mode;

};

#endif
