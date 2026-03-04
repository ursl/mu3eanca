#include <iostream>
#include <cstring>

#include "calAbs.hh"
#include "calPixelQualityLM.hh"
#include "Mu3eCalFactory.hh"

using namespace std;

// ----------------------------------------------------------------------
static void printPayloadCommon(calAbs *c, const string& hash, int verbose) {
  const payload& pl = c->getPayload(hash);
  cout << "hash:    " << pl.fHash << endl;
  cout << "comment: " << pl.fComment << endl;
  cout << "schema:  " << pl.fSchema << endl;
  cout << "date:    " << pl.fDate << endl;
  c->printBLOB(pl.fBLOB, verbose);
}

// ----------------------------------------------------------------------
static void printPixelQualityLMStats(calPixelQualityLM *cpq) {
  cpq->resetIterator();
  uint32_t id;
  int nGoodLinks(0), nMaskedLinks(0), nDeadLinks(0), nStatus4Links(0), nStatus5Links(0);
  int nChips(0), nBadColumns(0), nNoisyPixels(0), nGoodPixels(0);
  while (cpq->getNextID(id)) {
    nChips++;
    for (int icol = 0; icol < 256; icol++) {
      if (cpq->getColStatus(id, icol) != calPixelQualityLM::Good) nBadColumns++;
    }
    double lvdsOverflowRate = cpq->getLVDSOverflowRate(id);
    for (int ilink = 0; ilink < 3; ilink++) {
      if (cpq->getLinkStatus(id, ilink) == calPixelQualityLM::Good) nGoodLinks++;
      if (cpq->getLinkStatus(id, ilink) == calPixelQualityLM::Masked) nMaskedLinks++;
      if (cpq->isLinkDead(id, ilink)) nDeadLinks++;
      if (cpq->getLinkStatus(id, ilink) == calPixelQualityLM::LVDSErrorLink) nStatus4Links++;
      if (cpq->getLinkStatus(id, ilink) == calPixelQualityLM::LVDSErrorOtherLink) nStatus5Links++;
    }
    nNoisyPixels += cpq->getNpixWithStatus(id, calPixelQualityLM::Noisy);
    nGoodPixels += cpq->getNpixWithStatus(id, calPixelQualityLM::Good);
    if (lvdsOverflowRate > 0.) cout << "chipID: " << id << " LVDS overflow rate: " << lvdsOverflowRate << endl;
  }
  cout << "nChips: " << nChips << " nGoodLinks: " << nGoodLinks << " nMaskedLinks: " << nMaskedLinks
       << " nDeadLinks: " << nDeadLinks << " nStatus4Links: " << nStatus4Links << " nStatus5Links: " << nStatus5Links
       << " nBadColumns: " << nBadColumns << " nNoisyPixels: " << nNoisyPixels << " nGoodPixels: " << nGoodPixels << endl;
}

// ----------------------------------------------------------------------
// cdbPrintPayload /path/to/payload
// ---------------
//
// ----------------------------------------------------------------------

int main(int argc, const char* argv[]) {
  string filename(""), pdir(""), hash("");
  int verbose(10000);

  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-v")) verbose = atoi(argv[++i]);
  }

  if (argc < 2) {
    cout << "provide a payload file" << endl;
    return 0;
  }
  filename = argv[argc-1];

  pdir = filename.substr(0, filename.find_last_of("/")+1);
  if (pdir.empty()) pdir = "./";
  hash = filename.substr(filename.find_last_of("/")+1);
  cout << "payload ->" << filename << "<-" << endl
       << "dir ->" << pdir << "<-" << endl
       << "hash ->" << hash << "<-" << endl;

  calAbs *c = Mu3eCalFactory::instance()->createClassFromFile(hash, pdir);
  if (!c) {
    cout << "unknown payload type or file not found" << endl;
    return 0;
  }
  if (string::npos != c->getError().find("Error: file not found")) {
    cout << "file not found" << endl;
    delete c;
    return 0;
  }

  printPayloadCommon(c, hash, verbose);

  calPixelQualityLM *cpq = dynamic_cast<calPixelQualityLM*>(c);
  if (cpq) {
    printPixelQualityLMStats(cpq);
  }

  delete c;
  return 0;
}
