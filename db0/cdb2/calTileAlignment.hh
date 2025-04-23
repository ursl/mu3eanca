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

  std::string makeBLOB() override;
  std::string makeBLOB(const std::map<unsigned int, std::vector<double>>&) override;
  std::map<unsigned int, std::vector<double> > decodeBLOB(std::string) override;
  // -- verbosity = -1 (all), 0 (no elements), n (n elements)
  void printBLOB(std::string, int verbosity = -1) override;
  std::string readCsv(std::string filename);

  void        resetIterator() {fMapConstantsIt = fMapConstants.begin();}
  bool        getNextID(uint32_t &ID);

  std::string getSchema() override {return fSchema;}

private:
  std::string fTileAlignmentTag{"tilealignment_"};

  // -- local and private. Note order of id and sensor (different from alignment/tiles tree)!
  struct constants {
    uint32_t id;
    int sensor;
    double posx, posy, posz;
    double dirx, diry, dirz;
  };
  std::string fSchema{"ui_id,i_sensor,posx,posy,posz,dirx,diry,dirz"};

  std::map<uint32_t, constants> fMapConstants;
  std::map<uint32_t, constants>::iterator fMapConstantsIt{fMapConstants.end()};

};

#endif
