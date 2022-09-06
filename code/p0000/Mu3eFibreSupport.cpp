#include "Mu3eFibreSupport.h"

#include <G4LogicalVolume.hh>
#include <G4MultiUnion.hh>
#include <G4UnionSolid.hh>
#include <G4AffineTransform.hh>
#include <G4SubtractionSolid.hh>
#include <G4AssemblyVolume.hh>

#include "Mu3eFibreMppcSD.h"
#include "Mu3eFibreSmbSD.h"

#include <G4Box.hh>
#include <G4Polycone.hh>
#include <G4SDManager.hh>

#include <mu3e/util/g4.hpp>

namespace mu3e {
  namespace sim {

    using namespace::std;
    
    // ----------------------------------------------------------------------
    FibreSupport::FibreSupport(FibreDetector* detector_, const Mu3eDetectorCfg& cfg, const Mu3eDigiCfg& digi)
      : detector(detector_)    {
      using CLHEP::mm;
      using CLHEP::nm;
      
      auto& materials = Mu3eMaterials::Instance();
      
      //only cfg.getKaptonOverlength() before which is not part of cfg anymore. Hope that the new dimensions are consistent
      // -- calculate total length of pixel layer 4
      length = (cfg.getLargeSensorLength() * mm) * (cfg.getZSensors4()) / 2.
        + (cfg.getLargeSensorZSpacing() * mm) * (cfg.getZSensors4() - 1) + cfg.getKaptonExtLengthOuter(); 
      std::cout << "DEBUG length " << length << " lengthSup = " << lengthSup << std::endl;
      if (lengthSup < length) length = lengthSup;
      length -=  detector->length/2.;
      lengtPlateOffset = lengtPlate - length;
      MppcThicknessEpoxy = cfg.getFibreMppcEpoxyThickness() * mm;
      MppcThickness = cfg.getFibreMppcThickness() * mm;
      MppcThicknessPcb = cfg.getFibreMppcPcbThickness() * mm;
      MppcHeight = cfg.getFibreMppcHeight() * mm;
      MppcWidth = cfg.getFibreMppcWidth() * mm;//detector->ribbon->;
      MppcLength = MppcThicknessEpoxy + MppcThickness + MppcThicknessPcb;
      //PcbHeightTop = cfg.getFibreMppcPcBHeightTop() * mm;
      //PcbHeightBottom = cfg.getFibreMppcPcBHeightBottom() * mm;
      rIn = rInSup;//detector->rIn - 6 * mm;
      rOut = detector->rOut + PcbHeightTop;

      const int n = 4;
      double zPlane[n] = { -(lengtPlate-length/2.), -length/2.,    -length/2., length/2.};
      double rInner[n] = { rIn,                     rIn,           rIn,        rIn};
      double rOuter[n] = { rIn + rPlate,            rIn + rPlate,  rOut,       rOut};


      G4VSolid* solidFibreSupport = new G4Polycone(
                                                   " fibreSupport",
                                                   0 * CLHEP::radian,
                                                   2.0 * M_PI * CLHEP::radian,
                                                   n, zPlane, rInner, rOuter);

      std::cout << "DEBUG length " << length << " ";
      std::cout << "rOut " << rOut << std::endl;

      volume = new G4LogicalVolume(   solidFibreSupport,
                                      materials.He,
                                      "fibreSupport");
      // -- mirror volume. This is upstream (US)!
      fVolumeM = new G4LogicalVolume(   solidFibreSupport,
                                        materials.He,
                                        "fibreSupport");

      G4VSolid* solidFibreSupportPlate = new G4Tubs(
                                                    "fibreSupportPlate",
                                                    rIn, rIn+rPlate,
                                                    lengtPlate/2.,
                                                    0, 2*M_PI);

      G4VSolid* solidFibreSupportHolder = new G4Tubs(
                                                     "fibreSupportHolder",
                                                     rIn+rPlate, rOut,
                                                     (length - MppcLength - zOffset)/2.,
                                                     0, 2*M_PI);

      if (1) volumeFibreSupportPlate = new G4LogicalVolume(
                                                           solidFibreSupportPlate,
                                                           materials.Al,
                                                           "fibreSupportPlate");
      
      if (1) volumeFibreSupportHolder = new G4LogicalVolume(
                                                            solidFibreSupportHolder,
                                                            materials.Al,
                                                            "fibreSupportHolder");
      
      G4VSolid* solidFibreMppcEpoxy = new G4Box(
                                                "fibreMppcEpoxy",
                                                MppcWidth/2., MppcHeight/2.,
                                                MppcThicknessEpoxy/2.);

      volumeFibreMppcEpoxy = new G4LogicalVolume(
                                                 solidFibreMppcEpoxy,
                                                 materials.getEpoxy(),
                                                 "fibreMppcEpoxy");

      G4VSolid* solidFibreMppc      = new G4Box(
                                                "fibreMppc",
                                                MppcWidth/2., MppcHeight/2.,
                                                MppcThickness/2.);

      G4VSolid* solidFibreMppcSiO2  = new G4Box(
                                                "fibreMppcSiO2",
                                                MppcWidth/2., MppcHeight/2.,
                                                100 * nm);

      volumeFibreMppc = new G4LogicalVolume(
                                            solidFibreMppc,
                                            materials.Si,
                                            "fibreMppc");

      volumeFibreMppcSiO2 = new G4LogicalVolume(
                                                solidFibreMppcSiO2,
                                                materials.SiO2,
                                                "fibreMppcSiO2");

      G4VSolid* solidFibreMppcPcb   = new G4Box(
                                                "fibreMppcPcb",
                                                MppcWidth/2., (PcbHeightTop + PcbHeightBottom)/2.,
                                                MppcThicknessPcb/2.);

      volumeFibreMppcPcb = new G4LogicalVolume(
                                               solidFibreMppcPcb,
                                               materials.Si,
                                               "fibreMppcPcb");

      if (1) new G4PVPlacement(nullptr,
                               {0, 0, -lengtPlate/2. + length/2.},    
                               volumeFibreSupportPlate,
                               "fibreSupportPlate",
                               volume,
                               false,
                               0);
      if (1) new G4PVPlacement(nullptr,
                               {0, 0, + MppcLength/2. + zOffset/2. - 10*mm},
                               volumeFibreSupportHolder,
                               "fibreSupportHolder",
                               volume,
                               false,
                               0);

      // -- mirror volume
      if (1) new G4PVPlacement(nullptr,
                               {0, 0, -lengtPlate/2. + length/2.},
                               volumeFibreSupportPlate,
                               "fibreSupportPlate",
                               fVolumeM,
                               false,
                               0);
      if (1) new G4PVPlacement(nullptr,
                               {0, 0, + MppcLength/2. + zOffset/2.},
                               volumeFibreSupportHolder,
                               "fibreSupportHolder",
                               fVolumeM,
                               false,
                               0);

      
      // place SiO2 in Mppc
      new G4PVPlacement(nullptr,
                        {0,0, - MppcThickness/2. + 50 * nm},
                        volumeFibreMppcSiO2,
                        "fibreMppcSiO2",
                        volumeFibreMppc,
                        false,
                        0);
      //G4cout << "Ribbon " << i << " added at phi: " << phi << " at radius " << position.getRho() << G4endl;

      PlaceMppcInSupport(volume);
      PlaceSmbInSupport(volume);

      PlaceMppcInSupport(fVolumeM, true);
      PlaceSmbInSupport(fVolumeM, true);
      
      if(digi.isWriteFibres() >= 3) {
        auto fibreMppcSD = new Mu3eFibreMppcSD("mu3e/FibreMppcSD");
        G4SDManager::GetSDMpointer()->AddNewDetector(fibreMppcSD);
        volumeFibreMppc->SetSensitiveDetector(fibreMppcSD);
        volumeFibreMppcSiO2->SetSensitiveDetector(fibreMppcSD);
      }
      
      if (1) Mu3eConstruction::visVolume(volumeFibreSupportPlate, {0.5,0.5,0.5},
                                         Mu3eConstruction::PHYSICAL|
                                         Mu3eConstruction::FIBRE|
                                         Mu3eConstruction::SUPPORT);
      if (1)       Mu3eConstruction::visVolume(volumeFibreSupportHolder, {0.5,0.7,0.5},
                                               Mu3eConstruction::PHYSICAL|
                                               Mu3eConstruction::FIBRE|
                                               Mu3eConstruction::SUPPORT);
      Mu3eConstruction::visVolume(volumeFibreMppc, {0.8,0.0,0.5},
                                  Mu3eConstruction::PHYSICAL|
                                  Mu3eConstruction::FIBRE|
                                  Mu3eConstruction::ACTIVE);

      Mu3eConstruction::visVolume(volumeFibreMppcEpoxy, {0.8,0.4,0.5},
                                  Mu3eConstruction::PHYSICAL|
                                  Mu3eConstruction::FIBRE);
      Mu3eConstruction::visVolume(volumeFibreMppcSiO2, {0.8,0.4,0.0},
                                  Mu3eConstruction::PHYSICAL|
                                  Mu3eConstruction::FIBRE);
      Mu3eConstruction::visVolume(volumeFibreMppcPcb, {0.0,0.7,0.0},
                                  Mu3eConstruction::PHYSICAL|
                                  Mu3eConstruction::FIBRE);
      Mu3eConstruction::visVolume(fVolumeFibreSmbPcb[0], {0.8,0.2,0.4},
                                  Mu3eConstruction::PHYSICAL|
                                  Mu3eConstruction::FIBRE);
      Mu3eConstruction::visVolume(fVolumeFibreSmbPcb[1], {0.2,0.2,0.8},
                                  Mu3eConstruction::PHYSICAL|
                                  Mu3eConstruction::FIBRE);
      // Mu3eConstruction::visVolume(fVolumeFibreSmb, {0.8,0.8,0.8},
      //                             Mu3eConstruction::PHYSICAL|
      //                             Mu3eConstruction::FIBRE);
      
      Mu3eConstruction::visVolume(volume, {1,1,1},Mu3eConstruction::FIBRE);
      Mu3eConstruction::visVolume(fVolumeM, {1,1,1},Mu3eConstruction::FIBRE);


    }


