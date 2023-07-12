#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <string.h>

#include "Mu3eConditions.hh"
#include "cdbUtil.hh"
#include "calPixelQuality.hh"

#include "cdbJSON.hh"
#include "base64.hh"

#include "TRandom3.h"

#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>

using namespace std;

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::sub_array;
using bsoncxx::builder::basic::sub_document;
using bsoncxx::builder::basic::make_document;

void writeBlob(string);
void createPayload(string , calAbs *);

#define NCHIPS 4

// ----------------------------------------------------------------------
// testPixelQuality
// ----------------
//
// 
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  
  // -- command line arguments
  int verbose(0);
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-v"))   {verbose = atoi(argv[++i]);}
  }
  
  string gt("mcideal");
  cdbAbs *pDB = new cdbJSON(gt, "json", verbose);
  
  Mu3eConditions *pDC = Mu3eConditions::instance(gt, pDB);
  pDC->setVerbosity(verbose);
  
  calPixelQuality a;
  // -- create payload
  string hash("tag_pixelquality_mcideal_iov_1");
  createPayload(hash, &a);

  a.readPayloadFromFile(hash, ".");
  a.calculate(hash);

  uint32_t i(99999);
  while (a.getNextID(i)) {
    //    cout << i << ": " << static_cast<int>(a.getStatus(i, 108, 134)) << endl;
    a.printPixelQuality(i);
    cout << endl;
  }
}


// ----------------------------------------------------------------------
void writeBlob(string filename) {
  long unsigned int header(0xdeadface);
  ofstream ONS;
  ONS.open(filename);
  if (1) {
    ONS << dumpArray(uint2Blob(header));
  }
  
  char data[8], data1[8], data2[8]; 
  for (unsigned int i = 0; i < NCHIPS; ++i) {
    int ndeadpix(12*gRandom->Rndm());
    cout << "ichip = " << i << " ndeadpix = " << ndeadpix << endl;
    ONS << dumpArray(uint2Blob(i)) 
        << dumpArray(int2Blob(ndeadpix));
    for (unsigned int ipix = 0; ipix < ndeadpix; ++ipix) {
      int icol = 100 + 50*gRandom->Rndm();
      int irow = 120 + 50*gRandom->Rndm();
      char iqual = 1; 
      ONS << dumpArray(int2Blob(icol))
          << dumpArray(int2Blob(irow))
          << dumpArray(uint2Blob(static_cast<unsigned int>(iqual)));
      cout << "icol/irow = " << icol << "/" << irow
           << " qual = " << static_cast<unsigned int>(iqual) 
           << endl;
    }
  }

  ONS.close();
}


// ----------------------------------------------------------------------  
void createPayload(string hash, calAbs *a) {
  string tmpfilename("bla");
  writeBlob(tmpfilename);

  std::ifstream file;
  file.open(tmpfilename);
  vector<char> buffer(std::istreambuf_iterator<char>(file), {});
  string sblob("");
  for (unsigned int i = 0; i < buffer.size(); ++i) sblob.push_back(buffer[i]);

  payload pl;
  pl.fHash = hash;
  pl.fComment = "testing";
  pl.fBLOB = sblob;
  a->writePayloadToFile(hash, ".", pl); 
}
