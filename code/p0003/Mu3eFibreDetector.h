//

#pragma once

#include "Mu3eFibreRibbon.h"

#include <G4Tubs.hh>

namespace mu3e::sim {

  struct FibreDetector {

    G4LogicalVolume* volume;

    Ribbon* ribbon;

    double length;
    double rIn;
    double rOut;
    unsigned int nribbons;
    double fRibbonStagger;


  FibreDetector(Ribbon* ribbon_)
    : ribbon(ribbon_)
    {

      length = ribbon->length;
      nribbons = ribbon->nribbons;
      fRibbonStagger = ribbon->fStagger;
      
      // Fibre Detector dimensions
      // L' = (L_long + L_short)/2
      // R = L'/2 * 1/tan(2 pi/N_r) = L'/2 1/tan(pi/N)
      // R' = R - W/2
      double meanwidth = (ribbon->width + ribbon->widthtop) / 2.0 + ribbon->pitch;
      rIn = (meanwidth / 2.) / std::tan(M_PI / nribbons) - ribbon->height / 2. * ribbon->type;
      //- fibre->rOffset;
      rOut = sqrt(pow((rIn + ribbon->height), 2) + pow((ribbon->widthtop / 2.), 2));
      rIn -= ribbon->rOffset;

      G4VSolid* solidFibreDetector = new G4Tubs("fibreDetector", rIn, rOut, length / 2., 0, 2 * M_PI);

      volume = new G4LogicalVolume(solidFibreDetector, Mu3eMaterials::Instance().He, "fibreDetector");

      Mu3eConstruction::visVolume(volume, { 0.5, 0.5, 0.5 }, Mu3eConstruction::FIBRE);
      /// Place ribbons in detector
      PlaceRibbonsInDetector();
    }

    FibreDetector(const FibreDetector&) = delete;
    FibreDetector& operator=(const FibreDetector&) = delete;

  protected:
    void PlaceRibbonsInDetector() {
      G4RotationMatrix rotM = G4RotationMatrix();
      G4ThreeVector position = {};
      G4Transform3D transform;
      double phi(0.); 
      const double dphi = 2 * M_PI / nribbons;
      // -- y = 0 is between two ribbons. Therefore start with offset.
      rotM.rotateZ(dphi/2);
        
      printf("[FbDet] place %d ribbons: width = %.3f, height = %.3f, rIn = %.3f\n", nribbons, ribbon->width, ribbon->height, rIn);
      for(unsigned int i = 0; i < nribbons; ++i) {
        // -- legacy code
        //ignore  phi = i * dphi;

        // -- y = 0 is between two ribbons. Therefore start with offset.
        phi = dphi/2 + i * dphi;
        position =  {-std::sin(phi), std::cos(phi), 0.};
        if(i%2==1) position *= (rIn + ribbon->height/2.);
        else position *= ((rIn + ribbon->rOffset) + ribbon->height/2.);
        // -- add staggering
        if(i%2==0) {
          position.setZ(-fRibbonStagger/2); 
        } else {
          position.setZ(+fRibbonStagger/2); 
        }
        transform = G4Transform3D(rotM, position);
        if((i%2 == 1)&&( ribbon->type)) {
          new G4PVPlacement(transform,
                            ribbon->GetVolumeMirrow(),
                            "Ribbon",
                            volume,
                            false,
                            i,
                            0);
        } else {
          new G4PVPlacement(transform,
                            ribbon->logicalVolume,
                            "Ribbon",
                            volume,
                            false,
                            i,
                            0);
        }
        if(Mu3eDigiCfg::Instance().verbose >= 1) {
          G4cout << "Ribbon " << i << " (" << ribbon->width << ", " << ribbon->height << "," << ribbon->length << ") phi: " << phi
                 << " rIn(" << rIn << ") rho(" << position.getRho() << ")"
                 << " stagger/2 = " << fRibbonStagger/2
                 << " pos " << position
                 << G4endl;
          printf("    [%d] phi = %.3f, rho = %.3f\n", i, phi, position.getRho());
        }

        rotM.rotateZ(dphi);
      }
    }

  };

} // namespace mu3e::sim