    // ----------------------------------------------------------------------
    void FibreSupport::PlaceMppcInSupport(G4LogicalVolume *vol, bool mirrored) {
      using CLHEP::mm;
      G4RotationMatrix rotM = G4RotationMatrix();
      G4ThreeVector position = {};
      G4ThreeVector positionPcb = {};
      G4Transform3D transform;

      double phi;
      double dphi = 2 * M_PI / detector->nribbons;
      // -- y = 0 is between two ribbons. Therefore start with offset. Mirror this here as well.
      rotM.rotateZ(dphi/2);
      for(unsigned int i = 0; i < detector->nribbons; ++i) {
        phi = dphi/2 + i * dphi;
        position =  {-std::sin(phi), std::cos(phi), 0};
        if(i%2==1) {
          positionPcb = position * (detector->rIn + detector->ribbon->height/2.
                                    + (-PcbHeightBottom + PcbHeightTop)/2. );
          position *= (detector->rIn + detector->ribbon->height/2.);

        } else {
          positionPcb = position * ((detector->rIn + detector->ribbon->rOffset)
                                    + detector->ribbon->height/2.
                                    + (-PcbHeightBottom + PcbHeightTop)/2. );
          position *= ((detector->rIn + detector->ribbon->rOffset)
                       + detector->ribbon->height/2.);
	  
        }
        if(i%2==1) {
          position.setZ(length/2. + MppcThicknessEpoxy/2. - MppcLength/2. - 6.5*mm);
        } else {
          position.setZ(length/2. + MppcThicknessEpoxy/2. - MppcLength/2. - 19.0*mm);
        }
        transform = G4Transform3D(rotM, position);
        G4ThreeVector pMax, pMin;
        vol->GetSolid()->BoundingLimits(pMin, pMax);

        G4cout << "fibreMPPC " << i << " added at phi: " << phi
               << " at z " << pMin.getZ() << " " << pMax.getZ() << G4endl;
        // -- Epoxy
        new G4PVPlacement(transform,
                          volumeFibreMppcEpoxy,
                          "fibreMppcEpoxy",
                          vol,
                          false,
                          i,
                          0);
        position.setZ(position.z() + MppcThicknessEpoxy/2. + MppcThickness/2.);
        transform = G4Transform3D(rotM, position);
        // -- Mppc
        new G4PVPlacement(transform,
                          volumeFibreMppc,
                          "fibreMppc",
                          vol,
                          false,
                          i,
                          0);
        positionPcb.setZ(position.z() + MppcThickness/2. + MppcThicknessPcb/2.);
        transform = G4Transform3D(rotM, positionPcb);
        // -- Mppc Pcb
        new G4PVPlacement(transform,
                          volumeFibreMppcPcb,
                          "fibreMppcPcb",
                          vol,
                          false,
                          i,
                          0);
        rotM.rotateZ(dphi);
      }
    }


