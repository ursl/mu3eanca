#ifndef CDBROOT_h
#define CDBROOT_h

#include "cdbAbs.hh"

// ----------------------------------------------------------------------
// implementation class for a ROOT file-based DB
// ----------------------------------------------------------------------

class cdbROOT: public cdbAbs {
public:
  cdbROOT() = default;
  cdbROOT(std::string gt, std::string uri, int verbose);
  ~cdbROOT();

  void                 init();
  payload              getPayload(std::string hash) override;

  
private: 
  
};


#endif
