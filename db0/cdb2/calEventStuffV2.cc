#include "calEventStuffV2.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;

// ----------------------------------------------------------------------
calEventStuffV2::calEventStuffV2(cdbAbs *db) : calAbs(db) {
}


// ----------------------------------------------------------------------
calEventStuffV2::calEventStuffV2(cdbAbs *db, string tag) : calAbs(db, tag) {
  if (0) cout << "calEventStuffV2 created and registered with tag ->"
              << tag << "<-"
              << endl;
}


// ----------------------------------------------------------------------
calEventStuffV2::~calEventStuffV2() {
  if (fVerbose > 0) cout << "this is the end of calEventStuffV2 with tag ->" << fTag << "<-" << endl;
  }


// ----------------------------------------------------------------------
void calEventStuffV2::calculate(string hash) {
  if (fVerbose > 0) cout << "calEventStuffV2::calculate() with "
       << "fHash ->" << hash << "<-";
  string spl = fTagIOVPayloadMap[hash].fBLOB;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  if (fVerbose > 0) cout << " header: " << hex << header << dec << " BLOB size: " << buffer.size() << endl;

  fConstants.eventData.startFrame = blob2Uint64(getData(ibuffer));
  fConstants.eventData.endFrame = blob2Uint64(getData(ibuffer));
  fConstants.eventData.firstFrameSkippedHeader = blob2Uint64(getData(ibuffer));

  fConstants.pixelData.startFrame = blob2Uint64(getData(ibuffer));
  fConstants.pixelData.endFrame = blob2Uint64(getData(ibuffer));

  fConstants.tileData.startFrame = blob2Uint64(getData(ibuffer));
  fConstants.tileData.endFrame = blob2Uint64(getData(ibuffer));

  fConstants.fibreData.startFrame = blob2Uint64(getData(ibuffer));
  fConstants.fibreData.endFrame = blob2Uint64(getData(ibuffer));
}


// ----------------------------------------------------------------------
void calEventStuffV2::printBLOB(std::string sblob, int verbosity) {
  cout << printBLOBString(sblob, verbosity) << endl;
}

