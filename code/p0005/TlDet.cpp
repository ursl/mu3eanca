//

#include "TlDet.h"

#include <mu3e/tools/idmap.h>

#include <mu3e/util/double.hpp>

#include "Mu3eConditions.hh"
#include "calTileAlignment.hh"


TlDet::Tile::Tile(
    uint32_t id_,
    const float3& pos_, const float3& dir_
)
    : id(id_)
    , pos(pos_), dir(dir_)
    , plane(watson::Plane::Curvilinear(toVector3(dir), toVector3(pos)))
{
    posphi = pos.phi();
}

TlDet::Hit::Hit(const Tile& tile, uint32_t timestamp_, float edep, int32_t tid)
    : tile(tile), timestamp(timestamp_, Conf::inst.tl.n_fine_bits), edep(edep)
    , tid(tid)
{
}

#include <TTree.h>

void TlDet::readTiles(Tile::map_t& tiles, TTree* tree) {
  Mu3eConditions *pDC = Mu3eConditions::instance();
  if (pDC->getDB()) {
    std::cout << "Mu3eConditions with gt =  " << pDC->getGlobalTag()
              << " and db = " << pDC->getDB()->getName()
              << std::endl;
    pDC->printCalibrations();          
    calAbs *cal = pDC->getCalibration("tilealignment_");
    std::cout << "tilealignment_ cal = " << cal << std::endl;
    
    calTileAlignment *cpa = dynamic_cast<calTileAlignment*>(cal);
    uint32_t i(99999);
    std::cout << "HALLO TlDet::readTiles(...) cpa->getName() = " << cpa->getName() << std::endl;
    int cnt(0); 
    unsigned int id;
    double3 pos = {}, dir = {}; 
    while (cpa->getNextID(i)) {
      std::cout << "TlDet ID = " << i << " (cnt = " << cnt++ << ")" << std::endl;
      id = cpa->id(i); 
      pos.x = cpa->posx(i); 
      pos.y = cpa->posy(i); 
      pos.z = cpa->posz(i); 
      dir.x = cpa->dirx(i); 
      dir.y = cpa->diry(i); 
      dir.z = cpa->dirz(i); 
      tiles.emplace(std::piecewise_construct,
                    std::forward_as_tuple(id),
                    std::forward_as_tuple(id, make_float3(pos), make_float3(dir))
                    );
    }
  } else {
    if(!tree) {
      mu3e::log::warn("[TlDet::readTiles] No tiles alignment tree found (tree == NULL)\n");
      return;
    }
    
    uint32_t id = 0;
    double3 pos = {}, dir = {};
    
    if(tree->GetBranch("id")) {
      tree->SetBranchAddress("id", &id);
    }
    else {
      // TODO: remove (deprecated)
      tree->SetBranchAddress("sensor", &id);
    }
    tree->SetBranchAddress("posx", &pos.x);
    tree->SetBranchAddress("posy", &pos.y);
    tree->SetBranchAddress("posz", &pos.z);
    tree->SetBranchAddress("dirx", &dir.x);
    tree->SetBranchAddress("diry", &dir.y);
    tree->SetBranchAddress("dirz", &dir.z);
    
    for(Long64_t entry = 0, n = tree->GetEntries(); entry < n; entry++) {
      tree->GetEntry(entry);
      
      tiles.emplace(std::piecewise_construct,
                    std::forward_as_tuple(id),
                    std::forward_as_tuple(id, make_float3(pos), make_float3(dir))
                    );
    }
  }
}
