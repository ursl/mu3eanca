#include "calPixelMask.hh"

#include "cdbUtil.hh"

#include <glob.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

// ----------------------------------------------------------------------
calPixelMask::calPixelMask(cdbAbs *db) : calAbs(db) {
}

// ----------------------------------------------------------------------
calPixelMask::calPixelMask(cdbAbs *db, string tag) : calAbs(db, tag) {
  if (fVerbose) cout << "calPixelMask created and registered with tag ->"
                     << fTag << "<-"
                     << endl;
}

// ----------------------------------------------------------------------
calPixelMask::~calPixelMask() {
  if (fVerbose > 0) cout << "this is the end of calPixelMask with tag ->" << fTag << "<-" << endl;
}

// ----------------------------------------------------------------------
void calPixelMask::calculate(string hash) {
  fMapConstants.clear();
  string spl = fTagIOVPayloadMap[hash].fBLOB;
  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();
  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  if (fVerbose > 0) cout << " header: " << hex << header << dec;

  while (ibuffer != buffer.end()) {
    constants a;
    a.id = blob2UnsignedInt(getData(ibuffer));
    for (int i = 0; i < 256*250/8; ++i) {
      std::array<char,8> v = getData(ibuffer);
      for (int j = 0; j < 8; ++j) {
        a.mask[i*8 + j] = v[j];
      }
    }
    fMapConstants.insert(make_pair(a.id, a));
  }
  if (fVerbose > 0) cout << " inserted " << fMapConstants.size() << " constants" << endl;

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}

// ----------------------------------------------------------------------
bool calPixelMask::getNextID(uint32_t &ID) {
  if (fMapConstants.size() < 1) {
    cout << "calPixelMask::getNextID> ERROR: no constants loaded." << endl;
  }
  if (fMapConstantsIt == fMapConstants.end()) {
    ID = 999999;
    fMapConstantsIt = fMapConstants.begin();
    return false;
  }
  ID = fMapConstantsIt->first;
  fMapConstantsIt++;
  return true;
}

// ----------------------------------------------------------------------
string calPixelMask::makeBLOB() {
  stringstream s;
  unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  // -- format of m
  // chipID => [mask]
  for (auto it: fMapConstants) {
    s << dumpArray(uint2Blob(it.first));
    constants a = it.second;

    for (int i = 0; i < 256*250; i += 8) {
      std::array<char,8> v;
      for (int j = 0; j < 8; ++j) {
        v[j] = a.mask[i + j];
      }
      s << dumpArray(v);
    }
  }
  return s.str();
}


// ----------------------------------------------------------------------
void calPixelMask::printBLOB(string s, int verbosity) {
  cout << printBLOBString(s, verbosity) << endl;
}

// ----------------------------------------------------------------------
string calPixelMask::makeBLOB(const std::map<unsigned int, std::vector<double>>&) {
  return "nada";
}

