/// \file Mu3eExperimentConstruction.cpp

/*
 * Mu3eExperimentConstruction.cpp
 *
 *  Created on: Apr 27, 2012
 *      Author: nberger
 */

#include "Mu3eExperimentConstruction.h"

#include "G4Tubs.hh"
#include "G4AssemblyVolume.hh"
#include "G4SDManager.hh"

#include "Mu3eBeamlineConstruction.h"
#include "Mu3eTargetConstruction.h"
#include "Mu3eInnerTrackerConstruction.h"
#include "Mu3eOuterTrackerConstruction.h"
#include "Mu3eFibreTrackerConstruction.h"
#include "Mu3eRecurlStationConstruction.h"


#include "Mu3eTrackerSD.h"
#include "Mu3eFibreSD.h"
#include "Mu3eFibreDetectorSD.h"
#include "Mu3eFibreSmbSD.h"
#include "Mu3eTileSD.h"
#include "Mu3eTileSipmSD.h"
#include "Mu3eKaptonFlapSD.h"

Mu3eExperimentConstruction::Mu3eExperimentConstruction(Mu3eDetectorCfg & _cfg, Mu3eDigiCfg &  _digicfg, G4LogicalVolume * _mother):
Mu3eConstruction(_cfg,_digicfg, _mother),
beamlineConstruction(0),
targetConstruction(0),
innerTrackerConstruction(0),
outerTrackerConstruction(0),
recurlStationConstruction(0),
fibreTrackerConstruction(0)
{
}

Mu3eExperimentConstruction::~Mu3eExperimentConstruction() {
	if(beamlineConstruction)
		delete beamlineConstruction;
	if(targetConstruction)
		delete targetConstruction;
	if(innerTrackerConstruction)
		delete innerTrackerConstruction;
	if(outerTrackerConstruction)
		delete outerTrackerConstruction;
	if(fibreTrackerConstruction)
		delete fibreTrackerConstruction;
	if(recurlStationConstruction)
		delete recurlStationConstruction;
}

