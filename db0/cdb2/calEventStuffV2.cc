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

  fConstants.eventData.startFrameGoodData = blob2Uint64(getData(ibuffer));
  fConstants.eventData.endFrameGoodData = blob2Uint64(getData(ibuffer));
  fConstants.eventData.firstFrameWithFEBProblems = blob2Uint64(getData(ibuffer));

  fConstants.pixelData.startFrameGoodData = blob2Uint64(getData(ibuffer));
  fConstants.pixelData.endFrameGoodData = blob2Uint64(getData(ibuffer));

  fConstants.tileData.startFrameGoodData = blob2Uint64(getData(ibuffer));
  fConstants.tileData.endFrameGoodData = blob2Uint64(getData(ibuffer));

  fConstants.fibreData.startFrameGoodData = blob2Uint64(getData(ibuffer));
  fConstants.fibreData.endFrameGoodData = blob2Uint64(getData(ibuffer));
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
  uint64_t firstframewithfebproblems = blob2Uint64(getData(ibuffer));

  uint64_t startPixelframe = blob2Uint64(getData(ibuffer));
  uint64_t endPixelframe = blob2Uint64(getData(ibuffer));
  uint64_t firstFrameWithFEBUnsortedHitDataPixel = blob2Uint64(getData(ibuffer));

  uint64_t startTileframe = blob2Uint64(getData(ibuffer));
  uint64_t endTileframe = blob2Uint64(getData(ibuffer));
  uint64_t firstFrameWithFEBUnsortedHitDataTile = blob2Uint64(getData(ibuffer));

  uint64_t startFibreframe = blob2Uint64(getData(ibuffer));
  uint64_t endFibreframe = blob2Uint64(getData(ibuffer));
  uint64_t firstFrameWithFEBUnsortedHitDataFibre = blob2Uint64(getData(ibuffer));

  ss << "eventdata "  << endl
     << "  .startframegooddata               = " << setw(22) << setfill(' ') << firstEventframe << " "
     <<  hex << "0x" << setw(16) << setfill('0') << firstEventframe << dec << endl
     << "  .endframegooddata                 = " << setw(22) << setfill(' ') << lastEventframe << " "
     <<  hex << "0x" << setw(16) << setfill('0') << lastEventframe << dec << endl
     << "  .firstframewithfebproblems        = " << setw(22) << setfill(' ') << firstframewithfebproblems << " "
     <<  hex << "0x" << setw(16) << setfill('0') << firstframewithfebproblems << dec << endl
     << endl;
  ss << "pixeldata"  << endl
     << "  .startframegooddata               = " << setw(22) << setfill(' ') << startPixelframe << " "
     <<  hex << "0x" << setw(16) << setfill('0') << startPixelframe << dec << endl
     << "  .endframegooddata                 = " << setw(22) << setfill(' ') << endPixelframe << " "
     <<  hex << "0x" << setw(16) << setfill('0') << endPixelframe << dec << endl
     << "  .firstframewithfebunsortedhitdata = " << setw(22) << setfill(' ') << firstFrameWithFEBUnsortedHitDataPixel << " "
     <<  hex << "0x" << setw(16) << setfill('0') << firstFrameWithFEBUnsortedHitDataPixel << dec << endl
     << endl;
  ss << "tiledata"  << endl
     << "  .startframegooddata               = " << setw(22) << setfill(' ') << startTileframe << " "
     <<  hex << "0x" << setw(16) << setfill('0') << startTileframe << dec << endl
     << "  .endframegooddata                 = " << setw(22) << setfill(' ') << endTileframe << " "
     <<  hex << "0x" << setw(16) << setfill('0') << endTileframe << dec << endl
     << "  .firstframewithfebunsortedhitdata = " << setw(22) << setfill(' ') << firstFrameWithFEBUnsortedHitDataTile << " "
     <<  hex << "0x" << setw(16) << setfill('0') << firstFrameWithFEBUnsortedHitDataTile << dec << endl
     << endl;
  ss << "fibredata"  << endl
     << "  .startframegooddata               = " << setw(22) << setfill(' ') << startFibreframe << " "
     <<  hex << "0x" << setw(16) << setfill('0') << startFibreframe << dec << endl
     << "  .endframegooddata                 = " << setw(22) << setfill(' ') << endFibreframe << " "
     <<  hex << "0x" << setw(16) << setfill('0') << endFibreframe << dec << endl
     << "  .firstframewithfebunsortedhitdata = " << setw(22) << setfill(' ') << firstFrameWithFEBUnsortedHitDataFibre << " "
     <<  hex << "0x" << setw(16) << setfill('0') << firstFrameWithFEBUnsortedHitDataFibre << dec << endl
     << endl;
  return ss.str();
}