// ----------------------------------------------------------------------
string calEventStuffV2::printBLOBString(std::string sblob, int verbosity) {
  stringstream ss;

  std::vector<char> buffer(sblob.begin(), sblob.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  ss << "calEventStuffV2::printBLOB(string)" << endl;
  ss << "   header: " << hex << header << dec << "   BLOB size: " << buffer.size() << endl;

  if (0 == verbosity) return ss.str();

  uint64_t firstEventframe = blob2Uint64(getData(ibuffer));
  uint64_t lastEventframe = blob2Uint64(getData(ibuffer));
  uint64_t firstframeskippedheader = blob2Uint64(getData(ibuffer));

  uint64_t startPixelframe = blob2Uint64(getData(ibuffer));
  uint64_t endPixelframe = blob2Uint64(getData(ibuffer));

  uint64_t startTileframe = blob2Uint64(getData(ibuffer));
  uint64_t endTileframe = blob2Uint64(getData(ibuffer));

  uint64_t startFibreframe = blob2Uint64(getData(ibuffer));
  uint64_t endFibreframe = blob2Uint64(getData(ibuffer));

  ss << "eventdata "  << endl
     << "  .firstframe = " << setw(22) << setfill(' ') << firstEventframe << " "
     <<  hex << "0x" << setw(16) << setfill('0') << firstEventframe << dec << endl
     << "  .lastframe =  " << setw(22) << setfill(' ') << lastEventframe << " "
     <<  hex << "0x" << setw(16) << setfill('0') << lastEventframe << dec << endl
     << "  .firstframeskippedheader = " << setw(22) << setfill(' ') << firstframeskippedheader << " "
     <<  hex << "0x" << setw(16) << setfill('0') << firstframeskippedheader << dec << endl
     << endl;
  ss << "pixeldata"  << endl
     << "  .startframe = " << setw(22) << setfill(' ') << startPixelframe << " "
     <<  hex << "0x" << setw(16) << setfill('0') << startPixelframe << dec << endl
     << "  .endframe =   " << setw(22) << setfill(' ') << endPixelframe << " "
     <<  hex << "0x" << setw(16) << setfill('0') << endPixelframe << dec << endl
     << endl;
  ss << "tiledata"  << endl
     << "  .startframe = " << setw(22) << setfill(' ') << startTileframe << " "
     <<  hex << "0x" << setw(16) << setfill('0') << startTileframe << dec << endl
     << "  .endframe =   " << setw(22) << setfill(' ') << endTileframe << " "
     <<  hex << "0x" << setw(16) << setfill('0') << endTileframe << dec << endl
     << endl;
  ss << "fibredata"  << endl
     << "  .startframe = " << setw(22) << setfill(' ') << startFibreframe << " "
     <<  hex << "0x" << setw(16) << setfill('0') << startFibreframe << dec << endl
     << "  .endframe =   " << setw(22) << setfill(' ') << endFibreframe << " "
     <<  hex << "0x" << setw(16) << setfill('0') << endFibreframe << dec << endl
     << endl;
  return ss.str();
}


// ----------------------------------------------------------------------
string calEventStuffV2::makeBLOB() {
  stringstream s;
  unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  s << dumpArray(uint642Blob(fConstants.eventData.startFrame));
  s << dumpArray(uint642Blob(fConstants.eventData.endFrame));
  s << dumpArray(uint642Blob(fConstants.eventData.firstFrameSkippedHeader));
  s << dumpArray(uint642Blob(fConstants.pixelData.startFrame));
  s << dumpArray(uint642Blob(fConstants.pixelData.endFrame));
  s << dumpArray(uint642Blob(fConstants.tileData.startFrame));
  s << dumpArray(uint642Blob(fConstants.tileData.endFrame));
  s << dumpArray(uint642Blob(fConstants.fibreData.startFrame));
  s << dumpArray(uint642Blob(fConstants.fibreData.endFrame));
  return s.str();
}


// ----------------------------------------------------------------------
string calEventStuffV2::makeBLOB(const std::map<unsigned int, std::vector<double>>&) {
  // EventStuff does not support map-based BLOB creation; use current constants
  return makeBLOB();
}


// ----------------------------------------------------------------------
string calEventStuffV2::readJSON(string filename) {
  string spl("");
  ifstream INS(filename);
  if (!INS.is_open()) {
    return "calDetSetupV2::readJSON> Error, file " + filename + " not found";
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
  fConstants.eventData.firstFrameSkippedHeader = -1;
  fConstants.pixelData.startFrame = 0; 
  fConstants.pixelData.endFrame   = -1;
  fConstants.tileData.startFrame = 0; 
  fConstants.tileData.endFrame   = -1;
  fConstants.fibreData.startFrame = 0; 
  fConstants.fibreData.endFrame   = -1;
  if (string::npos == filename.find(".mid.lz4.json")) {
    fConstants.eventData.startFrame = ::stoul(jsonGetValue(spl, vector<string>{"eventdata", "startframe"}));
    fConstants.eventData.endFrame   = ::stoul(jsonGetValue(spl, vector<string>{"eventdata", "endframe"}));
    fConstants.eventData.firstFrameSkippedHeader = ::stoul(jsonGetValue(spl, vector<string>{"eventdata", "firstframeskippedheader"}));
    fConstants.pixelData.startFrame = ::stoull(jsonGetValue(spl, vector<string>{"pixeldata", "startframe"}));
    fConstants.pixelData.endFrame    = ::stoul(jsonGetValue(spl, vector<string>{"pixeldata", "endframe"}));
    // fConstants.tileData.startFrame = ::stoul(jsonGetValue(spl, vector<string>{"tiledata", "startframe"}));
    // fConstants.tileData.endFrame    = ::stoul(jsonGetValue(spl, vector<string>{"tiledata", "endframe"}));
    // fConstants.fibreData.startFrame = ::stoul(jsonGetValue(spl, vector<string>{"fibredata", "startframe"}));
    // fConstants.fibreData.endFrame    = ::stoul(jsonGetValue(spl, vector<string>{"fibredata", "endframe"}));
  } else {
    string send_frame_event_data = jsonGetValue(spl, vector<string> {"stat", "last_frame"});
    string first_frame_skipped_header = jsonGetValue(spl, vector<string> {"stat", "first_frame_skipped_header"});
    string sstart_frame_good_pixel_data = jsonGetValue(spl, vector<string> {"stat", "start_frame_good_pixel_data"});
    string send_frame_good_pixel_data = jsonGetValue(spl, vector<string> {"stat", "end_frame_good_pixel_data"});
    
    cout << "FIXME correct parsing for all data" << endl;
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
