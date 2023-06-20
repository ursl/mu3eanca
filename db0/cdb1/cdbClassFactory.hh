#ifndef CDBCLASSFACTORY_h
#define CDBCLASSFACTORY_h

#include <string>

class cdbAbs;
class calAbs;
class cdb;

// ----------------------------------------------------------------------
// factory for calibration classes
// ----------------------------------------------------------------------

class cdbClassFactory {
public:
  static cdbClassFactory* instance(cdbAbs *);
  calAbs* createClass(std::string name);
  calAbs* createClass(std::string name, std::string tag);
  calAbs* createClassWithDB(std::string name, std::string tag, cdbAbs *db);
  void setVerbosity(int v) {fVerbose = v;}

protected:
  cdbClassFactory(cdbAbs *);
  ~cdbClassFactory();

private:
  static cdbClassFactory* fInstance; 
  cdbAbs *fDB;
  int fVerbose{0};
};

#endif