    // ----------------------------------------------------------------------
    void FibreSupport::PlaceSmbInSupport(G4LogicalVolume *vol, bool mirrored) {
      G4RotationMatrix rotM = G4RotationMatrix();
      G4ThreeVector position = {};
      G4ThreeVector positionPcb = {};
      G4Transform3D transform;

      rotM.rotateY(M_PI*CLHEP::rad);

      double phi;
      double dphi = 2 * M_PI / detector->nribbons;
      //      if (mirrored) dphi *= -1.;
      // -- y = 0 is between two ribbons. Therefore start with offset. Mirror this here as well.
      rotM.rotateX(+M_PI/2*CLHEP::rad);
      rotM.rotateY(0.*CLHEP::rad);
      rotM.rotateZ(dphi/2);
      int index = (mirrored?1:0);
      G4AssemblyVolume *solidFibreSmb = makeSmb(index);
     
      for(unsigned int i = 0; i < detector->nribbons; ++i) {
        phi = dphi/2 + i * dphi;
        position =  {-std::sin(phi), std::cos(phi), 0};
        positionPcb = position * (rInSup + rPlate);
        positionPcb.setZ(position.z() - length/2.  - 1.3*CLHEP::cm);
        transform = G4Transform3D(rotM, positionPcb);
        solidFibreSmb->MakeImprint(vol, transform);
        rotM.rotateZ(dphi);
      }

      // -- rename entities
      vector<G4VPhysicalVolume*>::iterator ipv = solidFibreSmb->GetVolumesIterator();
      unsigned int nipv = solidFibreSmb->GetImprintsCount();
      cout << "solidFibreSmb->GetImprintsCount() = " << solidFibreSmb->GetImprintsCount() << endl;
      string simpr("SMB");
      char ssmb[100], sasic[100];
      int nSmb(-1);
      int DBX(1); 
      if (DBX) cout << "Renaming components for mirrored = " << mirrored
                    << ", i.e. simpr = " << simpr
                    << endl;
      while (1) {
        string sname = (*ipv)->GetName(); 
        // -- SMB PCB
        // -- numerology non-trivial because of rotation of mirrored volume (in Mu3eFibreTrackerConstruction)
        if (string::npos != sname.find("Pcb_")) {
          double phi = (*ipv)->GetTranslation().phi()*57.2957795131; 
          if (phi < 0) phi += 360.;
          if (mirrored) {
            nSmb = 8 - static_cast<int>(phi)/30;
            if (nSmb > 11) nSmb -= 12;
            if (nSmb < 0) nSmb += 12;
          } else {
            nSmb = 9 + static_cast<int>(phi)/30;
            if (nSmb > 11) nSmb -= 12;
          }
          sprintf(ssmb, "%s_%d", simpr.c_str(), nSmb);
          if (DBX) cout << "*ipv->GetName() = " << sname << " trsl = "
                        << (*ipv)->GetTranslation()
                        << " phi = " << phi
                        << " -> nSmb = " << nSmb
                        << " -> ssmb = " << ssmb
                        << endl;
          (*ipv)->SetName(ssmb); 
          (*ipv)->GetLogicalVolume()->SetName(ssmb); 
          (*ipv)->GetLogicalVolume()->SetName(ssmb); 
          ++ipv;
          sname = (*ipv)->GetName();
        }

        // -- MuTrigs
        if (string::npos != sname.find("Asic_")) {
          if (mirrored) {
            int iasic(0);
            sprintf(sasic, "%s_%d_ASIC_%d", simpr.c_str(), nSmb, iasic++);
            if (DBX) cout << "*ipv->GetName() = " << sname << " trsl = "
                          << (*ipv)->GetTranslation()
                          << " -> sasic = " << sasic
                          << endl;
            (*ipv)->SetName(sasic); 
            
            ++ipv;
            sname = (*ipv)->GetName();
            sprintf(sasic, "%s_%d_ASIC_%d", simpr.c_str(), nSmb, iasic++);
            if (DBX) cout << "*ipv->GetName() = " << sname << " trsl = "
                          << (*ipv)->GetTranslation()
                          << " -> sasic = " << sasic
                          << endl;
            (*ipv)->SetName(sasic); 
        
            ++ipv;
            sname = (*ipv)->GetName();
            sprintf(sasic, "%s_%d_ASIC_%d", simpr.c_str(), nSmb, iasic++);
            if (DBX) cout << "*ipv->GetName() = " << sname << " trsl = "
                          << (*ipv)->GetTranslation()
                          << " -> sasic = " << sasic
                          << endl;
            (*ipv)->SetName(sasic); 
        
            ++ipv;
            sname = (*ipv)->GetName();
            sprintf(sasic, "%s_%d_ASIC_%d", simpr.c_str(), nSmb, iasic++);
            if (DBX) cout << "*ipv->GetName() = " << sname << " trsl = "
                          << (*ipv)->GetTranslation()
                          << " -> sasic = " << sasic
                          << endl;
            (*ipv)->SetName(sasic); 
          } else {
            int iasic(3);
            sprintf(sasic, "%s_%d_ASIC_%d", simpr.c_str(), nSmb, iasic--);
            if (DBX) cout << "*ipv->GetName() = " << sname << " trsl = "
                          << (*ipv)->GetTranslation()
                          << " -> sasic = " << sasic
                          << endl;
            (*ipv)->SetName(sasic); 
        
            ++ipv;
            sname = (*ipv)->GetName();
            sprintf(sasic, "%s_%d_ASIC_%d", simpr.c_str(), nSmb, iasic--);
            if (DBX) cout << "*ipv->GetName() = " << sname << " trsl = "
                          << (*ipv)->GetTranslation()
                          << " -> sasic = " << sasic
                          << endl;
            (*ipv)->SetName(sasic); 
        
            ++ipv;
            sname = (*ipv)->GetName();
            sprintf(sasic, "%s_%d_ASIC_%d", simpr.c_str(), nSmb, iasic--);
            if (DBX) cout << "*ipv->GetName() = " << sname << " trsl = "
                          << (*ipv)->GetTranslation()
                          << " -> sasic = " << sasic
                          << endl;
            (*ipv)->SetName(sasic); 
        
            ++ipv;
            sname = (*ipv)->GetName();
            sprintf(sasic, "%s_%d_ASIC_%d", simpr.c_str(), nSmb, iasic--);
            if (DBX) cout << "*ipv->GetName() = " << sname << " trsl = "
                          << (*ipv)->GetTranslation()
                          << " -> sasic = " << sasic
                          << endl;
            (*ipv)->SetName(sasic); 
          }

          ++ipv;
          sname = (*ipv)->GetName();
        }

        // -- remaining chips
        if (string::npos != sname.find("Chip")) {
          int ichip(0);
          sprintf(sasic, "%s_%d_CHIP_%d", simpr.c_str(), nSmb, ichip++);
          if (DBX) cout << "*ipv->GetName() = " << sname << " trsl = "
                        << (*ipv)->GetTranslation()
                        << " -> sasic = " << sasic
                        << endl;
          (*ipv)->SetName(sasic); 
          ++ipv;
          sname = (*ipv)->GetName();

          sprintf(sasic, "%s_%d_CHIP_%d", simpr.c_str(), nSmb, ichip++);
          if (DBX) cout << "*ipv->GetName() = " << sname << " trsl = "
                        << (*ipv)->GetTranslation()
                        << " -> sasic = " << sasic
                        << endl;
          (*ipv)->SetName(sasic); 
          ++ipv;
          sname = (*ipv)->GetName();

          sprintf(sasic, "%s_%d_CHIP_%d", simpr.c_str(), nSmb, ichip++);
          if (DBX) cout << "*ipv->GetName() = " << sname << " trsl = "
                        << (*ipv)->GetTranslation()
                        << " -> sasic = " << sasic
                        << endl;
          (*ipv)->SetName(sasic); 

          ++ipv;
          sname = (*ipv)->GetName();
        }

        // -- final components
        if (string::npos != sname.find("Connector_")) {
          sprintf(sasic, "%s_%d_CONN", simpr.c_str(), nSmb);
          if (DBX) cout << "*ipv->GetName() = " << sname << " trsl = "
                        << (*ipv)->GetTranslation()
                        << " -> sasic = " << sasic
                        << endl;
          (*ipv)->SetName(sasic); 
        }

        // -- exit loop
        if (11 == nSmb) {
          break;
        } else {
          ++ipv;
        }
      }

      
    }


