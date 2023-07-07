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
string createPayload(string );

#define NCHIPS 20

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
  
  string spl = createPayload("bla");
  //  calPixelQuality *cal0 = dynamic_cast<calPixelQuality*>(pDC->createClass("pixelquality_"));

  calPixelQuality a;
  string hash("tag_pixelquality_mcideal_iov_1");
  a.readPayloadFromFile(".", hash);
  a.calculate(hash);

  uint32_t i(99999);
  while (a.getNextID(i)) {
    cout << i << ": " << static_cast<int>(a.getStatus(i, 108, 134)) << endl;
  }
}


// ----------------------------------------------------------------------
void writeBlob(string filename) {
  long unsigned int header(0xdeadface);
  ofstream ONS;
  ONS.open(filename);
  if (0) {
    printArray(ONS, uint2Blob(header));
  }
  if (1) {
    ONS << dumpArray(uint2Blob(header));
  }
  
  char data[8], data1[8], data2[8]; 
  for (unsigned int i = 0; i < NCHIPS; ++i) {
    int ndeadpix(20*gRandom->Rndm());
    cout << "ichip = " << i << " ndeadpix = " << ndeadpix << endl;
    ONS << dumpArray(uint2Blob(i)) 
        << dumpArray(int2Blob(ndeadpix));
    for (unsigned int ipix = 0; ipix < ndeadpix; ++ipix) {
      int icol = 100 + 50*gRandom->Rndm();
      int irow = 120 + 50*gRandom->Rndm();
      char iqual = 1; 
      cout << "icol/irow = " << icol << "/" << irow << endl;
      ONS << dumpArray(int2Blob(icol))
          << dumpArray(int2Blob(irow))
          << dumpArray(uint2Blob(static_cast<unsigned int>(iqual)));
    }
  }

  ONS.close();
}

// ----------------------------------------------------------------------  
string createPayload(string filename) {
  writeBlob(filename);
  
	auto builder = document{};
  std::ifstream file;
  file.open(filename);
  vector<char> buffer(std::istreambuf_iterator<char>(file), {});
  string sblob("");
  for (unsigned int i = 0; i < buffer.size(); ++i) sblob.push_back(buffer[i]);

  stringstream sh; 
  sh << "tag_" << "pixelquality_mcideal";
  sh << "_iov_" << "1";

  bsoncxx::document::value doc_value = builder
    << "hash" << sh.str()
    << "comment" << "testing"
    << "BLOB" << base64_encode(sblob)
    << finalize; 
  
  // -- JSON
  ofstream JS;
  JS.open("json/payloads/" + sh.str());
  if (JS.fail()) {
    cout << "Error failed to open " << "json/XXXXX" <<  endl;
  }
  JS << bsoncxx::to_json(doc_value.view()) << endl;
  JS.close();

  return bsoncxx::to_json(doc_value.view());
}