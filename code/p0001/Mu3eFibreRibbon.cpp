//

#include "Mu3eFibreRibbon.h"

#include <mu3e/util/g4.hpp>

#include <CLHEP/Random/RandFlat.h>

#include <G4CSGSolid.hh>
#include <G4Trap.hh>

namespace mu3e::sim {

Ribbon::Ribbon(const Mu3eDetectorCfg& cfg, const Mu3eDigiCfg& digicfg, bool mirrowed_)
    : mirrowed(mirrowed_)
{
    fibre = new mu3e::sim::Fibre(cfg, digicfg);

    type = cfg.getFibreGeometry();
    pitch = cfg.getFibreRibbonPitch() * CLHEP::mm;
    deadwidth = cfg.getFibreDeadWidth() * CLHEP::mm;
    nFibresPerLayer = cfg.getMaxFibersPerLayer();
    nlayers = cfg.getFibreLayers();
    nribbons = cfg.getFibreRibbons();

    rOffset = cfg.getFibreROffset() * CLHEP::mm;
    fStagger = cfg.getFibreRibbonsStagger() * CLHEP::mm;

    length = fibre->length;
    width = nFibresPerLayer * fibre->diameter + (nFibresPerLayer - 1) * deadwidth;
    if(type == TYPE_ZIGZAG) widthtop = width;
    else {
        widthtop = width - ((nlayers - 1) * (fibre->diameter + deadwidth));
        width += (width - widthtop) / (nlayers - 1);
    }

    if(!fibre->square) {
        height = ((nlayers - 1) * sqrt(3.) / 2.) * (fibre->diameter + deadwidth) + fibre->diameter;
    }
    else {
        height = nlayers * fibre->diameter + (nlayers - 1) * deadwidth;
    }

    G4VSolid* solidRibbon;
    if(type == TYPE_ZIGZAG) {
        solidRibbon = new G4Box("Ribbon", width / 2., height / 2., length / 2.);
    }
    else {
        if(mirrowed) {
            solidRibbon = new G4Trap("Ribbon",
                length / 2.,
                0,
                0,
                height / 2.,
                widthtop / 2.,
                width / 2,
                0,
                height / 2.,
                widthtop / 2.,
                width / 2,
                0
            );
        }
        else {
            solidRibbon = new G4Trap("Ribbon",
                length / 2.,
                0,
                0,
                height / 2.,
                width / 2.,
                widthtop / 2,
                0,
                height / 2.,
                width / 2.,
                widthtop / 2,
                0
            );
        }
    }
    logicalVolume = new G4LogicalVolume(solidRibbon, Mu3eMaterials::Instance().getEpoxy(), "Ribbon");

    if(type == TYPE_TRAPEZIUM && mirrowed == false) {
        mirrow = new Ribbon(cfg, digicfg, true);
    }

    /// Fill Fibres into Ribbon
    ArrangeFibresInRibbon();

    Mu3eConstruction::visVolume(logicalVolume,
        { 0.5, 0.5, 0.5 },
        Mu3eConstruction::FIBRE | Mu3eConstruction::ACTIVE | Mu3eConstruction::PHYSICAL);
}

Ribbon::~Ribbon() {
    delete fibre;
}

G4LogicalVolume* Ribbon::GetVolumeMirrow() {
    if(mirrow) return mirrow->logicalVolume;
    return nullptr;
}

int Ribbon::nFibresLayer(int layer) {
    if(mirrowed) return nFibresPerLayer - ((nlayers - 1 - layer) % 2) - type * ((nlayers - 1 - layer) / 2) * 2;
    else
        return nFibresPerLayer - (layer % 2) - type * (layer / 2) * 2;
}

double Ribbon::getPhotonPosX(uint32_t fid) {
    G4ThreeVector pos = fibre->getTransformInRibbon(fid).TransformPoint({});

    return pos.x() + fibre->getPhotonsDistX();
}

bool Ribbon::valid(mu3e::id::fibre fibreId) {
    return fibre->valid(fibreId);
}

bool Ribbon::isNeighbour(int rib, int layer, int fb, int rib2, int layer2, int fb2) {
    if(rib != rib2) return false;

    if(layer2 == layer) {
        if(fb2 == fb - 1 || fb2 == fb + 1) return true;
        else return false;
    }

    if(layer2 < layer - 1 || layer2 > layer + 1) return false;

    if(type && rib % 2 == 1 && nlayers % 2 == 0) {
        // trap    //mirrowd   //even layers
        if(layer % 2 == 0) {
            if(fb2 == fb || fb2 == fb + 1) return true;
            else return false;
        }
        else {
            if(fb2 == fb || fb2 == fb - 1) return true;
            else return false;
        }
    }

    if(layer % 2 == 0) {
        if(fb2 == fb || fb2 == fb - 1) return true;
        else return false;
    }

    else {
        if(fb2 == fb || fb2 == fb + 1) return true;
        else return false;
    }
}

std::vector<mu3e::id::fibre> Ribbon::getNeighbours(mu3e::id::fibre fibreId) {
    std::vector<mu3e::id::fibre> neighbours;

    int rib = fibreId.ribbon;
    int layer = fibreId.layer;
    int fbNo = fibreId.fb;

    for(int layerXtalk = layer - 1; layerXtalk <= layer + 1; ++layerXtalk) {
        if(!(0 <= layerXtalk && layerXtalk < nlayers)) continue;
        for(int fbXtalk = fbNo - 1; fbXtalk <= fbNo + 1; ++fbXtalk) {
            if(!(0 <= fbXtalk && fbXtalk < nFibresPerLayer)) continue;
            mu3e::id::fibre xtalkId(rib, layerXtalk, fbXtalk);
            if(!valid(xtalkId) || !isNeighbour(rib, layer, fbNo, rib, layerXtalk, fbXtalk)) continue;
            neighbours.push_back(xtalkId);
        }
    }

    return neighbours;
}

void Ribbon::ArrangeFibresInRibbon() {
    double x;
    double y = -height / 2. + fibre->diameter / 2.;
    int nFibresThisLayer;
    G4ThreeVector position;
    for(int layer = 0; layer < nlayers; ++layer) {
        //G4cout << "DEBUG: mirrowed: " << mirrowed << "\t layer: " << layer << "\t";
        nFibresThisLayer = nFibresLayer(layer);
        //G4cout << "nFibresThisLayer: " << nFibresThisLayer << G4endl;

        x = -(nFibresThisLayer / 2. - 0.5) * (fibre->diameter + deadwidth);
        int fstart = (nFibresPerLayer - nFibresThisLayer) / 2;
        for(int fno = fstart; fno < (fstart + nFibresThisLayer); ++fno) {
            int copyNo = mu3e::id::fibre(0, layer, fno).value();

            //G4cout << "\t" << fno << ") " << x << "/" << y << G4endl;
            position = { x, y, 0 };
            new G4PVPlacement(nullptr, // no rotation
                position, // at (x,y,z)
                fibre->logicalVolume, // its logical volume
                "Fibre", // its name
                logicalVolume, // its mother  volume
                false, // no boolean operations
                copyNo,
                false
            );
            //G4cout << "DEBUG:" << fno << "\t" << layer << "\t" << x << "/" << y << G4endl;

            x += fibre->diameter + deadwidth;
        }

        if(!fibre->square) {
            y += sqrt(3) / 2. * (fibre->diameter + deadwidth);
        }
        else {
            y += (fibre->diameter + deadwidth);
        }
    }
}

} // namespace mu3e::sim
