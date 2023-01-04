//

#include "Mu3eExperimentConstruction.h"

#include "G4Tubs.hh"

#include "Mu3eBeamlineConstruction.h"
#include "Mu3eInnerTrackerConstruction.h"
#include "Mu3eOuterTrackerConstruction.h"
#include "Mu3eRecurlStationConstruction.h"
#include "Mu3eTargetConstruction.h"

#include "Mu3eFibreSD.h"
#include "Mu3eFibreSmbSD.h"
#include "Mu3eFibreSmbMuTrigSD.h"
#include "Mu3eKaptonFlapSD.h"
#include "Mu3eTileSD.h"
#include "Mu3eTileSipmSD.h"
#include "Mu3eTrackerSD.h"

#include <boost/lexical_cast.hpp>

Mu3eExperimentConstruction::Mu3eExperimentConstruction(
    Mu3eDetectorCfg& _cfg, Mu3eDigiCfg& _digicfg, G4LogicalVolume* _mother
)
    : Mu3eConstruction(_cfg, _digicfg, _mother)
{
}

Mu3eExperimentConstruction::~Mu3eExperimentConstruction() {
    if(Mu3eDigiCfg::Instance().verbose >= 1) {
        mu3e::log::debug("[ExperimentConstruction] destructor\n");
    }

    delete beamlineConstruction;
    delete targetConstruction;
    delete innerTrackerConstruction;
    delete outerTrackerConstruction;
    delete fibreTrackerConstruction;
    delete recurlStationConstruction;

    delete tile;
    delete fibre;
    delete ribbon;
    delete smallSensor;
    delete largeSensor;
}

void Mu3eExperimentConstruction::placeSensors(const std::string& configFile, G4LogicalVolume* motherLogical) {
    mu3e::util::config_t config;
    config.read_file(configFile);

    struct sensor_t {
        G4ThreeVector v;
        G4ThreeVector drow;
        G4ThreeVector dcol;
        G4Transform3D transform;
    };
    std::map<uint32_t, sensor_t> sensors;

    for(auto& id2sensor : config.ptree) {
        auto id = boost::lexical_cast<uint32_t>(id2sensor.first);

        auto& sensor = sensors[id];

        sensor.v = {
            id2sensor.second.get("v.x", 0.0),
            id2sensor.second.get("v.y", 0.0),
            id2sensor.second.get("v.z", 0.0)
        };

        sensor.drow = {
            id2sensor.second.get("drow.x", 0.0),
            id2sensor.second.get("drow.y", 0.0),
            id2sensor.second.get("drow.z", 0.0)
        };
        sensor.dcol = {
            id2sensor.second.get("dcol.x", 0.0),
            id2sensor.second.get("dcol.y", 0.0),
            id2sensor.second.get("dcol.z", 0.0)
        };

        G4RotationMatrix rotation(sensor.drow, sensor.dcol, sensor.drow.cross(sensor.dcol));
        // G4Transform3D operates from right to left:
        // - sensor has origin at its center
        // - first change origin to its corner
        sensor.transform = G4Translate3D(-smallSensor->getLocalCorner());
        // - then rotate sensor around corner
        sensor.transform = G4Rotate3D(rotation) * sensor.transform;
        // - and finally move corner to 'v'
        sensor.transform = G4Translate3D(sensor.v) * sensor.transform;
    }

    // place sensors
    for(auto& id2sensor : sensors) {
        uint32_t id = id2sensor.first;
        auto& sensor = id2sensor.second;

        new G4PVPlacement(sensor.transform, smallSensor->logicalVolume, "", motherLogical, false, id);
    }
}

