#include "hitDataPixel.hh"
#include "hitDataIncludes.hh"

// ----------------------------------------------------------------------
// Run with: see derived classes!
//
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
hitDataPixel::hitDataPixel(TChain *chain, string treeName): hitDataBase(chain, treeName) {
  cout << "==> hitDataPixel: constructor..." << endl;
  if (chain == 0) {
    cout << "You need to pass a chain!" << endl;
  }
  fpChain = chain;
  fChainName = treeName;

  fNentries = fpChain->GetEntries();
  cout << "==> hitDataPixel: constructor fpChain: " << fpChain << "/" << fpChain->GetName()
       << " entries = " <<   fNentries
       << endl;
  
  setupTree();
  
}


// ----------------------------------------------------------------------
hitDataPixel::~hitDataPixel() {
  cout << "==> hitDataPixel: destructor ..." << endl;
  if (!fpChain) return;
  delete fpChain->GetCurrentFile();
}


// ----------------------------------------------------------------------
void hitDataPixel::eventProcessing() {
  cout << "hitDataPixel::eventProcessing() " << endl;
}

