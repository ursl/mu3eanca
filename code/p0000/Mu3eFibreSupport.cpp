#include "Mu3eFibreSupport.h"

#include <G4LogicalVolume.hh>
#include <G4MultiUnion.hh>
#include <G4UnionSolid.hh>
#include <G4AffineTransform.hh>
#include <G4SubtractionSolid.hh>
#include <G4AssemblyVolume.hh>

#include "Mu3eFibreMppcSD.h"
#include "Mu3eFibreSmbSD.h"
#include "Mu3eFibreSmbMuTrigSD.h"

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
      // -- calculate total length of pixel layer 3
      length = (cfg.getLargeSensorLength() * mm) * (cfg.getZSensors3()) / 2.
        + (cfg.getLargeSensorZSpacing() * mm) * (cfg.getZSensors3() - 1) + cfg.getKaptonExtLengthOuter(); 
      std::cout << "DEBUG length " << length << " lengthSup = " << lengthSup << std::endl;
      if (lengthSup < length) length = lengthSup;
      length -=  detector->length/2.;
      lengtPlateOffset = lengtPlate - length; // -- useless and not used
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

      double rExtra(3.0);
      const int n = 4;
      double zPlane[n] = {
        -(lengtPlate-length/2.) - 20.,
        -length/2.,
        -length/2.,
        length/2. + 20.
      };
      double rInner[n] = { rIn,                           rIn,                   rIn,        rIn};
      double rOuter[n] = { rIn + rPlate + rExtra,         rIn + rPlate + rExtra, rOut,       rOut};


      G4VSolid* solidFibreSupport = new G4Polycone(
                                                   "fibreSupport",
                                                   0 * CLHEP::radian,
                                                   2.0 * M_PI * CLHEP::radian,
                                                   n, zPlane, rInner, rOuter);

      cout << "----------------------------------------------------------------------" << endl;
      cout << "DEBUG length " << length  << " rOut " << rOut << " lengtPlate = " << lengtPlate << endl;
      cout << "zPlane: " << zPlane[0] << ", " << zPlane[1] << ", " << zPlane[2] << ", " << zPlane[3]<< endl;
      cout << "rInner: " << rInner[0] << endl;
      cout << "rOuter: " << rOuter[0] << ", "  << rOuter[1] << ", "  << rOuter[2] << ", "  << rOuter[3] << endl;

      volume = new G4LogicalVolume(   solidFibreSupport,
                                      materials.He,
                                      "fibreSupport");
      // -- mirror volume. This is upstream (US)!
      fVolumeM = new G4LogicalVolume(   solidFibreSupport,
                                        materials.He,
                                        "fibreSupport");

      // -- SMB are mounted on this
      G4VSolid* solidFibreSupportPlate = new G4Tubs(
                                                    "fibreSupportPlate",
                                                    rIn, rIn+rPlate,
                                                    lengtPlate/2.,
                                                    0, 2*M_PI);


      cout << "solidFibreSupportPlate: lengtPlate/2. = " << lengtPlate/2. << endl;
      
      // -- the green ring outside of support plate
      G4VSolid* solidFibreSupportHolder = new G4Tubs(
                                                     "fibreSupportHolder",
                                                     rIn+rPlate, rOut,
                                                     (length - MppcLength - zOffset)/2.,
                                                     0, 2*M_PI);
      cout << "solidFibreSupportHolder:  (length - MppcLength - zOffset)/2. = "
           << (length - MppcLength - zOffset)/2. << endl;

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
                               0, true);
      cout << "fibreSupportPlate placement at z = " << -lengtPlate/2. + length/2. << endl;

      if (1) new G4PVPlacement(nullptr,
                               {0, 0, + MppcLength/2. + zOffset/2. - 10*mm},
                               volumeFibreSupportHolder,
                               "fibreSupportHolder",
                               volume,
                               false,
                               0, true);
      cout << "fibreSupportHolder placement at z = " << + MppcLength/2. + zOffset/2. - 10*mm << endl;

      
      // -- mirror volume. This is upstream (US)!
      if (1) new G4PVPlacement(nullptr,
                               {0, 0, -lengtPlate/2. + length/2.},
                               volumeFibreSupportPlate,
                               "fibreSupportPlate",
                               fVolumeM,
                               false,
                               0, true);
      if (1) new G4PVPlacement(nullptr,
                               {0, 0, + MppcLength/2. + zOffset/2.},
                               volumeFibreSupportHolder,
                               "fibreSupportHolder",
                               fVolumeM,
                               false,
                               0, true);

      
      // place SiO2 in Mppc
      new G4PVPlacement(nullptr,
                        {0,0, - MppcThickness/2. + 50 * nm},
                        volumeFibreMppcSiO2,
                        "fibreMppcSiO2",
                        volumeFibreMppc,
                        false,
                        0, true);
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
      // Mu3eConstruction::visVolume(fVolumeFibreSmbPcb, {0.8,0.2,0.4},
      //                             Mu3eConstruction::PHYSICAL|
      //                             Mu3eConstruction::FIBRE);
      // Mu3eConstruction::visVolume(fVolumeFibreSmbPcb[1], {0.2,0.2,0.8},
      //                             Mu3eConstruction::PHYSICAL|
      //                             Mu3eConstruction::FIBRE);
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
          //          position.setZ(length/2. + MppcThicknessEpoxy/2. - MppcLength/2. - 6.5*mm);
          position.setZ(length/2. + MppcThicknessEpoxy/2. - MppcLength/2. - 14.15*mm);
        } else {
          //          position.setZ(length/2. + MppcThicknessEpoxy/2. - MppcLength/2. - 19.0*mm);
          position.setZ(length/2. + MppcThicknessEpoxy/2. - MppcLength/2. - 26.6*mm);
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
        // positionPcb.setZ(position.z() - length/2.  - 1.3*CLHEP::cm);
        positionPcb.setZ(position.z() - length/2.  - 0.47*CLHEP::cm);
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
          double phiOrig = phi;
          if (phi < 0) phi += 360.;
          if (mirrored) {
            // -- upstream, i.e. z < 0
            nSmb = 8 - static_cast<int>(phi)/30;
            if (nSmb < 0)  nSmb += 12;
            if (nSmb > 11) nSmb -= 12;
          } else {
            nSmb = 9 + static_cast<int>(phi)/30;
            if (nSmb > 11) nSmb -= 12;
          }
          sprintf(ssmb, "%s_%d", simpr.c_str(), nSmb);
          if (DBX) cout << "*ipv->GetName() = " << sname << " trsl = "
                        << (*ipv)->GetTranslation()
                        << " phiOrig = " << phiOrig
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
        if (string::npos != sname.find("fibreSMBAsic")) {
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
          sprintf(sasic, "%s_%d_CHIP_2", simpr.c_str(), nSmb);
          if (DBX) cout << "*ipv->GetName() = " << sname << " trsl = "
                        << (*ipv)->GetTranslation()
                        << " -> sasic = " << sasic
                        << endl;
          (*ipv)->SetName(sasic); 
          ++ipv;
          sname = (*ipv)->GetName();

          sprintf(sasic, "%s_%d_CHIP_3", simpr.c_str(), nSmb);
          if (DBX) cout << "*ipv->GetName() = " << sname << " trsl = "
                        << (*ipv)->GetTranslation()
                        << " -> sasic = " << sasic
                        << endl;
          (*ipv)->SetName(sasic); 
          ++ipv;
          sname = (*ipv)->GetName();

          sprintf(sasic, "%s_%d_CHIP_1", simpr.c_str(), nSmb);
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
        if (!mirrored) {
          if (11 == nSmb) {
            break;
          }
        } else {
          if (6 == nSmb) {
            break;
          }
        }
        ++ipv;
      }

      
    }


    // ----------------------------------------------------------------------
    G4AssemblyVolume* FibreSupport::makeSmb(int index) {
      using CLHEP::mm;
      using CLHEP::radian;
      
      auto& materials = Mu3eMaterials::Instance();

      G4RotationMatrix rotm = G4RotationMatrix();

      // -- the large base
      G4VSolid* solidFibreSMBPcb1   = new G4Box("fibreSMBPcb1", fSMBPcbWidth1/2., fSMBPcbLength1/2., fSMBPcbThickness/2.);
      // -- the smaller tail with the connector (interposer?)
      G4VSolid* solidFibreSMBPcb2   = new G4Box("fibreSMBPcb2", fSMBPcbWidth2/2., fSMBPcbLength2/2., fSMBPcbThickness/2.);
  
      // -- the MuTRIG ASIC
      G4VSolid* solidFibreSMBAsic0   = new G4Box("fibreSMBAsic0", fSMBAsicWidth/2., fSMBAsicWidth/2., fSMBAsicThickness/2.);
      G4VSolid* solidFibreSMBAsic1   = new G4Box("fibreSMBAsic1", fSMBAsicWidth/2., fSMBAsicWidth/2., fSMBAsicThickness/2.);
      G4VSolid* solidFibreSMBAsic2   = new G4Box("fibreSMBAsic2", fSMBAsicWidth/2., fSMBAsicWidth/2., fSMBAsicThickness/2.);
      G4VSolid* solidFibreSMBAsic3   = new G4Box("fibreSMBAsic3", fSMBAsicWidth/2., fSMBAsicWidth/2., fSMBAsicThickness/2.);

      // -- rotated chip next to the interposer
      G4VSolid* solidFibreSMBChip1   = new G4Box("fibreSMBChip1", fSMBChip1Width/2., fSMBChip1Width/2., fSMBChip1Thickness/2.);

      // -- chip with the two round structures *right next* to it
      G4VSolid* solidFibreSMBChip2   = new G4Box("fibreSMBChip2", fSMBChip2Width/2., fSMBChip2Width/2., fSMBChip2Thickness/2.);

      // -- chip with the two round structures at two different edges
      G4VSolid* solidFibreSMBChip3   = new G4Box("fibreSMBChip3", fSMBChip3Width/2., fSMBChip3Width/2., fSMBChip3Thickness/2.);

      // -- Connector at narrow end
      G4VSolid* solidFibreSMBConnector = new G4Box("fibreSMBConnector", fSMBConnectorLength/2., fSMBConnectorWidth/2., fSMBConnectorThickness/2.);

      // -- create PCB (non-rectangular) shape
      G4RotationMatrix* yRot = new G4RotationMatrix(); 
      G4ThreeVector yTrans(0., 0.5*(fSMBPcbLength1+fSMBPcbLength2), 0.);
      G4UnionSolid* solidFibreSMBPcb = new G4UnionSolid("solidFibreSMBPcb", solidFibreSMBPcb1, solidFibreSMBPcb2, yRot, yTrans);
  
      G4LogicalVolume *volumeFibreSmbPcb = new G4LogicalVolume(solidFibreSMBPcb, materials.Kapton, "fibreSMBPcb");

      G4VisAttributes *pVA  = new G4VisAttributes;
      if (0 == index) {
        pVA->SetColour(G4Colour(0.8, 0.2, 0.4, 0.5));
      } else {
        pVA->SetColour(G4Colour(0.2, 0.2, 0.8, 0.5));
      }
      pVA->SetForceSolid(true);
      volumeFibreSmbPcb->SetVisAttributes(pVA);

      G4SDManager *sdManager = G4SDManager::GetSDMpointer();
      G4VSensitiveDetector *sdSmb = sdManager->FindSensitiveDetector("mu3e/FibreSmbSD");
      cout << "sdSmb = " << sdSmb << endl;
      volumeFibreSmbPcb->SetSensitiveDetector(sdSmb); 
      
      // -- create complete board as assembly, first PCB
      G4RotationMatrix Ra;
      G4ThreeVector Ta;
      G4Transform3D Tr;
      G4AssemblyVolume* solidFibreSMB = new G4AssemblyVolume();
  
      Ra.rotateZ(0.);
  
      double tx(0.), ty(0.), tz(0.);
      Ta.setX(tx); Ta.setY(ty); Ta.setZ(tz);
      Tr = G4Transform3D(Ra,Ta);
      solidFibreSMB->AddPlacedVolume(volumeFibreSmbPcb, Tr);

      
      // -- add 4 asics (unrolled for-loop because of superstition)
      G4VisAttributes *pVAa  = new G4VisAttributes;
      pVAa->SetColour(G4Colour(0., 0., 0.));
      pVAa->SetForceSolid(true);

      G4VSensitiveDetector *sdSmbMuTrig = sdManager->FindSensitiveDetector("mu3e/FibreSmbMuTrigSD");

      bool DOASIC(true);
      
      G4LogicalVolume* pa = new G4LogicalVolume(solidFibreSMBAsic0, materials.Si, "fibreSMBAsic0");
      pa->SetSensitiveDetector(sdSmbMuTrig);
      pa->SetVisAttributes(pVAa);
      tx = 0.5*(fSMBPcbWidth1 - fSMBAsicWidth - 2.*fSMBAsicDeltaSide) - 0*(fSMBAsicWidth + fSMBAsicDeltaChip);
      ty = -0.5*(fSMBPcbLength1 - fSMBAsicWidth) + fSMBAsicDeltaFront;
      //      ty = 0.0;
      tz = 0.5*(fSMBPcbThickness + fSMBAsicThickness);
      Ta.setX(tx); Ta.setY(ty); Ta.setZ(tz);
      Tr = G4Transform3D(rotm, Ta);
      cout << "Placement fibreSMBAsic0 T = (" << tx << "," << ty << "," << tz << ")" << endl;
      if (DOASIC) solidFibreSMB->AddPlacedVolume(pa, Tr);

      pa = new G4LogicalVolume(solidFibreSMBAsic1, materials.Si, "fibreSMBAsic1");
      pa->SetSensitiveDetector(sdSmbMuTrig);
      pa->SetVisAttributes(pVAa);
      tx = 0.5*(fSMBPcbWidth1 - fSMBAsicWidth - 2.*fSMBAsicDeltaSide) - 1*(fSMBAsicWidth + fSMBAsicDeltaChip);
      Ta.setX(tx); Ta.setY(ty); Ta.setZ(tz);
      Tr = G4Transform3D(rotm, Ta);
      cout << "Placement fibreSMBAsic1 T = (" << tx << "," << ty << "," << tz << ")" << endl;
      if (DOASIC) solidFibreSMB->AddPlacedVolume(pa, Tr);

      pa = new G4LogicalVolume(solidFibreSMBAsic2, materials.Si, "fibreSMBAsic2");
      pa->SetSensitiveDetector(sdSmbMuTrig);
      pa->SetVisAttributes(pVAa);
      tx = 0.5*(fSMBPcbWidth1 - fSMBAsicWidth - 2.*fSMBAsicDeltaSide) - 2*(fSMBAsicWidth + fSMBAsicDeltaChip);
      Ta.setX(tx); Ta.setY(ty); Ta.setZ(tz);
      Tr = G4Transform3D(rotm, Ta);
      cout << "Placement fibreSMBAsic2 T = (" << tx << "," << ty << "," << tz << ")" << endl;
      if (DOASIC) solidFibreSMB->AddPlacedVolume(pa, Tr);

      pa = new G4LogicalVolume(solidFibreSMBAsic3, materials.Si, "fibreSMBAsic3");
      pa->SetSensitiveDetector(sdSmbMuTrig);
      pa->SetVisAttributes(pVAa);
      tx = 0.5*(fSMBPcbWidth1 - fSMBAsicWidth - 2.*fSMBAsicDeltaSide) - 3*(fSMBAsicWidth + fSMBAsicDeltaChip);
      Ta.setX(tx); Ta.setY(ty); Ta.setZ(tz);
      Tr = G4Transform3D(rotm, Ta);
      cout << "Placement fibreSMBAsic3 T = (" << tx << "," << ty << "," << tz << ")" << endl;
      if (DOASIC) solidFibreSMB->AddPlacedVolume(pa, Tr);

      // -- add chip #2
      pa = new G4LogicalVolume(solidFibreSMBChip2, materials.Si, "fibreSMBChip2");
      pa->SetSensitiveDetector(sdSmbMuTrig);
      
      G4VisAttributes *pVA2  = new G4VisAttributes;
      pVA2->SetColour(G4Colour(0., 0., 0.));
      pVA2->SetForceSolid(true);
      pa->SetVisAttributes(pVA2);
  
      tx = 0.5*(-fSMBPcbWidth1 + fSMBChip2Width + 2.*fSMBChip2DeltaSide); 
      ty = -0.5*(fSMBPcbLength1 - fSMBChip2Width) + fSMBChip2DeltaFront;
      tz = 0.5*(fSMBPcbThickness + fSMBChip2Thickness); 

      Ta.setX(tx); Ta.setY(ty); Ta.setZ(tz);
      Tr = G4Transform3D(rotm, Ta);
      cout << "Placement fibreSMBChip2 T = (" << tx << "," << ty << "," << tz << ")" << endl;
      solidFibreSMB->AddPlacedVolume(pa, Tr);
      
      // -- add chip #3
      pa = new G4LogicalVolume(solidFibreSMBChip3, materials.Si, "fibreSMBChip3");
      pa->SetSensitiveDetector(sdSmbMuTrig);
  
      G4VisAttributes *pVA3  = new G4VisAttributes;
      pVA3->SetColour(G4Colour(0., 0., 0.));
      pVA3->SetForceSolid(true);
      pa->SetVisAttributes(pVA3);
      
      tx = 0.5*(-fSMBPcbWidth1 + fSMBChip3Width + 2.*fSMBChip3DeltaSide);
      ty = -0.5*(fSMBPcbLength1 - fSMBChip3Width) + fSMBChip3DeltaFront;
      tz = 0.5*(fSMBPcbThickness + fSMBChip3Thickness);
      
      Ta.setX(tx);
      Ta.setY(ty);
      Ta.setZ(tz);
      Tr = G4Transform3D(rotm, Ta);
      cout << "Placement fibreSMBChip3 T = (" << tx << "," << ty << "," << tz << ")" << endl;
      solidFibreSMB->AddPlacedVolume(pa, Tr);

      // -- add chip #1
      pa = new G4LogicalVolume(solidFibreSMBChip1, materials.Si, "fibreSMBChip1");
      pa->SetSensitiveDetector(sdSmbMuTrig);
      
      G4VisAttributes *pVA1  = new G4VisAttributes;
      pVA1->SetColour(G4Colour(0., 0., 0.));
      pVA1->SetForceSolid(true);
      pa->SetVisAttributes(pVA1);
  
      tx = 0.;
      ty = -0.5*fSMBPcbLength1 + fSMBChip1DeltaCenter;
      tz = 0.5*(fSMBPcbThickness + fSMBChip1Thickness);
  
      Ta.setX(tx);
      Ta.setY(ty);
      Ta.setZ(tz);
      rotm.rotateZ(-M_PI/4*CLHEP::rad);
      Tr = G4Transform3D(rotm, Ta);
      cout << "Placement fibreSMBChip1 T = (" << tx << "," << ty << "," << tz << ")" << endl;
      solidFibreSMB->AddPlacedVolume(pa, Tr);
      // -- rotate back
      rotm.rotateZ(+M_PI/4*CLHEP::rad);

     
      // -- add connector
      pa = new G4LogicalVolume(solidFibreSMBConnector, materials.Si, "fibreSMBConnector");
      
      G4VisAttributes *pVA4  = new G4VisAttributes;
      pVA4->SetColour(G4Colour(0.7, 0.7, 0.7));
      pVA4->SetForceSolid(true);
      pa->SetVisAttributes(pVA4);
      
      Ta.setX(0.);
      Ta.setY(0.5*fSMBPcbLength1 + fSMBPcbLength2 - 0.5*fSMBConnectorWidth - fSMBConnectorDelta);
      Ta.setZ(0.5*(fSMBPcbThickness + fSMBConnectorThickness));
  
      Tr = G4Transform3D(rotm, Ta);
      solidFibreSMB->AddPlacedVolume(pa, Tr);

      return solidFibreSMB;
    }
  }
} // namespace mu3e::sim