G4VPhysicalVolume* Mu3eExperimentConstruction::Construct() {
    using CLHEP::mm;
    using CLHEP::rad;

    phase = cfg.getDetectorPhase();

    Mu3eMaterials& materials = Mu3eMaterials::Instance();

    G4double fMagnetInnerRadius = cfg.getMagnetInnerRadius() * mm;
    G4double fMagnetOuterRadius = cfg.getMagnetOuterRadius() * mm;
    G4double fMagnetLength = cfg.getMagnetLength() * mm;

    G4Tubs* ExperimentCylinder = new G4Tubs("experiment_cyl", 0, fMagnetOuterRadius, fMagnetLength / 2.0, 0.0, 2 * M_PI * rad);

    auto logicalExperiment = new G4LogicalVolume(ExperimentCylinder, materials.He, "Experiment");
    visVolume(logicalExperiment, { 1, 1, 0.5 }, 0);

    G4ThreeVector positionExperiment = G4ThreeVector(0, 0, 0);
    auto physicalExperiment = new mu3e::g4::PVPlacement(logicalExperiment, mother, positionExperiment);

    G4Tubs* MagnetCylinder = new G4Tubs("magnet_cyl", fMagnetInnerRadius, fMagnetOuterRadius, fMagnetLength / 2.0, 0.0, 2 * M_PI * rad);

    logicalMagnetCylinder = new G4LogicalVolume(MagnetCylinder, materials.Fe, "Magnet");
    new mu3e::g4::PVPlacement(logicalMagnetCylinder, logicalExperiment, positionExperiment);
    visVolume(logicalMagnetCylinder, { 0, 0, 0.6 }, MAGNET);

    beamlineConstruction = new Mu3eBeamlineConstruction(cfg, digicfg, logicalExperiment);
    beamlineConstruction->Construct();

    smallSensor = new mu3e::Sensor("SmallSensor", cfg, digicfg, mu3e::Sensor::TYPE_SMALL);
    auto smallSensorSD = new Mu3eTrackerSD(
        "mu3e/SmallSilicon",
        smallSensor,
        digicfg.getNumLinksInner(),
        digicfg.getReadoutSimulation(),
        digicfg.getClusterSimulation()
    );

    G4SDManager::GetSDMpointer()->AddNewDetector(smallSensorSD);
    smallSensor->logicalVolume->SetSensitiveDetector(smallSensorSD);

    largeSensor = new mu3e::Sensor("LargeSensor", cfg, digicfg, mu3e::Sensor::TYPE_LARGE);
    auto largeSensorSD = new Mu3eTrackerSD(
        "mu3e/LargeSilicon",
        largeSensor,
        digicfg.getNumLinksOuter(),
        digicfg.getReadoutSimulation(),
        digicfg.getClusterSimulation()
    );
    G4SDManager::GetSDMpointer()->AddNewDetector(largeSensorSD);
    largeSensor->logicalVolume->SetSensitiveDetector(largeSensorSD);

    // kapton ageing implementation
    auto kaptonFlapSD = new Mu3eKaptonFlapSD("mu3e/KaptonFlapSD", digicfg.isWriteKaptonFlaps());
    G4SDManager::GetSDMpointer()->AddNewDetector(kaptonFlapSD);

    targetConstruction = new Mu3eTargetConstruction(cfg, digicfg, logicalExperiment);
    targetConstruction->Construct();

    // place sensors at custom positions
    if(auto config = cfg.ptree.get_optional<std::string>("detector.pixels.customGeometry")) {
        mu3e::log::info("[ExperimentConstruction::Construct] use pixels.customGeometry: %s\n", config->c_str());

        placeSensors(*config, logicalExperiment);

        // do final geometry init (transforms)
        initTransforms(physicalExperiment);

        // and skip other detectors
        return physicalExperiment;
    }

    if(cfg.get<int>("pixels.small.nLayers") > 0) {
        innerTrackerConstruction = new Mu3eInnerTrackerConstruction(smallSensor, cfg, digicfg, logicalExperiment);
        innerTrackerConstruction->Construct();
    }

    if(cfg.get<int>("pixels.large.nLayers") > 0) {
        outerTrackerConstruction = new Mu3eOuterTrackerConstruction(largeSensor, cfg, digicfg, logicalExperiment);
        outerTrackerConstruction->Construct();
    }

    if(phase > 0 && cfg.getFibreLayers() > 0) {
        //fibre = new mu3e::sim::Fibre(cfg);
        ribbon = new mu3e::sim::Ribbon(cfg, digicfg);
        auto fibreSD = new Mu3eFibreSD("mu3e/FibreSD", ribbon,
            new mu3e::sim::mppc(cfg, digicfg),
            digicfg.isWriteFibres()
        );
        G4SDManager::GetSDMpointer()->AddNewDetector(fibreSD);
        ribbon->fibre->coreVolume->SetSensitiveDetector(fibreSD);

        // -- SMB SD 
        auto fibreSmbSD = new Mu3eFibreSmbSD("mu3e/FibreSmbSD");
        G4SDManager::GetSDMpointer()->AddNewDetector(fibreSmbSD);

        // -- SMB MuTrig SD 
        auto fibreSmbMuTrigSD = new Mu3eFibreSmbMuTrigSD("mu3e/FibreSmbMuTrigSD");
        G4SDManager::GetSDMpointer()->AddNewDetector(fibreSmbMuTrigSD);

        fibreTrackerConstruction = new mu3e::sim::FibreTrackerConstruction(ribbon, cfg, digicfg, logicalExperiment);
        fibreTrackerConstruction->Construct();
    }

    if(phase > 0 && outerTrackerConstruction != nullptr) {
        tile = new mu3e::sim::Tile(digicfg);
        auto tileSD = new Mu3eTileSD("mu3e/TileSensorSD", tile, digicfg.isWriteTiles());
        G4SDManager::GetSDMpointer()->AddNewDetector(tileSD);
        tile->logicalVolume->SetSensitiveDetector(tileSD);

        auto tileSipmSD = new Mu3eTileSipmSD("mu3e/TileSipmSD", tile, digicfg.isWriteTiles());
        G4SDManager::GetSDMpointer()->AddNewDetector(tileSipmSD);

        recurlStationConstruction = new Mu3eRecurlStationConstruction(
            largeSensor, tile, cfg, digicfg, logicalExperiment, outerTrackerConstruction->getLength()
        );
        recurlStationConstruction->Construct();
    }

    if(Mu3eDigiCfg::Instance().verbose >= 1) {
        G4SDManager::GetSDMpointer()->ListTree();
    }

    //if(auto fibreSD = (Mu3eFibreSD*)SDman->FindSensitiveDetector("mu3e/FibreSD")) {
    //    fibreSD->SetFibrePosInRibbon(fibres_, fibrenums);
    //    fibreSD->SetPhysicalVolumes(fibres, fibrenums, 0);
    //    fibreSD->SetPhysicalVolumes(ribbons, ribbonnums, 1);
    //}
    //else {
    //    G4cout << "Mu3eExperimentConstruction: WARNING : Fibres for noise/alignment not set!" << G4endl;
    //}

    initTransforms(physicalExperiment);

    return physicalExperiment;
}