    // ----------------------------------------------------------------------
    G4AssemblyVolume* FibreSupport::makeSmb(int index) {
      using CLHEP::mm;
      using CLHEP::radian;
      
      auto& materials = Mu3eMaterials::Instance();

      G4RotationMatrix rotm = G4RotationMatrix();

      // -- the large base
      G4VSolid* solidFibreSMBPcb1   = new G4Box("fibreSMBPcb1",
                                                fSMBPcbWidth1/2., fSMBPcbLength1/2.,
                                                fSMBPcbThickness/2.);
  
      // -- the smaller tail with the connector (interposer?)
      G4VSolid* solidFibreSMBPcb2   = new G4Box("fibreSMBPcb2",
                                                fSMBPcbWidth2/2., fSMBPcbLength2/2.,
                                                fSMBPcbThickness/2.);
  
      // -- the MuTRIG ASIC
      G4VSolid* solidFibreSMBAsic   = new G4Box("fibreSMBAsic",
                                                fSMBAsicWidth/2., fSMBAsicWidth/2.,
                                                fSMBAsicThickness/2.);
      // -- rotated chip next to the interposer
      G4VSolid* solidFibreSMBChip1   = new G4Box("fibreSMBChip1",
                                                 fSMBChip1Width/2., fSMBChip1Width/2.,
                                                 fSMBChip1Thickness/2.);

      // -- chip with the two round structures *right next* to it
      G4VSolid* solidFibreSMBChip2   = new G4Box("fibreSMBChip2",
                                                 fSMBChip2Width/2., fSMBChip2Width/2.,
                                                 fSMBChip2Thickness/2.);

      // -- chip with the two round structures at two different edges
      G4VSolid* solidFibreSMBChip3   = new G4Box("fibreSMBChip3",
                                                 fSMBChip3Width/2., fSMBChip3Width/2.,
                                                 fSMBChip3Thickness/2.);

      // -- Connector at narrow end
      G4VSolid* solidFibreSMBConnector   = new G4Box("fibreSMBConnector",
                                                     fSMBConnectorLength/2., fSMBConnectorWidth/2.,
                                                     fSMBConnectorThickness/2.);

  
      // -- create PCB (non-rectangular) shape
      G4RotationMatrix* yRot = new G4RotationMatrix(); 
      G4ThreeVector zTrans(0., 0.5*(fSMBPcbLength1+fSMBPcbLength2), 0.);
      G4UnionSolid* solidFibreSMBPcb = new G4UnionSolid("solidFibreSMBPcb",
                                                        solidFibreSMBPcb1, 
                                                        solidFibreSMBPcb2,
                                                        yRot,
                                                        zTrans);
  
      fVolumeFibreSmbPcb[index] = new G4LogicalVolume(solidFibreSMBPcb,
                                               materials.Kapton,
                                               "fibreSMBPcb");

      G4VisAttributes *pVA  = new G4VisAttributes;
      if (0 == index) {
        pVA->SetColour(G4Colour(0.8, 0.2, 0.4, 0.5));
      } else {
        pVA->SetColour(G4Colour(0.2, 0.2, 0.8, 0.5));
      }
      pVA->SetForceSolid(true);
      fVolumeFibreSmbPcb[index]->SetVisAttributes(pVA);

      G4SDManager *sdManager = G4SDManager::GetSDMpointer();
      G4VSensitiveDetector *sdSmb = sdManager->FindSensitiveDetector("mu3e/FibreSmbSD");
      cout << "sdSmb = " << sdSmb << endl;
      fVolumeFibreSmbPcb[index]->SetSensitiveDetector(sdSmb); 
    
  
      // -- create complete board as assembly, first PCB
      G4RotationMatrix Ra;
      G4ThreeVector Ta;
      G4Transform3D Tr;
      G4AssemblyVolume* solidFibreSMB = new G4AssemblyVolume();
  
      //  Ra.rotateZ(M_PI*radian);
      Ra.rotateZ(0.);
  
      Ta.setX(0.);
      Ta.setY(0.);
      Ta.setZ(0.);
      Tr = G4Transform3D(Ra,Ta);
      solidFibreSMB->AddPlacedVolume(fVolumeFibreSmbPcb[index], Tr);
  
      // -- add 4 asics
      for (unsigned int i = 0; i < 4; ++i) {
        G4LogicalVolume* pa = new G4LogicalVolume(solidFibreSMBAsic,
                                                     materials.Si,
                                                     "fibreSMBAsic");
       
        G4VisAttributes *pVA2  = new G4VisAttributes;
        pVA2->SetColour(G4Colour(0., 0., 0.));
        pVA2->SetForceSolid(true);
        pa->SetVisAttributes(pVA2);
    
        Ta.setX(0.5*(fSMBPcbWidth1 - fSMBAsicWidth - 2.*fSMBAsicDeltaSide) - i*(fSMBAsicWidth + fSMBAsicDeltaChip));
        Ta.setY(-0.5*(fSMBPcbLength1 - fSMBAsicWidth) + fSMBAsicDeltaFront);
        Ta.setZ(0.5*(fSMBPcbThickness + fSMBAsicThickness));
    
        Tr = G4Transform3D(rotm, Ta);
        solidFibreSMB->AddPlacedVolume(pa, Tr);
      }



  
      // -- add chip #2
      fVolumeFibreSMBChip2 = new G4LogicalVolume(solidFibreSMBChip2,
                                                 materials.Si,
                                                 "fibreSMBChip2");
  
      G4VisAttributes *pVA2  = new G4VisAttributes;
      pVA2->SetColour(G4Colour(0., 0., 0.));
      pVA2->SetForceSolid(true);
      fVolumeFibreSMBChip2->SetVisAttributes(pVA2);
  
      Ta.setX(0.5*(-fSMBPcbWidth1 + fSMBChip2Width + 2.*fSMBChip2DeltaSide));
      Ta.setY(-0.5*(fSMBPcbLength1 - fSMBChip2Width) + fSMBChip2DeltaFront);
      Ta.setZ(0.5*(fSMBPcbThickness + fSMBChip2Thickness));
  
      Tr = G4Transform3D(rotm, Ta);
      solidFibreSMB->AddPlacedVolume(fVolumeFibreSMBChip2, Tr);


      // -- add chip #3
      fVolumeFibreSMBChip3 = new G4LogicalVolume(solidFibreSMBChip3,
                                                 materials.Si,
                                                 "fibreSMBChip3");
  
      G4VisAttributes *pVA3  = new G4VisAttributes;
      pVA3->SetColour(G4Colour(0., 0., 0.));
      pVA3->SetForceSolid(true);
      fVolumeFibreSMBChip3->SetVisAttributes(pVA3);
  
      Ta.setX(0.5*(-fSMBPcbWidth1 + fSMBChip3Width + 2.*fSMBChip3DeltaSide));
      Ta.setY(-0.5*(fSMBPcbLength1 - fSMBChip3Width) + fSMBChip3DeltaFront);
      Ta.setZ(0.5*(fSMBPcbThickness + fSMBChip3Thickness));
  
      Tr = G4Transform3D(rotm, Ta);
      solidFibreSMB->AddPlacedVolume(fVolumeFibreSMBChip3, Tr);


      // -- add chip #1
      fVolumeFibreSMBChip1 = new G4LogicalVolume(solidFibreSMBChip1,
                                                 materials.Si,
                                                 "fibreSMBChip1");
  
      G4VisAttributes *pVA1  = new G4VisAttributes;
      pVA1->SetColour(G4Colour(0., 0., 0.));
      pVA1->SetForceSolid(true);
      fVolumeFibreSMBChip1->SetVisAttributes(pVA1);
  
      Ta.setX(0.);
      Ta.setY(-0.5*fSMBPcbLength1 + fSMBChip1DeltaCenter);
      //  Ta.setY(-0.5*(fSMBPcbLength1) + fSMBChip1DeltaFront);
      Ta.setZ(0.5*(fSMBPcbThickness + fSMBChip1Thickness));
  
      rotm.rotateZ(-M_PI/4*CLHEP::rad);
      Tr = G4Transform3D(rotm, Ta);
      solidFibreSMB->AddPlacedVolume(fVolumeFibreSMBChip1, Tr);

      // -- add connector
      fVolumeFibreSMBConnector = new G4LogicalVolume(solidFibreSMBConnector,
                                                     materials.Si,
                                                     "fibreSMBConnector");
  
      G4VisAttributes *pVA4  = new G4VisAttributes;
      pVA4->SetColour(G4Colour(0.7, 0.7, 0.7));
      pVA4->SetForceSolid(true);
      fVolumeFibreSMBConnector->SetVisAttributes(pVA4);
  
      Ta.setX(0.);
      Ta.setY(0.5*fSMBPcbLength1 + fSMBPcbLength2 - 0.5*fSMBConnectorWidth - fSMBConnectorDelta);
      Ta.setZ(0.5*(fSMBPcbThickness + fSMBConnectorThickness));
  
      // -- rotate back
      rotm.rotateZ(+M_PI/4*CLHEP::rad);
      Tr = G4Transform3D(rotm, Ta);
      solidFibreSMB->AddPlacedVolume(fVolumeFibreSMBConnector, Tr);

      return solidFibreSMB;

    }
  
    
    // ----------------------------------------------------------------------
    //   G4AssemblyVolume* FibreSupport::makeSmb0() {
    //     using CLHEP::mm;
    //     using CLHEP::radian;
      
