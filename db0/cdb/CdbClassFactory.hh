#ifndef CDBCLASSFACTORY_h
#define CDBCLASSFACTORY_h

#include <string>

class calAbs;
class cdb;

// ----------------------------------------------------------------------
// factory for calibration classes
// ----------------------------------------------------------------------

class CdbClassFactory {
public:
  static CdbClassFactory* instance(cdb *);
  calAbs* createClass(std::string name, std::string tag);
  calAbs* createClassWithDB(std::string name, cdb *db, std::string tag);

protected:
  CdbClassFactory(cdb *);
  ~CdbClassFactory();

private:
  static CdbClassFactory* fInstance; 
  cdb *fDB;
};

#endif

