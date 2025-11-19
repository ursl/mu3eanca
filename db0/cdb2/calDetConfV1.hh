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
  int targetShape() {return fConstants.target.shape;}
  double targetThickness1() {return fConstants.target.thickness1;}
  double targetThickness2() {return fConstants.target.thickness2;}
  double targetLength() {return fConstants.target.length;}
  double targetRadius() {return fConstants.target.radius;}
  double targetOffsetX() {return fConstants.target.offsetX;}
  double targetOffsetY() {return fConstants.target.offsetY;}
  double targetOffsetZ() {return fConstants.target.offsetZ;}

  double magnetFieldStrength() {return fConstants.magnet.fieldStrength;}

  std::string getName() override {return fDetConfTag;}
  void        calculate(std::string hash) override;

  std::string makeBLOB() override;
  std::string makeBLOB(const std::map<unsigned int, std::vector<double>>&) override;
  // -- verbosity = -1 (all), 0 (no elements), n (n elements)
  void printBLOB(std::string, int verbosity = -1) override;
  std::string readJSON(std::string filename);
  std::string getSchema() override {return fSchema;}

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

  std::string fSchema{"target.(shape,thickness1,thickness2,length,radius,offsetX,offsetY,offsetZ),magnet.field.strength"};

  constants fConstants;
};

#endif
