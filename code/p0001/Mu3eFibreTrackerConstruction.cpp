/*
 * Mu3eFibreTrackerConstruction.cpp
 *
 * The fibre detector is created by first placing the necesary fibres in
 * a ribbon and afterwords aranging the ribbons in a cylinder to form the
 * tracker. There are two possible ribbon geomtries, which are determined
 * by the value of fGeometry as follows:
 *
 *          o o o o              o o o o
 *     0:    o o o          1:    o o o
 *          o o o o                o o
 *
 *  Created on: May 4, 2012
 *      Author: nberger
 *  Updated on: June, 2014
 *      corrodis
 *
 *  Editted on: Oct 17, 2014
 *          By: A. Damyanova

 */

#include "Mu3eFibreTrackerConstruction.h"

#include "G4Tubs.hh"
#include "G4Box.hh"
#include <G4Polycone.hh>

#include "Mu3eFibreDetectorSD.h"
#include "Mu3eFibreMppcSD.h"

using namespace mu3e::sim;

FibreTrackerConstruction::FibreTrackerConstruction(mu3e::sim::Ribbon* ribbon_, Mu3eDetectorCfg& cfg_, Mu3eDigiCfg& digicfg_, G4LogicalVolume* mother_)
    : Mu3eConstruction(cfg_, digicfg_, mother_)
    , ribbon(ribbon_)
{
    detector = new mu3e::sim::FibreDetector(ribbon);
    support = new mu3e::sim::FibreSupport(detector, cfg, digicfg);

    //AssignLocalVarValues(cfg);
}

FibreTrackerConstruction::~FibreTrackerConstruction() {
    delete detector;
    delete support;
}

G4VPhysicalVolume* FibreTrackerConstruction::Construct() {
    using CLHEP::mm, CLHEP::radian;

    auto lDet      = detector->length;
    auto lSup      = support->length;
    auto lSupPlate = support->lengtPlateOffset;
    auto rInDet    = detector->rIn - 0.1 * mm;
    auto rInSup    = support->rIn;
    auto rOutDet   = detector->rOut + 0.1 * mm;
    auto rOutSup   = support->rOut + 0.1 * mm;

    const int n = 6;
    double zPlane[n] = { -lDet/2. -lSup, -lDet/2.+lSupPlate, -lDet/2.+lSupPlate,  lDet/2.-lSupPlate, lDet/2.-lSupPlate, lDet/2. + lSup };
    double rInner[n] = { rInSup,         rInSup,             rInDet,             rInDet,             rInSup,            rInSup};
    double rOuter[n] = { rOutSup,        rOutSup,            rOutDet,            rOutDet,            rOutSup,           rOutSup};

    auto solidFibreTracker = new G4Polycone("fibreTracker", 0 * radian, 2.0 * M_PI * radian, n, zPlane, rInner, rOuter);

    logicalFibreTracker = new G4LogicalVolume(solidFibreTracker,
                                      Mu3eMaterials::Instance().He,
                                      "fibreTracker");

    auto fibreDetectorSD = new Mu3eFibreDetectorSD("mu3e/FibreDetectorSD");
    G4SDManager::GetSDMpointer()->AddNewDetector(fibreDetectorSD);
    detector->volume->SetSensitiveDetector(fibreDetectorSD);

    new G4PVPlacement(nullptr, {},
                      detector->volume,
                      "fibreDetector",
                      logicalFibreTracker,
                      false,
                      0);

    // -- place DS support
    new G4PVPlacement(
        G4Transform3D(
            CLHEP::HepRotation({0, 0, 1}, 0),
            { 0, 0, +(detector->length/2 + support->length/2) }
        ),
        support->volume,
        "fibreSupport",
        logicalFibreTracker,
        false,
        0
    );

    // -- place US support (mirror volume because of SMB numerology, rotated by pi around x-axis)
    new G4PVPlacement(
        G4Transform3D(
            CLHEP::HepRotation({1, 0, 0}, M_PI),
            { 0, 0, -(detector->length/2 + support->length/2) }
        ),
        support->fVolumeM,
        "fibreSupport",
        logicalFibreTracker,
        false,
        1
    );

    auto fibreTracker = new G4PVPlacement(nullptr, {},
                      logicalFibreTracker,
                      "fibreTracker",
                      mother,
                      false,
                      0);


    visVolume(logicalFibreTracker, {0.2,1.0,0.2}, FIBRE);
    return fibreTracker;
}