    //     auto& materials = Mu3eMaterials::Instance();

    //     G4VSolid* solidFibreSmbPcb1   = new G4Box("fibreSmbPcb1",
    //                                               fSmbPcbWidth1/2., fSmbPcbLength1/2.,
    //                                               fSmbPcbThickness/2.);

    //     G4MultiUnion* fourHoles = new G4MultiUnion("FourHoles");
    //     G4Tubs *hole = new G4Tubs("hole", 0, 1*mm, fSmbPcbThickness+2*mm, 0, 2.*M_PI*radian);
    //     G4RotationMatrix rotm = G4RotationMatrix();
    //     G4Transform3D tr1 = G4Transform3D(rotm, G4ThreeVector(-11, -19, -1));
    //     fourHoles->AddNode(*hole, tr1);
    //     G4Transform3D tr2 = G4Transform3D(rotm, G4ThreeVector(-11, 19, -1));
    //     fourHoles->AddNode(*hole, tr2);
    //     G4Transform3D tr3 = G4Transform3D(rotm, G4ThreeVector(+11, -19, -1));
    //     fourHoles->AddNode(*hole, tr3);
    //     G4Transform3D tr4 = G4Transform3D(rotm, G4ThreeVector(+11, 19, -1));
    //     fourHoles->AddNode(*hole, tr4);
  
