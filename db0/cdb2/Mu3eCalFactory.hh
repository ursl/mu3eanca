#ifndef MU3ECALFACTORY_h
#define MU3ECALFACTORY_h

#include <vector>
#include <string>

class cdbAbs;
class calAbs;

// ----------------------------------------------------------------------
// create concrete cal* objects and return calAbs*
// ----------------------------------------------------------------------

class Mu3eCalFactory {
public:
  static Mu3eCalFactory* instance(std::string gt = "", cdbAbs *db = 0);

  calAbs* createClass(std::string name);
  calAbs* createClass(std::string name, std::string tag);
  calAbs* createClassWithDB(std::string name, std::string tag, cdbAbs *db);
  calAbs* createClassFromFile(std::string hash, std::string dir);

  void    setVerbosity(int v)   {fVerbose = v;}

private:
  Mu3eCalFactory(std::string gt = "", cdbAbs *db = 0);
  ~Mu3eCalFactory();

  static Mu3eCalFactory* fInstance;

  std::string fGT{"GT unset"};
  cdbAbs *fDB{0};
  int fVerbose{10}, fPrintTiming{0};

	// -- for the set global tag
  std::vector<std::string> fTags;


};

#endif
