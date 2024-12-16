#ifndef ANAENSEMBLE_H
#define ANAENSEMBLE_H

#include "anaLadder.hh"

#include <string>
#include <map>

// ----------------------------------------------------------------------
class anaEnsemble  {
public:
  anaEnsemble(std::string dirname);
  ~anaEnsemble();


private:
  std::string fDirectory;
  
  std::map<std::string, anaLadder*> fEnsemble;
  
  
};

#endif
