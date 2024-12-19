#ifndef ANAENSEMBLE_H
#define ANAENSEMBLE_H

#include "anaLadder.hh"

#include <string>
#include <map>
#include <vector>

// ----------------------------------------------------------------------
class anaEnsemble  {
public:
  anaEnsemble(std::string dirname);
  anaEnsemble(std::string dirname, std::vector<std::string>& ladderlist);
  ~anaEnsemble();


private:
  std::string fDirectory;
  
  std::map<std::string, anaLadder*> fEnsemble;
  
  
};

#endif
