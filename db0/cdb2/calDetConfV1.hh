#ifndef CALDETCONFV1_h
#define CALDETCONFV1_h

#include "calAbs.hh"

#include <string>
#include <map>

// ----------------------------------------------------------------------
// DetConf class
// ----------------------------------------------------------------------
class calDetConfV1 : public calAbs {
public:

  calDetConfV1() = default;
  calDetConfV1(cdbAbs *db);
  calDetConfV1(cdbAbs *db, std::string tag);
  ~calDetConfV1();

  // -- direct accessors
  int targetShape() {return 0;}
  double targetThickness1() {return 0;}
  double targetThickness2() {return 0;}
  double targetLength() {return 0;}
  double targetRadius() {return 0;}
  double targetOffsetX() {return 0;}
  double targetOffsetY() {return 0;}
  double targetOffsetZ() {return 0;}

  double magnetFieldStrength() {return 0;}

  std::string getName() override {return fDetConfTag;}
  void        calculate(std::string hash) override;

  std::string makeBLOB() override;
  std::string makeBLOB(std::map<unsigned int, std::vector<double> >) override;
  std::map<unsigned int, std::vector<double> > decodeBLOB(std::string) override;
  // -- verbosity = -1 (all), 0 (no elements), n (n elements)
  void printBLOB(std::string, int verbosity = -1) override;
  std::string readJSON(std::string filename);

private:
  std::string fDetConfTag{"detconfv1_"};

  // -- local and private
  struct constants {
    struct Target {
      int shape;
      double thickness1;
      double thickness2;
      double length;
      double radius;
      double offsetX;
      double offsetY;
      double offsetZ;
    } target;
    struct Magnet {
      double fieldStrength;
    } magnet;
  };

  constants fConstants;
};

#endif