// ----------------------------------------------------------------------
string calEventStuffV2::makeBLOB() {
  stringstream s;
  unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  s << dumpArray(uint642Blob(fConstants.eventData.startFrameGoodData));
  s << dumpArray(uint642Blob(fConstants.eventData.endFrameGoodData));
  s << dumpArray(uint642Blob(fConstants.eventData.firstFrameWithFEBProblems));
  s << dumpArray(uint642Blob(fConstants.pixelData.startFrameGoodData));
  s << dumpArray(uint642Blob(fConstants.pixelData.endFrameGoodData));
  s << dumpArray(uint642Blob(fConstants.pixelData.firstFrameWithFEBUnsortedHitData));
  s << dumpArray(uint642Blob(fConstants.tileData.startFrameGoodData));
  s << dumpArray(uint642Blob(fConstants.tileData.endFrameGoodData));
  s << dumpArray(uint642Blob(fConstants.tileData.firstFrameWithFEBUnsortedHitData));
  s << dumpArray(uint642Blob(fConstants.fibreData.startFrameGoodData));
  s << dumpArray(uint642Blob(fConstants.fibreData.endFrameGoodData));
  s << dumpArray(uint642Blob(fConstants.fibreData.firstFrameWithFEBUnsortedHitData));
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
  fConstants.eventData.startFrameGoodData = 0; 
  fConstants.eventData.endFrameGoodData   = -1;
  fConstants.eventData.firstFrameWithFEBProblems = -1;
  fConstants.pixelData.startFrameGoodData = 0; 
  fConstants.pixelData.endFrameGoodData   = -1;
  fConstants.tileData.startFrameGoodData = 0; 
  fConstants.tileData.endFrameGoodData   = -1;
  fConstants.fibreData.startFrameGoodData = 0; 
  fConstants.fibreData.endFrameGoodData   = -1;
  if (string::npos == filename.find(".mid.lz4.json")) {
    fConstants.eventData.startFrameGoodData = ::stoul(jsonGetValue(spl, vector<string>{"eventdata", "startframegooddata"}));
    fConstants.eventData.endFrameGoodData   = ::stoul(jsonGetValue(spl, vector<string>{"eventdata", "endframegooddata"}));
    fConstants.eventData.firstFrameWithFEBProblems = ::stoul(jsonGetValue(spl, vector<string>{"eventdata", "firstframewithfebproblems"}));
    fConstants.pixelData.startFrameGoodData = ::stoull(jsonGetValue(spl, vector<string>{"pixeldata", "startframegooddata"}));
    fConstants.pixelData.endFrameGoodData    = ::stoul(jsonGetValue(spl, vector<string>{"pixeldata", "endframegooddata"}));
    fConstants.pixelData.firstFrameWithFEBUnsortedHitData = ::stoul(jsonGetValue(spl, vector<string>{"pixeldata", "firstframewithfebunsortedhitdata"}));
    // fConstants.tileData.startFrameGoodData = ::stoul(jsonGetValue(spl, vector<string>{"tiledata", "startframegooddata"}));
    // fConstants.tileData.endFrameGoodData    = ::stoul(jsonGetValue(spl, vector<string>{"tiledata", "endframegooddata"}));
    // fConstants.tileData.firstFrameWithFEBUnsortedHitData = ::stoul(jsonGetValue(spl, vector<string>{"tiledata", "firstframewithfebunsortedhitdata"}));
    // fConstants.fibreData.startFrameGoodData = ::stoul(jsonGetValue(spl, vector<string>{"fibredata", "startframegooddata"}));
    // fConstants.fibreData.endFrameGoodData    = ::stoul(jsonGetValue(spl, vector<string>{"fibredata", "endframegooddata"}));
    // fConstants.fibreData.firstFrameWithFEBUnsortedHitData = ::stoul(jsonGetValue(spl, vector<string>{"fibredata", "firstframewithfebunsortedhitdata"}));
  } else {
    string send_frame_event_data = jsonGetValue(spl, vector<string> {"stat", "last_frame_of_the_run"});
    string sstart_frame_good_pixel_data = jsonGetValue(spl, vector<string> {"stat", "start_frame_good_pixel_data"});
    string send_frame_good_pixel_data = jsonGetValue(spl, vector<string> {"stat", "end_frame_good_pixel_data"});
    string first_frame_pixel_skipped_header = jsonGetValue(spl, vector<string> {"stat", "first_frame_at_least_one_pixel_FEB_had_unsorted_hit_data"});
    
    cout << "FIXME correct parsing for all data" << endl;
    cout << "start_frame_good_pixel_data ->" << sstart_frame_good_pixel_data << "<-" << endl;
    cout << "end_frame_good_pixel_data ->" << send_frame_good_pixel_data << "<-" << endl;
    cout << "last_frame ->" << send_frame_event_data << "<-" << endl;

    if (send_frame_event_data != "parseError") {
      fConstants.eventData.endFrameGoodData = ::stoull(send_frame_event_data);
    }

    if (sstart_frame_good_pixel_data != "parseError") {
      fConstants.pixelData.startFrameGoodData = ::stoull(sstart_frame_good_pixel_data);
    }
    if (send_frame_good_pixel_data != "parseError") {
      fConstants.pixelData.endFrameGoodData    = ::stoull(send_frame_good_pixel_data);
    }
    if (first_frame_pixel_skipped_header != "parseError") {
      fConstants.pixelData.firstFrameWithFEBUnsortedHitData = ::stoull(first_frame_pixel_skipped_header);
    }
  }
  return spl;
}
