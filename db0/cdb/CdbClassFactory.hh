#ifndef CDBCLASSFACTORY_h
#define CDBCLASSFACTORY_h

#include <string>

class calAbs;
class cdb;

// ----------------------------------------------------------------------
// factory for calibration classes
// ----------------------------------------------------------------------

class cdbClassFactory {
public:
  static cdbClassFactory* instance();
  calAbs* createClass(std::string name, cdb *db, std::string tag);

protected:
  cdbClassFactory();
  ~cdbClassFactory();

private:
  static cdbClassFactory* fInstance; 
};

#endif