void Mu3eExperimentConstruction::initTransforms(const G4VPhysicalVolume* physicalVolume) {
    mu3e::g4::Geometry::stack_t stack_;
    stack_.entries.emplace_back(physicalVolume, G4AffineTransform());

    mu3e::g4::Geometry::iterate(stack_, [this] (
        const mu3e::g4::Geometry::stack_t& stack
    ) {
        auto logicalVolume = stack.volume(0)->GetLogicalVolume();
        auto& transform = stack.transform(0);

        int copyNo = stack.volume(0)->GetCopyNo();
        int errors = 0;

        if(smallSensor && logicalVolume == smallSensor->logicalVolume) {
            // check that copyNo is unique
            if(smallSensor->volumes.find(copyNo) != smallSensor->volumes.end()) errors |= 0x01;
            auto& v = smallSensor->volumes[copyNo];
            v = {
                mu3e::id::sensor(static_cast<uint32_t>(copyNo)),
                transform
            };
        }
        if(largeSensor && logicalVolume == largeSensor->logicalVolume) {
            if(largeSensor->volumes.find(copyNo) != largeSensor->volumes.end()) errors |= 0x01;
            auto& v = largeSensor->volumes[copyNo];
            v = {
                mu3e::id::sensor(static_cast<uint32_t>(copyNo)),
                transform
            };
        }
        if(ribbon && logicalVolume == ribbon->fibre->logicalVolume) {
            int id = (stack.volume(1)->GetCopyNo() << 9) + copyNo;
            auto& v = ribbon->fibre->volumes[id];
            if(v.id != 0xFFFFFFFF) errors |= 0x01;
            v = {
                static_cast<uint32_t>(id),
                transform,
                { stack.volume(0)->GetRotation(), stack.volume(0)->GetTranslation() }
            };
        }
        if(ribbon && logicalVolume == ribbon->logicalVolume) {
            auto& v = ribbon->volumes[copyNo];
            if(v.id != 0xFFFFFFFF) errors |= 0x01;
            v = {
                static_cast<uint32_t>(copyNo),
                transform
            };
        }
        if(tile && logicalVolume == tile->logicalVolume) {
            for(auto& volume : tile->volumes | boost::adaptors::map_values) {
                if(volume.mother == stack.volume(1)) {
                    // update tile transform
                    volume.transform *= stack.transform(1);
                }
            }
        }

        if(errors & 0x01) {
            mu3e::log::fatal("[recurseGeometry] volume '%s': duplicate copyNo %d, (0x%08X)\n", logicalVolume->GetName().c_str(), copyNo, copyNo);
            exit(EXIT_FAILURE);
        }
    });
}
