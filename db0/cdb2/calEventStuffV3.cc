#include "calEventStuffV3.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <ostream>
#include <sstream>
#include <iomanip>

using namespace std;

// ----------------------------------------------------------------------
calEventStuffV3::calEventStuffV3(cdbAbs *db) : calAbs(db) {
  fSchema = "eventdata.(ull_startframedata,ull_endframedata,ull_firstframewithfebproblems,ull_n_ts,ull_n_unexpected_ts,ull_n_midasevent,ull_n_inv_dsin)";
  fSchema += ",pixeldata.(ull_startframegooddata,ull_endframegooddata,ull_firstframewithfebunsortedhitdata,b_pixel_febs_ok,ull_n_phit,ull_n_phit_dup,s_maskdirname,b_maskdirname_ok)";
  fSchema += ",tiledata.(ull_startframegooddata,ull_endframegooddata,ull_firstframewithfebunsortedhitdata,ull_n_thit,b_tile_febs_ok)";
  fSchema += ",fibredata.(ull_startframegooddata,ull_endframegooddata,ull_firstframewithfebunsortedhitdata,ull_n_fhit,b_fibre_febs_ok)";
  fSchema += ",daqdata.(i_n_deff[,s_deff_name][,d_deff_value],i_n_link[,s_lvde_name][,d_lvde_value])";
  fSchema += ",magnetdata.(d_current_0,d_current_1)";
  fSchema += ",beamdata.(d_beam_shutter,d_proton_current,s_filename)";
  fSchema += ",reservedata.(i_nint[,s_int_name][,i_int_value],i_nuint64[,s_uint64_name][,ull_uint64_value],i_ndouble[,s_double_name][,d_double_value])";
}


// ----------------------------------------------------------------------
calEventStuffV3::calEventStuffV3(cdbAbs *db, string tag) : calAbs(db, tag) {
  if (0) cout << "calEventStuffV3 created and registered with tag ->"
              << tag << "<-"
              << endl;
}


// ----------------------------------------------------------------------
calEventStuffV3::~calEventStuffV3() {
  if (fVerbose > 0) cout << "this is the end of calEventStuffV3 with tag ->" << fTag << "<-" << endl;
  }


// ----------------------------------------------------------------------
// A name that is not in the list returns 0.
double calEventStuffV3::deff(const string& name) {
  size_t n = min(fConstants.daqData.deff_names.size(), fConstants.daqData.deff_values.size());
  for (size_t i = 0; i < n; ++i) {
    if (fConstants.daqData.deff_names[i] == name) return fConstants.daqData.deff_values[i];
  }
  return 0.0;
}


// ----------------------------------------------------------------------
double calEventStuffV3::lvde(const string& name) {
  size_t n = min(fConstants.daqData.lvde_names.size(), fConstants.daqData.lvde_values.size());
  for (size_t i = 0; i < n; ++i) {
    if (fConstants.daqData.lvde_names[i] == name) return fConstants.daqData.lvde_values[i];
  }
  return 0.0;
}


// ----------------------------------------------------------------------
int calEventStuffV3::reserveInt(const string& name) {
  size_t n = min(fConstants.reserveData.int_names.size(), fConstants.reserveData.int_values.size());
  for (size_t i = 0; i < n; ++i) {
    if (fConstants.reserveData.int_names[i] == name) return fConstants.reserveData.int_values[i];
  }
  return 0;
}


// ----------------------------------------------------------------------
uint64_t calEventStuffV3::reserveUint64(const string& name) {
  size_t n = min(fConstants.reserveData.uint64_names.size(), fConstants.reserveData.uint64_values.size());
  for (size_t i = 0; i < n; ++i) {
    if (fConstants.reserveData.uint64_names[i] == name) return fConstants.reserveData.uint64_values[i];
  }
  return 0;
}


// ----------------------------------------------------------------------
double calEventStuffV3::reserveDouble(const string& name) {
  size_t n = min(fConstants.reserveData.double_names.size(), fConstants.reserveData.double_values.size());
  for (size_t i = 0; i < n; ++i) {
    if (fConstants.reserveData.double_names[i] == name) return fConstants.reserveData.double_values[i];
  }
  return 0.0;
}


