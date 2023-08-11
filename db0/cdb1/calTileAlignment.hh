#ifndef CALTILEALIGNMENT_h
#define CALTILEALIGNMENT_h

#include "calAbs.hh"

#include <string>
#include <map>

// ----------------------------------------------------------------------
// tile alignment class 
// ----------------------------------------------------------------------
class calTileAlignment : public calAbs {
public:
  
  calTileAlignment() = default;
  calTileAlignment(cdbAbs *db);
  calTileAlignment(cdbAbs *db, std::string tag);
  ~calTileAlignment();

  // -- direct accessors
  uint32_t id(uint32_t id) {return fMapConstants[id].id;}
  int      sensor(uint32_t id) {return fMapConstants[id].sensor;}
  double   posx(uint32_t id) {return fMapConstants[id].posx;}
  double   posy(uint32_t id) {return fMapConstants[id].posy;}
  double   posz(uint32_t id) {return fMapConstants[id].posz;}
  double   dirx(uint32_t id) {return fMapConstants[id].dirx;}
  double   diry(uint32_t id) {return fMapConstants[id].diry;}
  double   dirz(uint32_t id) {return fMapConstants[id].dirz;}

  std::string getName() override {return fTileAlignmentTag;}
  void        calculate(std::string hash) override;
  std::string makeBLOB(std::map<unsigned int, std::vector<double> >) override {return "FIXME"; }

  bool        getNextID(uint32_t &ID);
  
private:
  std::string fTileAlignmentTag{"tilealignment_"};

  // -- local and private
  struct constants {
    int sensor;
    uint32_t id; 
    double posx, posy, posz;
    double dirx, diry, dirz;
  };

  std::map<uint32_t, constants> fMapConstants;
  std::map<uint32_t, constants>::iterator fMapConstantsIt{fMapConstants.end()};
  
};

#endif
