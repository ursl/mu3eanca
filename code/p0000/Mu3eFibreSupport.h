#ifndef MU3EFIBRESUPPORT_H_
#define MU3EFIBRESUPPORT_H_

#include <mu3e/util/g4.hpp>

#include "Mu3eFibreDetector.h"
#include "Mu3eConstruction.h"

#include <string>
#include <vector>

class G4LogicalVolume;
class G4AssemblyVolume;

namespace mu3e { namespace sim {

    struct FibreSupport {

      G4LogicalVolume* volume;
      G4LogicalVolume* fVolumeM;
      
      double length, MppcLength;
      double rIn;
      double rOut;
      double lengtPlateOffset;

    protected:
      G4LogicalVolume* volumeFibreSupportPlate;
      G4LogicalVolume* volumeFibreSupportHolder;
      G4LogicalVolume* volumeFibreMppcEpoxy;
      G4LogicalVolume* volumeFibreMppc;
      G4LogicalVolume* volumeFibreMppcSiO2;
      G4LogicalVolume* volumeFibreMppcPcb;
      G4LogicalVolume* fVolumeFibreSmbPcb[2];

      FibreDetector* detector;

      double MppcThickness, MppcThicknessEpoxy, MppcThicknessPcb;
      double MppcHeight, MppcWidth;
      // fibre detector support structure dimension
      double zOffset         =  6.25* CLHEP::mm;
      double PcbHeightTop    = 2.55 * CLHEP::mm;
      double PcbHeightBottom = 4.05 * CLHEP::mm;
      double rInSup          =   39 * CLHEP::mm;
      double lengthSup       =  165 * CLHEP::mm;
      double rPlate          =   15 * CLHEP::mm;
      //ul      double lengtPlate      =   50 * CLHEP::mm;
      double lengtPlate      =   55.5 * CLHEP::mm;

      // -- additions for the new SMB (as of mid 2022)
      G4LogicalVolume* fVolumeFibreSMBChip1;
      G4LogicalVolume* fVolumeFibreSMBChip2;
      G4LogicalVolume* fVolumeFibreSMBChip3;
      G4LogicalVolume* fVolumeFibreSMBConnector;

      /*
               +---------------------------------------+
               |                         +--+          |
               |           Chip3         |  |          |
        +------+                         +--+          |
        | C                                            |
        | o                              +--+          |
        | n     C                        |  |          |
        | n      h                       +--+          |
     (2)| e       i                                    | fSMBPcbWidth1
        | c        p                     +--+          |
        | t         1                    |  |          |
        | o                              +--+          |
        | r                                            |
        +------+                         +--+          |
           ^   |         Chip2           |  | (1)      |   (1) fSMBAsicDeltaFront
           |   |                         +--+          |   (2) fSMBPcbWidth2
           |   +---------------------------------------+
           |                fSMBPcbLength1
           |
           +--SMBPcbLength2

      */


  
      double fSMBPcbLength1     = 44.5   * CLHEP::mm;
      double fSMBPcbWidth1      = 26.0   * CLHEP::mm;
      double fSMBPcbThickness   =  1.005 * CLHEP::mm;
      double fSMBPcbLength2     = 11.0   * CLHEP::mm;
      double fSMBPcbWidth2      = 16.0   * CLHEP::mm;

      double fSMBAsicWidth      = 5.0  * CLHEP::mm;
      double fSMBAsicThickness  = 0.3  * CLHEP::mm;

      double fSMBChip1Width     = 5.7  * CLHEP::mm;
      double fSMBChip1Thickness = 0.75 * CLHEP::mm;
      double fSMBChip2Width     = 4.1  * CLHEP::mm;
      double fSMBChip2Thickness = 1.0  * CLHEP::mm;
      double fSMBChip3Width     = 3.1  * CLHEP::mm;
      double fSMBChip3Thickness = 1.0  * CLHEP::mm;

      double fSMBConnectorLength    = 13.1  * CLHEP::mm;
      double fSMBConnectorWidth     =  7.1  * CLHEP::mm;
      double fSMBConnectorThickness =  0.35 * CLHEP::mm;

      double fSMBAsicDeltaFront = 16.3 * CLHEP::mm;    // chip rhs wrt 55.5 (right edge)
      double fSMBAsicDeltaSide  = 1.05 * CLHEP::mm;  
      double fSMBAsicDeltaChip  = 1.30 * CLHEP::mm; 


      double fSMBChip1DeltaCenter = 42.79 * CLHEP::mm; // chip center wrt 55.5 (right edge)
    
      double fSMBChip2DeltaFront = 30.75 * CLHEP::mm;  // chip rhs wrt 55.5 (right edge)
      double fSMBChip2DeltaSide  = 0.25  * CLHEP::mm;

      double fSMBChip3DeltaFront = 28.95 * CLHEP::mm;  // chip rhs wrt 55.5 (right edge) 
      double fSMBChip3DeltaSide  = 22.45 * CLHEP::mm;

      double fSMBConnectorDelta  =  0.355* CLHEP::mm;  // connector separation wrt 0.0 (left edge)

    public:

      FibreSupport(FibreDetector* detector_, const Mu3eDetectorCfg& cfg, const Mu3eDigiCfg& digi);
      FibreSupport(const FibreSupport&) = delete;
      FibreSupport& operator=(const FibreSupport&) = delete;


    protected:
      // -- allow mirrored version for proper staggering of support components
      void PlaceMppcInSupport(G4LogicalVolume *, bool mirrored = false);
      
      G4AssemblyVolume* makeSmb(int index);
      void PlaceSmbInSupport(G4LogicalVolume *, bool mirrored = false);
    };
    
  } } // namespace mu3e::sim

#endif /* MU3EFIBRESUPPORT_H_ */
