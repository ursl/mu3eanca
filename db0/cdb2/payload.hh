#ifndef PAYLOAD_h
#define PAYLOAD_h

#include <vector>
#include <string>

class payload {
public:
  payload();
  void print();
  std::string printString(bool prtAll = true);
  std::string json();

  std::string fHash;
  std::string fComment, fSchema;
  std::string fDate;
  std::string fBLOB;
};

#endif

