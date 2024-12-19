#ifndef ANAENSEMBLE_H
#define ANAENSEMBLE_H

#include "anaLadder.hh"

#include <string>
#include <map>
#include <vector>

#include "TLatex.h"
#include "TLegend.h"
#include "TLegendEntry.h"
#include "TLine.h"
#include "TArrow.h"
#include "TBox.h"
 
#include "TString.h"
#include "TObject.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TProfile.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TROOT.h"

#include <TH2D.h>

// ----------------------------------------------------------------------
class anaEnsemble  {
public:
  anaEnsemble(std::string dirname, std::string pdfprefix = "ldrmode");
  anaEnsemble(std::string dirname, std::vector<std::string>& ladderlist, std::string pdfprefix = "ldrlist");
  ~anaEnsemble();

  void analysis(int mode);
  void plotLinkQuality(int mode);
  void plotLVCurrents(int mode);
  
  void makeCanvas(int i);
  void labelAxes(TH2D*);
  int  getXbin(std::string label, TH2D *h);
  int  getYbin(std::string label, TH2D *h);

  void setPDFDir(std::string dir = ".") {fPDFDir = dir;}
  
private:
  std::string fDirectory;

  int fnLadders{0}; 
  std::vector<std::pair<std::string, anaLadder*>> fEnsemble;

  // -- histograms
  std::map<std::string, TH1 *> fHists;

  std::string fPDFPrefix{"bla"}, fPDFDir{"."};
  
  TCanvas *c0, *c1, *c2, *c3, *c4, *c5;
  TLatex *tl;
  TBox *box;
  TArrow *pa;
  TLine *pl;
  TLegend *legg;
  TLegendEntry *legge;
  
};

#endif
