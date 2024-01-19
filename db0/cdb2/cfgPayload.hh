#ifndef CFGPAYLOAD_h
#define CFGPAYLOAD_h

#include <vector>
#include <string>

class cfgPayload {
public:
  cfgPayload();
  void print(bool prtAll = true);
  std::string printString(bool prtAll = true);
  std::string json();

  std::string fHash;
  std::string fDate;
  std::string fCfgString;
};

#endif