    //     G4SubtractionSolid *subtraction = new G4SubtractionSolid("fibreSmbPcb11", solidFibreSmbPcb1, fourHoles);

  
    //     G4VSolid* solidFibreSmbPcb2   = new G4Box("fibreSmbPcb2",
    //                                               fSmbPcbWidth2/2., fSmbPcbLength2/2.,
    //                                               fSmbPcbThickness/2.);
  
    //     G4VSolid* solidFibreSmbAsic   = new G4Box("fibreSmbAsic",
    //                                               fSmbAsicWidth/2., fSmbAsicLength/2.,
    //                                               fSmbAsicThickness/2.);
  
  
    //     // -- create PCB (non-rectangular) shape
    //     G4RotationMatrix* yRot = new G4RotationMatrix(); 
    //     G4ThreeVector zTrans(0., 0.5*(fSmbPcbLength1+fSmbPcbLength2), 0.);
    //     G4UnionSolid* solidFibreSmbPcb = new G4UnionSolid("solidFibreSmbPcb",
    //                                                       //subtraction, 
    //                                                       solidFibreSmbPcb1,
    //                                                       solidFibreSmbPcb2,
    //                                                       yRot,
    //                                                       zTrans);
  
    //     fVolumeFibreSmbPcb = new G4LogicalVolume(solidFibreSmbPcb,
    //                                              materials.Kapton,
    //                                              "fibreSmbPcb");

