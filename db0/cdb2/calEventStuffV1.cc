#include "calEventStuffV1.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <sstream>
#include <iomanip>

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
  cout << " header: " << hex << header << dec << " BLOB size: " << buffer.size() << endl;

  fConstants.pixelData.startFrame = blob2Uint64(getData(ibuffer));
  fConstants.pixelData.endFrame = blob2Uint64(getData(ibuffer));
  fConstants.eventData.startFrame = blob2Uint64(getData(ibuffer));
  fConstants.eventData.endFrame = blob2Uint64(getData(ibuffer));
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
  ss << "   header: " << hex << header << dec << "   BLOB size: " << buffer.size() << endl;

  if (0 == verbosity) return ss.str();

  uint64_t startframe = blob2Uint64(getData(ibuffer));
  uint64_t endframe = blob2Uint64(getData(ibuffer));
  uint64_t firstframe{0}, lastframe{0xffffffffffffffff}; 
  bool have_eventdata = false;
  if (buffer.size() > 24) {
    firstframe = blob2Uint64(getData(ibuffer));
    lastframe = blob2Uint64(getData(ibuffer));
    have_eventdata = true;
  }
  ss << "pixeldata"  << endl
     << "  .startframe = " << setw(22) << setfill(' ') << startframe << " "
     <<  hex << "0x" << setw(16) << setfill('0') << startframe << dec << endl
     << "  .endframe =   " << setw(22) << setfill(' ') << endframe << " "
     <<  hex << "0x" << setw(16) << setfill('0') << endframe << dec << endl
     << endl;
  ss << "eventdata "  << (have_eventdata ? "(present)" : "(absent)") << endl
     << "  .firstframe = " << setw(22) << setfill(' ') << firstframe << " "
     <<  hex << "0x" << setw(16) << setfill('0') << firstframe << dec << endl
     << "  .lastframe =  " << setw(22) << setfill(' ') << lastframe << " "
     <<  hex << "0x" << setw(16) << setfill('0') << lastframe << dec << endl
     << endl;
  return ss.str();
}


// ----------------------------------------------------------------------
string calEventStuffV1::makeBLOB() {
  stringstream s;
  unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  s << dumpArray(uint642Blob(fConstants.pixelData.startFrame));
  s << dumpArray(uint642Blob(fConstants.pixelData.endFrame));
  s << dumpArray(uint642Blob(fConstants.eventData.startFrame));
  s << dumpArray(uint642Blob(fConstants.eventData.endFrame));
  return s.str();
}


// ----------------------------------------------------------------------
string calEventStuffV1::makeBLOB(const std::map<unsigned int, std::vector<double>>&) {
  // EventStuff does not support map-based BLOB creation; use current constants
  return makeBLOB();
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
  fConstants.eventData.startFrame = 0; 
  fConstants.eventData.endFrame   = -1;
  fConstants.pixelData.startFrame = 0; 
  fConstants.pixelData.endFrame   = -1;
  if (string::npos == filename.find(".mid.lz4.json")) {
    fConstants.pixelData.startFrame = ::stoull(jsonGetValue(spl, vector<string> {"pixeldata", "startframe"}));
    fConstants.pixelData.endFrame    = ::stoul(jsonGetValue(spl, vector<string> {"pixeldata", "endframe"}));
  } else {
    string sstart_frame_good_pixel_data = jsonGetValue(spl, vector<string> {"stat", "start_frame_good_pixel_data"});
    string send_frame_good_pixel_data = jsonGetValue(spl, vector<string> {"stat", "end_frame_good_pixel_data"});
    string send_frame_event_data = jsonGetValue(spl, vector<string> {"stat", "last_frame"});
    cout << "start_frame_good_pixel_data ->" << sstart_frame_good_pixel_data << "<-" << endl;
    cout << "end_frame_good_pixel_data ->" << send_frame_good_pixel_data << "<-" << endl;
    cout << "last_frame ->" << send_frame_event_data << "<-" << endl;
    if (sstart_frame_good_pixel_data != "parseError") {
      fConstants.pixelData.startFrame = ::stoull(sstart_frame_good_pixel_data);
    }
    if (send_frame_good_pixel_data != "parseError") {
      fConstants.pixelData.endFrame    = ::stoull(send_frame_good_pixel_data);
    }
    if (send_frame_event_data != "parseError") {
      fConstants.eventData.endFrame = ::stoull(send_frame_event_data);
    }
  }
  return spl;
}