G4VPhysicalVolume* Mu3eExperimentConstruction::Construct() {
    using CLHEP::mm;
    using CLHEP::rad;

    phase = cfg.getDetectorPhase();


	Mu3eMaterials & materials = Mu3eMaterials::Instance();


	G4double fMagnetInnerRadius = cfg.getMagnetInnerRadius()*mm;
	G4double fMagnetOuterRadius = cfg.getMagnetOuterRadius()*mm;
	G4double fMagnetLength      = cfg.getMagnetLength()*mm;

	G4Tubs * ExperimentCylinder = new G4Tubs("experiment_cyl", 0, fMagnetOuterRadius, fMagnetLength/2.0, 0.0, 2*M_PI*rad);

	logicalExperiment = new G4LogicalVolume(ExperimentCylinder, materials.He, "Experiment");
    visVolume(logicalExperiment, {1,1,0.5},0);

	G4ThreeVector positionExperiment = G4ThreeVector(0,0,0);
    auto physicalExperiment = new mu3e::g4::PVPlacement(logicalExperiment, mother, positionExperiment);

	G4Tubs * MagnetCylinder = new G4Tubs("magnet_cyl", fMagnetInnerRadius, fMagnetOuterRadius, fMagnetLength/2.0, 0.0, 2*M_PI*rad);

    logicalMagnetCylinder = new G4LogicalVolume(MagnetCylinder, materials.Fe, "Magnet");
    new mu3e::g4::PVPlacement(logicalMagnetCylinder, logicalExperiment, positionExperiment);
    visVolume(logicalMagnetCylinder, {0,0,0.6},MAGNET);

	beamlineConstruction = new Mu3eBeamlineConstruction(cfg,digicfg,logicalExperiment);
	beamlineConstruction->Construct();

    smallSensor = new mu3e::Sensor("SmallSensor", cfg, digicfg, mu3e::Sensor::TYPE_SMALL);
    auto smallSensorSD = new Mu3eTrackerSD("mu3e/SmallSilicon", smallSensor,
                                           digicfg.getNumLinksInner(),
                                           digicfg.getReadoutSimulation(),
                                           digicfg.getClusterSimulation()
    );

    G4SDManager::GetSDMpointer()->AddNewDetector(smallSensorSD);
    smallSensor->volume->SetSensitiveDetector(smallSensorSD);

    largeSensor = new mu3e::Sensor("LargeSensor", cfg, digicfg, mu3e::Sensor::TYPE_LARGE);
    auto largeSensorSD = new Mu3eTrackerSD("mu3e/LargeSilicon", largeSensor,
                                           digicfg.getNumLinksOuter(),
                                           digicfg.getReadoutSimulation(),
                                           digicfg.getClusterSimulation()
    );
    G4SDManager::GetSDMpointer()->AddNewDetector(largeSensorSD);
    largeSensor->volume->SetSensitiveDetector(largeSensorSD);

    //Kapton ageing implementation
    auto kaptonFlapSD = new Mu3eKaptonFlapSD("mu3e/KaptonFlapSD", digicfg.isWriteKaptonFlaps());
    G4SDManager::GetSDMpointer()->AddNewDetector(kaptonFlapSD);

	innerTrackerConstruction = new Mu3eInnerTrackerConstruction(smallSensor, cfg, digicfg, logicalExperiment);
    innerTrackerConstruction->Construct();

    targetConstruction = new Mu3eTargetConstruction(cfg,digicfg,logicalExperiment);
	targetConstruction->Construct();

    outerTrackerConstruction = new Mu3eOuterTrackerConstruction(largeSensor, cfg, digicfg, logicalExperiment);
    outerTrackerConstruction->Construct();
    double centralOuterTrackerLength = outerTrackerConstruction->getLength();



    if(phase > 0 && cfg.getFibreLayers() > 0) {
        //fibre = new mu3e::sim::Fibre(cfg);
        ribbon = new mu3e::sim::Ribbon(cfg, digicfg);
        auto fibreSD = new Mu3eFibreSD("mu3e/FibreSD", ribbon,
            new mu3e::sim::mppc(cfg, digicfg),
            digicfg.isWriteFibres()
        );
        G4SDManager::GetSDMpointer()->AddNewDetector(fibreSD);
        ribbon->fibre->coreVolume->SetSensitiveDetector(fibreSD);
        //auto fibreSD = new Mu3eFibreSD("mu3e/FibreSD", cfg, digicfg);
        //G4SDManager::GetSDMpointer()->AddNewDetector(fibreSD);
        //fibre->coreVolume->SetSensitiveDetector(fibreSD);

        // SMB ASIC SD 
        auto fibreSmbSD = new Mu3eFibreSmbSD("mu3e/FibreSmbSD");
        G4SDManager::GetSDMpointer()->AddNewDetector(fibreSmbSD);
        
        fibreTrackerConstruction = new Mu3eFibreTrackerConstruction(ribbon, cfg, digicfg, logicalExperiment);
        fibreTrackerConstruction->Construct();
    }

    if(phase > 0) {
        tile = new mu3e::sim::Tile(cfg, digicfg);
        auto tileSD = new Mu3eTileSD("mu3e/TileSensorSD", tile,
                                     digicfg.isWriteTiles()
                                     );
        G4SDManager::GetSDMpointer()->AddNewDetector(tileSD);
        tile->volume->SetSensitiveDetector(tileSD);

        auto tileSipmSD = new Mu3eTileSipmSD("mu3e/TileSipmSD",
                                     digicfg.isWriteTiles()
                                     );
        G4SDManager::GetSDMpointer()->AddNewDetector(tileSipmSD);

        recurlStationConstruction = new Mu3eRecurlStationConstruction(largeSensor, tile, cfg,digicfg, logicalExperiment, centralOuterTrackerLength );

        recurlStationConstruction->Construct();
    }

    std::vector<int> copyHistory;
    recurseGeometry(*logicalExperiment, {}, copyHistory);

    G4SDManager::GetSDMpointer()->ListTree();

//    if(auto fibreSD = (Mu3eFibreSD*)SDman->FindSensitiveDetector("mu3e/FibreSD")) {
//        fibreSD->SetFibrePosInRibbon(fibres_, fibrenums);
//        fibreSD->SetPhysicalVolumes(fibres, fibrenums, 0);
//        fibreSD->SetPhysicalVolumes(ribbons, ribbonnums, 1);
//	} else {
//		G4cout << "Mu3eExperimentConstruction: WARNING : Fibres for noise/alignment not set!" << G4endl;
//	}


	return physicalExperiment;
}