    //     G4VisAttributes *pVA1  = new G4VisAttributes;
    //     pVA1->SetColour(G4Colour(0.8, 0.2, 0.4, 0.5));
    //     pVA1->SetForceSolid(true);
    //     fVolumeFibreSmbPcb->SetVisAttributes(pVA1);

  
    //     // -- create complete board as assembly, first PCB
    //     G4RotationMatrix Ra;
    //     G4ThreeVector Ta;
    //     G4Transform3D Tr;
    //     G4AssemblyVolume* solidFibreSmb = new G4AssemblyVolume();
  
    //     Ra.rotateZ(M_PI*radian);
  
    //     Ta.setX(0.);
    //     Ta.setY(0.);
    //     Ta.setZ(0.);
    //     Tr = G4Transform3D(Ra,Ta);
    //     solidFibreSmb->AddPlacedVolume(fVolumeFibreSmbPcb, Tr);
  
    //     // -- add asics
    //     for (unsigned int i = 0; i < 4; ++i) {
    //       fVolumeFibreSmbAsic[i] = new G4LogicalVolume(solidFibreSmbAsic,
    //                                                    materials.Si,
    //                                                    "fibreSmbAsic");
    
    //       G4VisAttributes *pVA2  = new G4VisAttributes;
    //       pVA2->SetColour(G4Colour(0., 0., 0.));
    //       pVA2->SetForceSolid(true);
    //       fVolumeFibreSmbAsic[i]->SetVisAttributes(pVA2);
	