// ----------------------------------------------------------------------
// Wire order is fSchema. Each scalar is one 8-byte word. A list is a count,
// then that many strings, then that many values. Bool is an int 0/1.
void calEventStuffV3::readBLOB(vector<char>::iterator& it, constants& c) {
  c.eventData.startFrameData = blob2Uint64(getData(it));
  c.eventData.endFrameData = blob2Uint64(getData(it));
  c.eventData.firstFrameWithFEBProblems = blob2Uint64(getData(it));
  c.eventData.n_ts = blob2Uint64(getData(it));
  c.eventData.n_unexpected_ts = blob2Uint64(getData(it));
  c.eventData.n_midasEvent = blob2Uint64(getData(it));
  c.eventData.n_inv_dsin = blob2Uint64(getData(it));

  c.pixelData.startFrameGoodData = blob2Uint64(getData(it));
  c.pixelData.endFrameGoodData = blob2Uint64(getData(it));
  c.pixelData.firstFrameWithFEBUnsortedHitData = blob2Uint64(getData(it));
  c.pixelData.pixel_febs_ok = blob2Int(getData(it)) != 0;
  c.pixelData.n_phit = blob2Uint64(getData(it));
  c.pixelData.n_phit_dup = blob2Uint64(getData(it));
  c.pixelData.maskdirname = getStringData(it);
  c.pixelData.maskdirname_ok = blob2Int(getData(it)) != 0;

  c.tileData.startFrameGoodData = blob2Uint64(getData(it));
  c.tileData.endFrameGoodData = blob2Uint64(getData(it));
  c.tileData.firstFrameWithFEBUnsortedHitData = blob2Uint64(getData(it));
  c.tileData.n_thit = blob2Uint64(getData(it));
  c.tileData.tile_febs_ok = blob2Int(getData(it)) != 0;

  c.fibreData.startFrameGoodData = blob2Uint64(getData(it));
  c.fibreData.endFrameGoodData = blob2Uint64(getData(it));
  c.fibreData.firstFrameWithFEBUnsortedHitData = blob2Uint64(getData(it));
  c.fibreData.n_fhit = blob2Uint64(getData(it));
  c.fibreData.fibre_febs_ok = blob2Int(getData(it)) != 0;

  c.daqData.n_deff = blob2Int(getData(it));
  if (c.daqData.n_deff < 0) c.daqData.n_deff = 0;
  c.daqData.deff_names.clear();
  c.daqData.deff_values.clear();
  for (int i = 0; i < c.daqData.n_deff; ++i) c.daqData.deff_names.push_back(getStringData(it));
  for (int i = 0; i < c.daqData.n_deff; ++i) c.daqData.deff_values.push_back(blob2Double(getData(it)));

  c.daqData.n_link = blob2Int(getData(it));
  if (c.daqData.n_link < 0) c.daqData.n_link = 0;
  c.daqData.lvde_names.clear();
  c.daqData.lvde_values.clear();
  for (int i = 0; i < c.daqData.n_link; ++i) c.daqData.lvde_names.push_back(getStringData(it));
  for (int i = 0; i < c.daqData.n_link; ++i) c.daqData.lvde_values.push_back(blob2Double(getData(it)));

  c.magnetData.current_0 = blob2Double(getData(it));
  c.magnetData.current_1 = blob2Double(getData(it));

  c.beamData.beam_shutter = blob2Double(getData(it));
  c.beamData.proton_current = blob2Double(getData(it));
  c.beamData.filename = getStringData(it);

  c.reserveData.nInt = blob2Int(getData(it));
  if (c.reserveData.nInt < 0) c.reserveData.nInt = 0;
  c.reserveData.int_names.clear();
  c.reserveData.int_values.clear();
  for (int i = 0; i < c.reserveData.nInt; ++i) c.reserveData.int_names.push_back(getStringData(it));
  for (int i = 0; i < c.reserveData.nInt; ++i) c.reserveData.int_values.push_back(blob2Int(getData(it)));

  c.reserveData.nUint64 = blob2Int(getData(it));
  if (c.reserveData.nUint64 < 0) c.reserveData.nUint64 = 0;
  c.reserveData.uint64_names.clear();
  c.reserveData.uint64_values.clear();
  for (int i = 0; i < c.reserveData.nUint64; ++i) c.reserveData.uint64_names.push_back(getStringData(it));
  for (int i = 0; i < c.reserveData.nUint64; ++i) c.reserveData.uint64_values.push_back(blob2Uint64(getData(it)));

  c.reserveData.nDouble = blob2Int(getData(it));
  if (c.reserveData.nDouble < 0) c.reserveData.nDouble = 0;
  c.reserveData.double_names.clear();
  c.reserveData.double_values.clear();
  for (int i = 0; i < c.reserveData.nDouble; ++i) c.reserveData.double_names.push_back(getStringData(it));
  for (int i = 0; i < c.reserveData.nDouble; ++i) c.reserveData.double_values.push_back(blob2Double(getData(it)));
}