/**
 * Build transforms (from local to global coordinate system) by walking geometry
 * tree from top (experiment volume) to bottom (individual detector volumes).
 *
 * Transforms (rotations and translation) specified for physical volumes define
 * how to transform local coordinate to global coordinate.
 *
 * The final transform
 *     T = TN * ... * T2 * T1
 * transforms local coordinate 'l' to global coordinate 'g'
 *     G = L * T = L * r + t
 * where r is the rotation matrix of T and t is translation of T.
 *
 * Math:
 *     T1 = (r1, t1)
 *     T2 = (r2, t2)
 *
 *     G4AffineTransform:
 *       T2 * T1 = (r2 * r1, t2 * r1 + t1)
 *       v * T = v * r + t
 *
 *     G4Transform3D:
 *       T1 * T2 = (r1 * r2, r1 * t2 + t1)
 *       T * v = r * v + t
 *       NOTE: multiply from right leads to inverse rotation!
 */
void Mu3eExperimentConstruction::recurseGeometry(const G4LogicalVolume& motherVolume, const G4AffineTransform& motherTransform, std::vector<int>& copyHistory) {
    int level = copyHistory.size();
    copyHistory.push_back(-1);

    for(int i = 0, n = motherVolume.GetNoDaughters(); i < n; i++) {
        auto volume = motherVolume.GetDaughter(i);

        auto name = volume->GetLogicalVolume()->GetName();
        auto copyNo = volume->GetCopyNo();
        copyHistory[level] = copyNo;

        G4AffineTransform volumeTransform(volume->GetRotation(), volume->GetTranslation());
        auto transform = volumeTransform * motherTransform;
        recurseGeometry(*volume->GetLogicalVolume(), transform, copyHistory);

        // TODO: compare log volumes
        if(smallSensor && name == smallSensor->volume->GetName()) {
            smallSensor->ids.push_back(copyNo);
            smallSensor->transforms.push_back(transform);
        }
        if(largeSensor && name == largeSensor->volume->GetName()) {
            largeSensor->ids.push_back(copyNo);
            largeSensor->transforms.push_back(transform);
        }
        if(ribbon && name == ribbon->fibre->volume->GetName()) {
            ribbon->fibre->ids.push_back((copyHistory[level - 1] << 9) + copyNo);
            ribbon->fibre->transforms.push_back(transform);
            ribbon->fibre->transformsInRibbon.push_back(volumeTransform);
            //fibres.push_back(transform);
            //fibres_.push_back(volumeTransform);
            //fibrenums.push_back((motherCopyNo << 11) + copyNo); // = (ribbonCopyNo << 11) + fibreCopyNo
        }
        if(ribbon && name == ribbon->volume->GetName()) {
            ribbon->ids.push_back(copyNo);
            ribbon->transforms.push_back(transform);
        }
        if(tile && name == tile->volume->GetName()) {
            for(unsigned int j = 0; j < tile->ids.size(); ++j) {
                if(copyHistory[level - 3] / 100000 == (int)tile->ids[j] / 100000)
                    tile->transforms[j] *= motherTransform;
            }
        }
    }

    copyHistory.pop_back();
}
