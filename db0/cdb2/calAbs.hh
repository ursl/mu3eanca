#ifndef CALABS_h
#define CALABS_h

#include "cdbAbs.hh"
#include "payload.hh"

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <map>

// ----------------------------------------------------------------------
// abstract base class for calibration classes
// ----------------------------------------------------------------------

class calAbs {
public:
  calAbs() = default;
  calAbs(cdbAbs *db);
  calAbs(cdbAbs *db, std::string tag);
  virtual ~calAbs();

  void                setIOVs(std::vector<int> v) {fIOVs = v;}
  void                setVerbosity(int v) {fVerbose = v;}
  virtual std::string getName() { return "blurp"; }
  virtual std::string getHash() {return fHash;}
  virtual void        calculate(std::string hash) {
    std::cout << "calAbs::calculate() ?" << std::endl;
  }

  // -- BLOB creation from fMapConstants. This is base-class dependent and hence needs to be overridden.
  virtual std::string makeBLOB() {
    return "calAbs::makeBLOB() ?";
  }
  // -- BLOB creation from a vector<double>. This is base-class dependent and hence needs to be overridden.
  virtual std::string makeBLOB(const std::map<unsigned int, std::vector<double>>&) {
    return "calAbs::makeBLOB(map<int, vector<double>>) ?";
  }

  // -- BLOB decoding. This is base-class dependent and hence needs to be overridden.
  virtual std::map<unsigned int, std::vector<double> > decodeBLOB(std::string) {
    return {};
  }

  // -- print the payload. This is base-class dependent and hence needs to be overridden.
  virtual void printBLOB(std::string, int verbosity = 1) {
    std::cout << "calAbs::printBLOB() ?" << std::endl;
  }

  virtual std::string  getSchema() {
    return "calAbs::getSchema()?";
  }

  // -- direct interactions
  void                readPayloadFromFile(std::string hash, std::string dir);
  void                writePayloadToFile(std::string hash, std::string dir);
  void                writePayloadToFile(std::string hash, std::string dir, payload &pl);
  payload             getPayload(std::string hash) {return fTagIOVPayloadMap[hash];}
  void                insertPayload(std::string hash, payload x);

  void                update(std::string hash);
  void                setPrintTiming(int v) {fPrintTiming = v;}
  std::string         getError() {return fError;}

  // -- payload string parsing/extraction
  std::string getValue(std::string key);

protected:
  cdbAbs                            *fDB;
  std::string                        fTag{"unset"}, fHash{"unset"}, fError{"unset"};
  int                                fVerbose{0}, fPrintTiming{0};
  std::vector<int>                   fIOVs;  // set it up initially
  std::map<std::string, payload>     fTagIOVPayloadMap;  // cache
  std::string                        fPayloadString{""};
};

#endif