// ----------------------------------------------------------------------
string calPixelMask::printBLOBString(string sblob, int verbosity) {

  vector<char> buffer(sblob.begin(), sblob.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  stringstream ss;
  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  ss << "calPixelMask::printBLOB(string," << verbosity << ")" << endl;
  ss << "   header: " << hex << header << dec << endl;
  if (0xdeadface != header) {
    ss << "XXXXX ERRROR in calPixelMask::printBLOB> header is wrong. Something is really messed up!" << endl;
  }

  while (ibuffer != buffer.end()) {
    unsigned int chipid = blob2UnsignedInt(getData(ibuffer));
    char mask[256*250];
    int nMasked(0);
    for (int i = 0; i < 256*250/8; ++i) {
      std::array<char,8> v = getData(ibuffer);
      for (int j = 0; j < 8; ++j) {
        if (v[j] == Masked::Masked) {
          mask[i*8 + j] = 9;
          ++nMasked;
        } else {
          mask[i*8 + j] = 0;
        }
      }
    }
    ss << "==> chipId: " << chipid ;
    if (nMasked > 0) ss << " begin " << endl;
    for (int i = 0; i < 256; ++i) {
      for (int j = 0; j < 250; ++j) {
        if (mask[i*250 + j] == Masked::Masked) {
          ss << i << "/" << j << ":9,";
        } 
      }
      if (nMasked > 0) ss << endl;
    }
   if (nMasked > 0) ss << "--> chipId: " << chipid;
   ss << " nMasked: " << nMasked << endl;
  }
  return ss.str();
}


// ----------------------------------------------------------------------
void calPixelMask::readAllMaskBinaryFiles(string directory) {
  vector<string> vFiles;
  glob_t globbuf;
  string lPattern = directory + "/*mask_chip_*.bin";
  cout << "globbing " << lPattern << endl;
  if (lPattern.find("*") == string::npos) {
    lPattern = lPattern + "*";
  }
  if (::glob(lPattern.c_str(), GLOB_TILDE, NULL, &globbuf) == 0) {
    for (size_t i = 0; i < globbuf.gl_pathc; i++) {
      if (string(globbuf.gl_pathv[i]).find("mask_chip_blank.bin") != string::npos) {
        continue;
      }
      vFiles.push_back(string(globbuf.gl_pathv[i]));
    }
  }
  globfree(&globbuf);

  cout << "found " << vFiles.size() << " mask binary files" << endl;
  for (const auto & entry : vFiles) {
    cout << "entry: " << entry << endl;
    readMaskBinaryFile(entry);
  }
}

// ----------------------------------------------------------------------
void calPixelMask::readMaskBinaryFile(string filename) {
  cout << "reading file ->" << filename << "<-" << endl;
  unsigned int chipid(0);
  size_t pos = filename.find("mask_chip_");
  if (pos != std::string::npos) {
    std::sscanf(filename.c_str() + pos, "mask_chip_%d.bin", &chipid);
    cout << "chipid: " << chipid << endl;
  } else {
    cout << "Error: chipid not found in filename ->" << filename << "<-" << endl;
    return;
  }

  std::vector<uint32_t> vec(256 * 64);
  ifstream file(filename, std::ios::binary);
  file.read(reinterpret_cast<char*>(&vec[0]), 256 * 64 * sizeof(uint32_t));
  file.close();

  constexpr int kCols = 256;
  constexpr int kMaskPerCol = 250;  // hardware: col0 row0..row249, col1 row0.., etc.
  const uint32_t mask = 0x07070707;
  char cmask[kCols * kMaskPerCol];
  // -- Each 0x47 byte: high nibble = mask state (4 => unmasked), low nibble 0x7 = filler (XOR mask).
  //    One cmask entry per byte (high nibble only), not per nibble — avoids 7,4,7,4 → bogus 0,1,0,1.
  auto packHi = [](unsigned byte) -> char {
    return static_cast<char>((((byte >> 4) & 0xF) >> 2));
  };
  for (int col = 0; col < kCols; ++col) {
    const int base = col * kMaskPerCol;
    for (int row = 0; row < 62; ++row) {
      uint32_t val = vec[col * 64 + row];
      vec[col * 64 + row] = val ^ mask;
      for (int b = 0; b < 4; ++b) {
        unsigned byte = (val >> (8 * b)) & 0xFFu;
        cmask[base + 4 * row + b] = packHi(byte);
      }
    }
    uint32_t val2 = vec[col * 64 + 62];
    vec[col * 64 + 62] = val2 ^ 0x0707;
    // -- Row 62: low 16 bits are the two 0x47-like bytes; high 16 bits are 0xdada filler (LE layout).
    unsigned b0 = val2 & 0xFFu;
    unsigned b1 = (val2 >> 8) & 0xFFu;
    cmask[base + 248] = packHi(b0);
    cmask[base + 249] = packHi(b1);
    vec[col * 64 + 63] = 0xda00da00 | ((col & 0xff) << 16);
  }

  constants cc;
  cc.id = chipid;
  for (int i = 0; i < 256*250; ++i) {
    if (cmask[i] == 0) {
      cc.mask[i] = Masked::Masked;
    } else {
      cc.mask[i] = Masked::Unmasked;
    }
  }
  fMapConstants.insert(make_pair(chipid, cc));

  for (int col = 0; col < 256; ++col) {
    for (int row = 0; row < 64; ++row) {
      //cout << "col: " << setw(3) << dec << col << " row: " << setw(3) << dec << row 
      //<< " vec: " << setw(8) << hex << vec[col*64 + row] 
      //<< endl;
    }
  }

  int nMasked(0);
  for (int col = 0; col < kCols; ++col) {
    for (int row = 0; row < kMaskPerCol; ++row) {
      //cout << "col: " << setw(3) << dec << col << " row: " << setw(3) << dec << row
      //     << " cmask: " << setw(2) << dec
      //     << static_cast<int>(static_cast<unsigned char>(cmask[col * kMaskPerCol + row]))
      //     << " api: " << getMasked(chipid, col, row)
      //     << endl;
      if (static_cast<int>(static_cast<unsigned char>(cmask[col * kMaskPerCol + row])) == 0) {
        ++nMasked;
      }
    }
  }
  cout << "nMasked: " << nMasked << endl;
}

// ----------------------------------------------------------------------
enum Masked calPixelMask::getMasked(unsigned int chipid, int icol, int irow) {
  if (fMapConstants.find(chipid) == fMapConstants.end()) {
    return Masked::Unknown;
  }
  return static_cast<enum Masked>(fMapConstants[chipid].mask[icol * 250 + irow]);
}