#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <dirent.h>  /// for directory reading
#include <cstring>

#include <chrono>

#include "cdbUtil.hh"
#include "base64.hh"

#include "calPixelAlignment.hh"
#include "calFibreAlignment.hh"
#include "calMppcAlignment.hh"
#include "calTileAlignment.hh"
#include "calDetConfV1.hh"
#include "calDetSetupV1.hh"
#include "calEventStuffV1.hh"

#include "calPixelCablingMap.hh"
#include "calPixelQualityLM.hh"
#include "calPixelTimeCalibration.hh"
#include "calPixelEfficiency.hh"

#include "calFibreAlignment.hh"
#include "calTileQuality.hh"
#include "calFibreQuality.hh"


using namespace std;

// ----------------------------------------------------------------------
// cdbPrintPayload /path/to/payload
// ---------------
//
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------=
int main(int argc, const char* argv[]) {

  string filename(""), pdir(""), hash("");
  int verbose(10000);
  
  // -- commmand line parsing
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-v")) {
      verbose = atoi(argv[++i]);
    }
  }

  // -- command line arguments
  if (argc < 2) {
    cout << "provide a payload file" << endl;
    return 0;
  } else {
    filename = argv[argc-1];
  }
  
  pdir = filename.substr(0, filename.find_last_of("/")+1);
  if (pdir.empty()) {
    pdir = "./";
  }
  hash = filename.substr(filename.find_last_of("/")+1);
  cout << "payload ->" << filename  << "<-" << endl
       << "dir ->" << pdir << "<-" << endl
       << "hash ->" << hash << "<-" << endl;
       
  calAbs *c(0);
  if (string::npos != filename.find("pixelalignment_")) {
    c = new calPixelAlignment();
    c->readPayloadFromFile(hash, pdir);
    cout << "hash:    " << c->getPayload(hash).fHash << endl;
    cout << "comment: " << c->getPayload(hash).fComment << endl;
    cout << "schema:  " << c->getPayload(hash).fSchema << endl;
    cout << "date:    " << c->getPayload(hash).fDate << endl;
    c->printBLOB(c->getPayload(hash).fBLOB, verbose);
  } else if (string::npos != filename.find("fibrealignment_")) {
    c = new calFibreAlignment();
    c->readPayloadFromFile(hash, pdir);
    cout << "hash:    " << c->getPayload(hash).fHash << endl;
    cout << "comment: " << c->getPayload(hash).fComment << endl;
    cout << "schema:  " << c->getPayload(hash).fSchema << endl;
    cout << "date:    " << c->getPayload(hash).fDate << endl;
    c->printBLOB(c->getPayload(hash).fBLOB, verbose);
  } else if (string::npos != filename.find("mppcalignment_")) {
    c = new calMppcAlignment();
    c->readPayloadFromFile(hash, pdir);
    cout << "hash:    " << c->getPayload(hash).fHash << endl;
    cout << "comment: " << c->getPayload(hash).fComment << endl;
    cout << "schema:  " << c->getPayload(hash).fSchema << endl;
    cout << "date:    " << c->getPayload(hash).fDate << endl;
    c->printBLOB(c->getPayload(hash).fBLOB, verbose);
  } else if (string::npos != filename.find("tilealignment_")) {
    c = new calTileAlignment();
    c->readPayloadFromFile(hash, pdir);
    cout << "hash:    " << c->getPayload(hash).fHash << endl;
    cout << "comment: " << c->getPayload(hash).fComment << endl;
    cout << "schema:  " << c->getPayload(hash).fSchema << endl;
    cout << "date:    " << c->getPayload(hash).fDate << endl;
    c->printBLOB(c->getPayload(hash).fBLOB, verbose);
  } else if (string::npos != filename.find("detconfv1_")) {
    c = new calDetConfV1();
    c->readPayloadFromFile(hash, pdir);
    cout << "hash:    " << c->getPayload(hash).fHash << endl;
    cout << "comment: " << c->getPayload(hash).fComment << endl;
    cout << "schema:  " << c->getPayload(hash).fSchema << endl;
    cout << "date:    " << c->getPayload(hash).fDate << endl;
    c->printBLOB(c->getPayload(hash).fBLOB, verbose);
  } else if (string::npos != filename.find("detsetupv1_")) {
    c = new calDetSetupV1();
    c->readPayloadFromFile(hash, pdir);
    cout << "hash:    " << c->getPayload(hash).fHash << endl;
    cout << "comment: " << c->getPayload(hash).fComment << endl;
    cout << "schema:  " << c->getPayload(hash).fSchema << endl;
    cout << "date:    " << c->getPayload(hash).fDate << endl;
    c->printBLOB(c->getPayload(hash).fBLOB, verbose);
  } else if (string::npos != filename.find("pixelqualitylm_")) {
    c = new calPixelQualityLM();
    calPixelQualityLM *cpq = dynamic_cast<calPixelQualityLM*>(c);
    cpq->readPayloadFromFile(hash, pdir);
    cout << "hash:    " << cpq->getPayload(hash).fHash << endl;
    cout << "comment: " << cpq->getPayload(hash).fComment << endl;
    cout << "schema:  " << cpq->getPayload(hash).fSchema << endl;
    cout << "date:    " << cpq->getPayload(hash).fDate << endl;
    cpq->printBLOB(cpq->getPayload(hash).fBLOB, verbose);
    cpq->calculate(hash);
    cpq->resetIterator();
    uint32_t id;
    int nGoodLinks(0), nMaskedLinks(0), nDeadLinks(0), nStatus4Links(0), nStatus5Links(0);
    int nBadColumns(0);
    int nNoisyPixels(0), nGoodPixels(0);
    while (cpq->getNextID(id)) {
      for (int icol = 0; icol < 256; icol++) {
        if (cpq->getColStatus(id, icol) != calPixelQualityLM::Good) {
          nBadColumns++;
        }
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
      if (lvdsOverflowRate > 0.) { 
        cout << "chipID: " << id << " LVDS overflow rate: " << lvdsOverflowRate << endl; 
      }
    }
    cout << "nGoodLinks: " << nGoodLinks 
         << " nMaskedLinks: " << nMaskedLinks
         << " nDeadLinks: " << nDeadLinks
         << " nStatus4Links: " << nStatus4Links
         << " nStatus5Links: " << nStatus5Links
         << " nBadColumns: " << nBadColumns
         << " nNoisyPixels: " << nNoisyPixels 
         << " nGoodPixels: " << nGoodPixels 
         << endl;
  } else if (string::npos != filename.find("pixelcabling_")) {
    c = new calPixelCablingMap();
    c->readPayloadFromFile(hash, pdir);
    cout << "hash:    " << c->getPayload(hash).fHash << endl;
    cout << "comment: " << c->getPayload(hash).fComment << endl;
    cout << "schema:  " << c->getPayload(hash).fSchema << endl;
    cout << "date:    " << c->getPayload(hash).fDate << endl;
    c->printBLOB(c->getPayload(hash).fBLOB, verbose);
  } else if (string::npos != filename.find("pixeltimecalibration_")) {
    c = new calPixelTimeCalibration();
    c->readPayloadFromFile(hash, pdir);
    cout << "hash:    " << c->getPayload(hash).fHash << endl;
    cout << "comment: " << c->getPayload(hash).fComment << endl;
    cout << "schema:  " << c->getPayload(hash).fSchema << endl;
    cout << "date:    " << c->getPayload(hash).fDate << endl;
    c->printBLOB(c->getPayload(hash).fBLOB, verbose);
  } else if (string::npos != filename.find("tilequality_")) {
    c = new calTileQuality();
    c->readPayloadFromFile(hash, pdir);
    cout << "hash:    " << c->getPayload(hash).fHash << endl;
    cout << "comment: " << c->getPayload(hash).fComment << endl;
    cout << "schema:  " << c->getPayload(hash).fSchema << endl;
    cout << "date:    " << c->getPayload(hash).fDate << endl;
    c->printBLOB(c->getPayload(hash).fBLOB, verbose);
  } else if (string::npos != filename.find("fibrequality_")) {
    c = new calFibreQuality();
    c->readPayloadFromFile(hash, pdir);
    cout << "hash:    " << c->getPayload(hash).fHash << endl;
    cout << "comment: " << c->getPayload(hash).fComment << endl;
    cout << "schema:  " << c->getPayload(hash).fSchema << endl;
    cout << "date:    " << c->getPayload(hash).fDate << endl;
    c->printBLOB(c->getPayload(hash).fBLOB, verbose);
  } else if (string::npos != filename.find("pixelefficiency_")) {
    c = new calPixelEfficiency();
    c->readPayloadFromFile(hash, pdir);
    cout << "hash:    " << c->getPayload(hash).fHash << endl;
    cout << "comment: " << c->getPayload(hash).fComment << endl;
    cout << "schema:  " << c->getPayload(hash).fSchema << endl;
    cout << "date:    " << c->getPayload(hash).fDate << endl;
    c->printBLOB(c->getPayload(hash).fBLOB, verbose);
  } else if (string::npos != filename.find("eventstuffv1_")) {
    c = new calEventStuffV1();
    c->readPayloadFromFile(hash, pdir);
    cout << "hash:    " << c->getPayload(hash).fHash << endl;
    cout << "comment: " << c->getPayload(hash).fComment << endl;
    cout << "schema:  " << c->getPayload(hash).fSchema << endl;
    cout << "date:    " << c->getPayload(hash).fDate << endl;
    c->printBLOB(c->getPayload(hash).fBLOB, verbose);
  }
  
  
  return 0;
}