    //       G4SDManager *sdManager = G4SDManager::GetSDMpointer();
    //       G4VSensitiveDetector *sdSmb = sdManager->FindSensitiveDetector("mu3e/FibreSmbSD");
    //       cout << "sdSmb = " << sdSmb << endl;
    //       fVolumeFibreSmbAsic[i]->SetSensitiveDetector(sdSmb); 
	
    //       //    Ta.setX(-0.5*fSmbPcbWidth1 + 0.5*(fSmbAsicDeltaSide + fSmbAsicWidth) + i*(fSmbAsicWidth + fSmbAsicDeltaChip)); 
    //       Ta.setX(0.5*(-fSmbPcbWidth1 + fSmbAsicDeltaSide + fSmbAsicWidth) + i*(fSmbAsicWidth + fSmbAsicDeltaChip));
    //       Ta.setY(0.5*fSmbPcbLength1 - 0.5*fSmbAsicLength - fSmbAsicDeltaFront);
    //       Ta.setZ(0.5*(fSmbPcbThickness + fSmbAsicThickness));
    
    //       Tr = G4Transform3D(rotm, Ta);
    //       solidFibreSmb->AddPlacedVolume(fVolumeFibreSmbAsic[i], Tr);
    //     }

    //     return solidFibreSmb;
    //   }
     
  }
} // namespace mu3e::sim

