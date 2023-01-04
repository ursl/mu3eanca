/// \file Mu3eRibbon.h

#ifndef MU3ERIBBON_H_
#define MU3ERIBBON_H_

#include "Mu3eFibre.h"
#include "Mu3eFibreHit.h"

namespace mu3e::sim {

struct Ribbon {
    static const int TYPE_ZIGZAG = 0;
    static const int TYPE_TRAPEZIUM = 1;

    bool mirrowed;

    int type;

    G4LogicalVolume* logicalVolume;
    Ribbon* mirrow = nullptr;

    Fibre* fibre;

    double width;
    double widthtop;
    double height;
    double length;
    int nlayers;
    int nribbons;
    int nFibresPerLayer;
    int nfibres;

    double deadwidth;
    double pitch;
    double rOffset;
    double fStagger;

    /// ids and corresponding coordinate transforms (local to global)
    struct volume_t {
        uint32_t id = 0xFFFFFFFF;
        G4AffineTransform transform;
    };
    std::map<uint32_t, volume_t> volumes;

    Ribbon(const Mu3eDetectorCfg& cfg, const Mu3eDigiCfg& digicfg, bool mirrowed_ = false);
    virtual ~Ribbon();

    Ribbon(const Ribbon&) = delete;
    Ribbon& operator=(const Ribbon&) = delete;
    G4LogicalVolume* GetVolumeMirrow();

    int nFibresLayer(int layer);

    /// check if fibre exists in this ribbon
    bool valid(mu3e::id::fibre fibreId);
    /// check if is a neghbour fibre
    bool isNeighbour(int rib, int layer, int fb, int rib2, int layer2, int fb2);
    /// get all neighbour fibres
    std::vector<mu3e::id::fibre> getNeighbours(mu3e::id::fibre fibreId);

    /// get a random x position for photons leaving fibre fid
    double getPhotonPosX(uint32_t fid);

private:
    void ArrangeFibresInRibbon();
};

} // namespace mu3e::sim

#endif /* MU3ERIBBON_H_ */
