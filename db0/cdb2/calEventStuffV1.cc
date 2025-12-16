#include "calEventStuffV1.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <sstream>


using namespace std;

// ----------------------------------------------------------------------
calEventStuffV1::calEventStuffV1(cdbAbs *db) : calAbs(db) {
}


// ----------------------------------------------------------------------
calEventStuffV1::calEventStuffV1(cdbAbs *db, string tag) : calAbs(db, tag) {
  if (0) cout << "calEventStuffV1 created and registered with tag ->"
              << tag << "<-"
              << endl;
}


// ----------------------------------------------------------------------
calEventStuffV1::~calEventStuffV1() {
  cout << "this is the end of calEventStuffV1 with tag ->" << fTag << "<-" << endl;
  }


// ----------------------------------------------------------------------
void calEventStuffV1::calculate(string hash) {
  cout << "calEventStuffV1::calculate() with "
       << "fHash ->" << hash << "<-";
  string spl = fTagIOVPayloadMap[hash].fBLOB;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  cout << " header: " << hex << header << dec << endl;

  fConstants.pixelData.startFrame = blob2Uint64(getData(ibuffer));
  fConstants.pixelData.endFrame = blob2Uint64(getData(ibuffer));
}


// ----------------------------------------------------------------------
void calEventStuffV1::printBLOB(std::string sblob, int verbosity) {
  cout << printBLOBString(sblob, verbosity) << endl;
}

// ----------------------------------------------------------------------
string calEventStuffV1::printBLOBString(std::string sblob, int verbosity) {
  stringstream ss;

  std::vector<char> buffer(sblob.begin(), sblob.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  ss << "calEventStuffV1::printBLOB(string)" << endl;
  ss << "   header: " << hex << header << dec << endl;

  if (0 == verbosity) return ss.str();

  ss << "pixeldata"  << endl
     << "  .startframe = " << blob2Uint64(getData(ibuffer)) << endl
     << "  .endframe = " << blob2Uint64(getData(ibuffer)) << endl;
  return ss.str();
}


// ----------------------------------------------------------------------
string calEventStuffV1::makeBLOB() {
  stringstream s;
  unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  s << dumpArray(uint642Blob(fConstants.pixelData.startFrame));
  s << dumpArray(uint642Blob(fConstants.pixelData.endFrame));
  return s.str();
}


// ----------------------------------------------------------------------
string calEventStuffV1::readJSON(string filename) {
  string spl("");
  ifstream INS(filename);
  if (!INS.is_open()) {
    return "calDetSetupV1::readJSON> Error, file " + filename + " not found";
  }

  string sline;
  while (getline(INS, sline)) {
    replaceAll(sline, "\n", " ");
    spl += sline;
  }
  INS.close();

  // -- figure out what type of JSON file this was
  // -- custom with "pixeldata" or midas meta data 
  if (string::npos == filename.find(".mid.lz4.json")) {
    fConstants.pixelData.startFrame = ::stoull(jsonGetValue(spl, vector<string> {"pixeldata", "startframe"}));
    fConstants.pixelData.endFrame    = ::stoul(jsonGetValue(spl, vector<string> {"pixeldata", "endframe"}));
  } else {

    string sstart_frame_good_pixel_data = jsonGetValue(spl, vector<string> {"stat", "start_frame_good_pixel_data"});
    string send_frame_good_pixel_data = jsonGetValue(spl, vector<string> {"stat", "end_frame_good_pixel_data"});
    cout << "start_frame_good_pixel_data ->" << sstart_frame_good_pixel_data << "<-" << endl;
    cout << "end_frame_good_pixel_data ->" << send_frame_good_pixel_data << "<-" << endl;
    fConstants.pixelData.startFrame = ::stoull(sstart_frame_good_pixel_data);
    fConstants.pixelData.endFrame    = ::stoull(send_frame_good_pixel_data);
  }
  return spl;
}
