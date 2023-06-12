#ifndef PAYLOAD_h
#define PAYLOAD_h

#include <vector>
#include <string>

class payload {
public:
  payload();
  int fLength;
  std::string fComment;
  std::string fHash;
  std::string fBLOB;
};

#endif