// ----------------------------------------------------------------------
void calEventStuffV3::calculate(string hash) {
  if (fVerbose > 0) cout << "calEventStuffV3::calculate() with "
       << "fHash ->" << hash << "<-";
  string spl = fTagIOVPayloadMap[hash].fBLOB;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  if (fVerbose > 0) cout << " header: " << hex << header << dec << " BLOB size: " << buffer.size() << endl;

  fIovRun = runFromHash(hash);
  readBLOB(ibuffer, fConstants);
}


// ----------------------------------------------------------------------
void calEventStuffV3::printBLOB(std::string sblob, int verbosity) {
  cout << printBLOBString(sblob, verbosity) << endl;
}

// ----------------------------------------------------------------------
string calEventStuffV3::printBLOBString(std::string sblob, int verbosity) {
  stringstream ss;

  std::vector<char> buffer(sblob.begin(), sblob.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  ss << "calEventStuffV3::printBLOB(string)" << endl;
  ss << "   header: " << hex << header << dec << "   BLOB size: " << buffer.size() << endl;

  if (0 == verbosity) return ss.str();

  constants c;
  readBLOB(ibuffer, c);

  auto ull = [&](const string& name, uint64_t v) {
    ss << "  ." << setw(33) << left << name << " = " << setw(22) << right << setfill(' ') << v << " "
       << hex << "0x" << setw(16) << setfill('0') << v << dec << setfill(' ') << endl;
  };
  auto showDoubles = [&](const string& title, int n, const vector<string>& names, const vector<double>& values) {
    ss << title << " n=" << n << endl;
    for (int i = 0; i < n; ++i) {
      ss << "  " << (i < (int)names.size() ? names[i] : string("?"))
         << " = " << (i < (int)values.size() ? values[i] : 0.0) << endl;
    }
  };

  ss << "eventdata" << endl;
  ull("startframedata", c.eventData.startFrameData);
  ull("endframedata", c.eventData.endFrameData);
  ull("firstframewithfebproblems", c.eventData.firstFrameWithFEBProblems);
  ull("n_ts", c.eventData.n_ts);
  ull("n_unexpected_ts", c.eventData.n_unexpected_ts);
  ull("n_midasevent", c.eventData.n_midasEvent);
  ull("n_inv_dsin", c.eventData.n_inv_dsin);

  ss << "pixeldata" << endl;
  ull("startframegooddata", c.pixelData.startFrameGoodData);
  ull("endframegooddata", c.pixelData.endFrameGoodData);
  ull("firstframewithfebunsortedhitdata", c.pixelData.firstFrameWithFEBUnsortedHitData);
  ss << "  .pixel_febs_ok                    = " << c.pixelData.pixel_febs_ok << endl;
  ull("n_phit", c.pixelData.n_phit);
  ull("n_phit_dup", c.pixelData.n_phit_dup);
  ss << "  .maskdirname                      = " << c.pixelData.maskdirname << endl;
  ss << "  .maskdirname_ok                   = " << c.pixelData.maskdirname_ok << endl;

  ss << "tiledata" << endl;
  ull("startframegooddata", c.tileData.startFrameGoodData);
  ull("endframegooddata", c.tileData.endFrameGoodData);
  ull("firstframewithfebunsortedhitdata", c.tileData.firstFrameWithFEBUnsortedHitData);
  ull("n_thit", c.tileData.n_thit);
  ss << "  .tile_febs_ok                     = " << c.tileData.tile_febs_ok << endl;

  ss << "fibredata" << endl;
  ull("startframegooddata", c.fibreData.startFrameGoodData);
  ull("endframegooddata", c.fibreData.endFrameGoodData);
  ull("firstframewithfebunsortedhitdata", c.fibreData.firstFrameWithFEBUnsortedHitData);
  ull("n_fhit", c.fibreData.n_fhit);
  ss << "  .fibre_febs_ok                    = " << c.fibreData.fibre_febs_ok << endl;

  showDoubles("daqdata deff", c.daqData.n_deff, c.daqData.deff_names, c.daqData.deff_values);
  showDoubles("daqdata lvde", c.daqData.n_link, c.daqData.lvde_names, c.daqData.lvde_values);

  ss << "magnetdata" << endl
     << "  .current_0 = " << c.magnetData.current_0 << endl
     << "  .current_1 = " << c.magnetData.current_1 << endl;
  ss << "beamdata" << endl
     << "  .beam_shutter = " << c.beamData.beam_shutter << endl
     << "  .proton_current = " << c.beamData.proton_current << endl
     << "  .filename = " << c.beamData.filename << endl;

  ss << "reservedata int n=" << c.reserveData.nInt << endl;
  for (int i = 0; i < c.reserveData.nInt; ++i) {
    ss << "  " << (i < (int)c.reserveData.int_names.size() ? c.reserveData.int_names[i] : string("?"))
       << " = " << (i < (int)c.reserveData.int_values.size() ? c.reserveData.int_values[i] : 0) << endl;
  }
  ss << "reservedata uint64 n=" << c.reserveData.nUint64 << endl;
  for (int i = 0; i < c.reserveData.nUint64; ++i) {
    ss << "  " << (i < (int)c.reserveData.uint64_names.size() ? c.reserveData.uint64_names[i] : string("?"))
       << " = " << (i < (int)c.reserveData.uint64_values.size() ? c.reserveData.uint64_values[i] : 0) << endl;
  }
  showDoubles("reservedata double", c.reserveData.nDouble, c.reserveData.double_names, c.reserveData.double_values);
  return ss.str();
}


// ----------------------------------------------------------------------
string calEventStuffV3::makeBLOB() {
  stringstream s;
  auto u64 = [&](uint64_t v) { s << dumpArray(uint642Blob(v)); };
  auto i32 = [&](int v) { s << dumpArray(int2Blob(v)); };
  auto dbl = [&](double v) { s << dumpArray(double2Blob(v)); };
  auto str = [&](const string& t) { s << dumpVector(string2Blob(t)); };
  auto flag = [&](bool v) { i32(v ? 1 : 0); };

  s << dumpArray(uint2Blob(0xdeadface));
  u64(fConstants.eventData.startFrameData);
  u64(fConstants.eventData.endFrameData);
  u64(fConstants.eventData.firstFrameWithFEBProblems);
  u64(fConstants.eventData.n_ts);
  u64(fConstants.eventData.n_unexpected_ts);
  u64(fConstants.eventData.n_midasEvent);
  u64(fConstants.eventData.n_inv_dsin);

  u64(fConstants.pixelData.startFrameGoodData);
  u64(fConstants.pixelData.endFrameGoodData);
  u64(fConstants.pixelData.firstFrameWithFEBUnsortedHitData);
  flag(fConstants.pixelData.pixel_febs_ok);
  u64(fConstants.pixelData.n_phit);
  u64(fConstants.pixelData.n_phit_dup);
  str(fConstants.pixelData.maskdirname);
  flag(fConstants.pixelData.maskdirname_ok);

  u64(fConstants.tileData.startFrameGoodData);
  u64(fConstants.tileData.endFrameGoodData);
  u64(fConstants.tileData.firstFrameWithFEBUnsortedHitData);
  u64(fConstants.tileData.n_thit);
  flag(fConstants.tileData.tile_febs_ok);

  u64(fConstants.fibreData.startFrameGoodData);
  u64(fConstants.fibreData.endFrameGoodData);
  u64(fConstants.fibreData.firstFrameWithFEBUnsortedHitData);
  u64(fConstants.fibreData.n_fhit);
  flag(fConstants.fibreData.fibre_febs_ok);

  i32(fConstants.daqData.n_deff);
  for (int i = 0; i < fConstants.daqData.n_deff; ++i) {
    str(i < (int)fConstants.daqData.deff_names.size() ? fConstants.daqData.deff_names[i] : string());
  }
  for (int i = 0; i < fConstants.daqData.n_deff; ++i) {
    dbl(i < (int)fConstants.daqData.deff_values.size() ? fConstants.daqData.deff_values[i] : 0.0);
  }
  i32(fConstants.daqData.n_link);
  for (int i = 0; i < fConstants.daqData.n_link; ++i) {
    str(i < (int)fConstants.daqData.lvde_names.size() ? fConstants.daqData.lvde_names[i] : string());
  }
  for (int i = 0; i < fConstants.daqData.n_link; ++i) {
    dbl(i < (int)fConstants.daqData.lvde_values.size() ? fConstants.daqData.lvde_values[i] : 0.0);
  }

  dbl(fConstants.magnetData.current_0);
  dbl(fConstants.magnetData.current_1);
  dbl(fConstants.beamData.beam_shutter);
  dbl(fConstants.beamData.proton_current);
  str(fConstants.beamData.filename);

  i32(fConstants.reserveData.nInt);
  for (int i = 0; i < fConstants.reserveData.nInt; ++i) {
    str(i < (int)fConstants.reserveData.int_names.size() ? fConstants.reserveData.int_names[i] : string());
  }
  for (int i = 0; i < fConstants.reserveData.nInt; ++i) {
    i32(i < (int)fConstants.reserveData.int_values.size() ? fConstants.reserveData.int_values[i] : 0);
  }
  i32(fConstants.reserveData.nUint64);
  for (int i = 0; i < fConstants.reserveData.nUint64; ++i) {
    str(i < (int)fConstants.reserveData.uint64_names.size() ? fConstants.reserveData.uint64_names[i] : string());
  }
  for (int i = 0; i < fConstants.reserveData.nUint64; ++i) {
    u64(i < (int)fConstants.reserveData.uint64_values.size() ? fConstants.reserveData.uint64_values[i] : 0);
  }
  i32(fConstants.reserveData.nDouble);
  for (int i = 0; i < fConstants.reserveData.nDouble; ++i) {
    str(i < (int)fConstants.reserveData.double_names.size() ? fConstants.reserveData.double_names[i] : string());
  }
  for (int i = 0; i < fConstants.reserveData.nDouble; ++i) {
    dbl(i < (int)fConstants.reserveData.double_values.size() ? fConstants.reserveData.double_values[i] : 0.0);
  }
  return s.str();
}


// ----------------------------------------------------------------------
string calEventStuffV3::makeBLOB(const std::map<unsigned int, std::vector<double>>&) {
  // EventStuff does not support map-based BLOB creation; use current constants
  return makeBLOB();
}


namespace {

const string kMidasSuffix = ".mid.lz4.json";

string readTextFile(const string& filename) {
  ifstream in(filename);
  if (!in.is_open()) return "";
  stringstream buf;
  buf << in.rdbuf();
  return buf.str();
}

bool endsWith(const string& s, const string& suf) {
  return s.size() >= suf.size() && s.compare(s.size() - suf.size(), suf.size(), suf) == 0;
}

// Quoted JSON string array. jsonGetVector removes every space, which joins
// names such as "Central pixel sorter".
vector<string> jsonStringArray(const string& jstring, const string& key) {
  vector<string> out;
  string search = "\"" + key + "\"";
  size_t s0 = jstring.find(search);
  if (s0 == string::npos) return out;
  size_t colon = jstring.find(':', s0 + search.size());
  if (colon == string::npos) return out;
  size_t lb = jstring.find('[', colon);
  size_t rb = jstring.find(']', lb);
  if (lb == string::npos || rb == string::npos || rb < lb) return out;
  string body = jstring.substr(lb + 1, rb - lb - 1);
  for (size_t i = 0; i < body.size(); ) {
    size_t q1 = body.find('"', i);
    if (q1 == string::npos) break;
    size_t q2 = body.find('"', q1 + 1);
    if (q2 == string::npos) break;
    out.push_back(body.substr(q1 + 1, q2 - q1 - 1));
    i = q2 + 1;
  }
  return out;
}

vector<double> jsonDoubleArray(const string& jstring, const string& key) {
  vector<double> out;
  string body = jsonGetVector(jstring, key);
  if (body == "parseError" || body.empty()) return out;
  vector<string> toks;
  split(body, ',', toks);
  for (const string& t : toks) {
    if (t.empty()) continue;
    try {
      out.push_back(stod(t));
    } catch (...) {
    }
  }
  return out;
}

bool takeU64(uint64_t& dst, const string& src) {
  if (src.empty() || src == "parseError") return false;
  try {
    dst = stoull(src);
    return true;
  } catch (...) {
    return false;
  }
}

bool takeDouble(double& dst, const string& src) {
  if (src.empty() || src == "parseError") return false;
  try {
    dst = stod(src);
    return true;
  } catch (...) {
    return false;
  }
}

bool takeBool(bool& dst, const string& src) {
  if (src == "true" || src == "1") {
    dst = true;
    return true;
  }
  if (src == "false" || src == "0") {
    dst = false;
    return true;
  }
  return false;
}

// Directory of a path, without the filename. No slash means there is no directory.
string directoryName(const string& path) {
  size_t slash = path.find_last_of('/');
  if (slash == string::npos) return "";
  return path.substr(0, slash);
}

}  // namespace


// ----------------------------------------------------------------------
string calEventStuffV3::readJSON(string filename) {
  string text = readTextFile(filename);
  if (text.empty()) {
    return "calEventStuffV3::readJSON> Error, file " + filename + " not found";
  }
  fConstants = constants();

  auto parseMidas = [&](const string& midas) {
    if (midas.empty()) return;
    takeU64(fConstants.eventData.endFrameData, jsonGetValue(midas, vector<string>{"stat", "last_frame_of_the_run"}));
    takeU64(fConstants.eventData.n_ts, jsonGetValue(midas, vector<string>{"stat", "n_ts"}));
    takeU64(fConstants.eventData.n_unexpected_ts, jsonGetValue(midas, vector<string>{"stat", "n_unexp_ts"}));
    takeU64(fConstants.eventData.n_midasEvent, jsonGetValue(midas, vector<string>{"stat", "n_midasEvent"}));
    takeU64(fConstants.eventData.n_inv_dsin, jsonGetValue(midas, vector<string>{"stat", "n_inv_dsin"}));

    takeU64(fConstants.pixelData.startFrameGoodData, jsonGetValue(midas, vector<string>{"stat", "start_frame_good_pixel_data"}));
    takeU64(fConstants.pixelData.endFrameGoodData, jsonGetValue(midas, vector<string>{"stat", "end_frame_good_pixel_data"}));
    takeU64(fConstants.pixelData.firstFrameWithFEBUnsortedHitData,
            jsonGetValue(midas, vector<string>{"stat", "first_frame_at_least_one_pixel_FEB_had_unsorted_hit_data"}));
    takeBool(fConstants.pixelData.pixel_febs_ok, jsonGetValue(midas, vector<string>{"stat", "pixel_febs_ok"}));
    takeU64(fConstants.pixelData.n_phit, jsonGetValue(midas, vector<string>{"stat", "n_phit"}));
    takeU64(fConstants.pixelData.n_phit_dup, jsonGetValue(midas, vector<string>{"stat", "n_phit_dup"}));

    takeBool(fConstants.tileData.tile_febs_ok, jsonGetValue(midas, vector<string>{"stat", "tile_febs_ok"}));
    takeU64(fConstants.tileData.n_thit, jsonGetValue(midas, vector<string>{"stat", "n_thit"}));

    takeBool(fConstants.fibreData.fibre_febs_ok, jsonGetValue(midas, vector<string>{"stat", "scifi_febs_ok"}));
    takeU64(fConstants.fibreData.n_fhit, jsonGetValue(midas, vector<string>{"stat", "n_fhit"}));

    takeDouble(fConstants.magnetData.current_0, jsonGetValue(midas, vector<string>{"magnet", "current_0"}));
    takeDouble(fConstants.magnetData.current_1, jsonGetValue(midas, vector<string>{"magnet", "current_1"}));

    takeDouble(fConstants.beamData.beam_shutter, jsonGetValue(midas, vector<string>{"beam", "beam_shutter"}));
    takeDouble(fConstants.beamData.proton_current, jsonGetValue(midas, vector<string>{"beam", "proton_current"}));
    string fname = jsonGetString(midas, vector<string>{"beam", "filename"});
    if (!fname.empty() && fname != "parseError") fConstants.beamData.filename = fname;
  };

  auto parseOdb = [&](const string& odb) {
    if (odb.empty()) return;
    vector<string> names = jsonStringArray(odb, "Names DEFF");
    vector<double> values = jsonDoubleArray(odb, "DEFF");
    int n = static_cast<int>(min(names.size(), values.size()));
    fConstants.daqData.n_deff = n;
    fConstants.daqData.deff_names.assign(names.begin(), names.begin() + n);
    fConstants.daqData.deff_values.assign(values.begin(), values.begin() + n);

    vector<string> lvdeNames = jsonStringArray(odb, "Names LVDE");
    vector<double> lvdeValues = jsonDoubleArray(odb, "LVDE");
    int nl = static_cast<int>(min(lvdeNames.size(), lvdeValues.size()));
    fConstants.daqData.n_link = nl;
    fConstants.daqData.lvde_names.assign(lvdeNames.begin(), lvdeNames.begin() + nl);
    fConstants.daqData.lvde_values.assign(lvdeValues.begin(), lvdeValues.begin() + nl);

    vector<string> tdac = jsonStringArray(odb, "TDACFILE");
    if (tdac.empty()) return;
    string dir = directoryName(tdac.front());
    bool same = true;
    for (size_t i = 1; i < tdac.size(); ++i) {
      if (directoryName(tdac[i]) != dir) {
        cout << "########### calEventStuffV3::readJSON> Error, TDACFILE directories do not match: " 
        << endl        
        << directoryName(tdac[i]) << endl 
        << dir << endl;
        same = false;
        break;
      }
    }
    fConstants.pixelData.maskdirname = dir;
    fConstants.pixelData.maskdirname_ok = same;
  };

  if (endsWith(filename, kMidasSuffix)) {
    parseMidas(text);
    string odbName = filename.substr(0, filename.size() - kMidasSuffix.size()) + ".json";
    parseOdb(readTextFile(odbName));
  } else if (text.find("\"Names DEFF\"") != string::npos) {
    parseOdb(text);
    if (endsWith(filename, ".json")) {
      string midasName = filename.substr(0, filename.size() - 5) + kMidasSuffix;
      parseMidas(readTextFile(midasName));
    }
  } else {
    takeU64(fConstants.eventData.startFrameData, jsonGetValue(text, vector<string>{"eventdata", "startframedata"}));
    takeU64(fConstants.eventData.endFrameData, jsonGetValue(text, vector<string>{"eventdata", "endframedata"}));
    takeU64(fConstants.eventData.firstFrameWithFEBProblems, jsonGetValue(text, vector<string>{"eventdata", "firstframewithfebproblems"}));
    takeU64(fConstants.pixelData.startFrameGoodData, jsonGetValue(text, vector<string>{"pixeldata", "startframegooddata"}));
    takeU64(fConstants.pixelData.endFrameGoodData, jsonGetValue(text, vector<string>{"pixeldata", "endframegooddata"}));
    takeU64(fConstants.pixelData.firstFrameWithFEBUnsortedHitData, jsonGetValue(text, vector<string>{"pixeldata", "firstframewithfebunsortedhitdata"}));
    takeU64(fConstants.tileData.startFrameGoodData, jsonGetValue(text, vector<string>{"tiledata", "startframegooddata"}));
    takeU64(fConstants.tileData.endFrameGoodData, jsonGetValue(text, vector<string>{"tiledata", "endframegooddata"}));
    takeU64(fConstants.tileData.firstFrameWithFEBUnsortedHitData, jsonGetValue(text, vector<string>{"tiledata", "firstframewithfebunsortedhitdata"}));
    takeU64(fConstants.fibreData.startFrameGoodData, jsonGetValue(text, vector<string>{"fibredata", "startframegooddata"}));
    takeU64(fConstants.fibreData.endFrameGoodData, jsonGetValue(text, vector<string>{"fibredata", "endframegooddata"}));
    takeU64(fConstants.fibreData.firstFrameWithFEBUnsortedHitData, jsonGetValue(text, vector<string>{"fibredata", "firstframewithfebunsortedhitdata"}));
  }
  return text;
}
