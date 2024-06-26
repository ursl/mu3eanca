#ifndef CFGPAYLOAD_h
#define CFGPAYLOAD_h

#include <vector>
#include <string>

class cfgPayload {
public:
  cfgPayload();
  cfgPayload(std::string cfgPayloadFile);
  void readFromFile(std::string hash, std::string dir);
  void print(bool prtAll = true);
  std::string getString(bool prtAll = true);
  std::string getJson();
  
  std::string fHash;
  std::string fDate;
  std::string fCfgString;
  std::string fError;
};

#endif
