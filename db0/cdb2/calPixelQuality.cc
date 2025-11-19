#include "calPixelQuality.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <sstream>


using namespace std;

// ----------------------------------------------------------------------
calPixelQuality::calPixelQuality(cdbAbs *db) : calAbs(db) {
}


// ----------------------------------------------------------------------
bool calPixelQuality::getNextID(uint32_t &ID) {
  if (fMapConstantsIt == fMapConstants.end()) {
    // -- reset
    ID = 999999;
    fMapConstantsIt = fMapConstants.begin();
    return false;
  } else {
    ID = fMapConstantsIt->first;
    fMapConstantsIt++;
  }
  return true;
}


// ----------------------------------------------------------------------
calPixelQuality::calPixelQuality(cdbAbs *db, string tag) : calAbs(db, tag) {
  cout << "calPixelQuality created and registered with tag ->" << fTag << "<-"
       << endl;
}


// ----------------------------------------------------------------------
calPixelQuality::~calPixelQuality() {
  fMapConstants.clear();
  cout << "this is the end of calPixelQuality with tag ->" << fTag << "<-" << endl;
}


// ----------------------------------------------------------------------
void calPixelQuality::calculate(string hash) {
  cout << "calPixelQuality::calculate() with "
       << "fHash ->" << hash << "<-"
       << endl;
  fMapConstants.clear();
  string spl = fTagIOVPayloadMap[hash].fBLOB;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  cout << "calPixelQuality header: " << hex << header << dec << endl;

  int npix(0);
  while (ibuffer != buffer.end()) {
    constants a;
    // -- chipID
    a.id = blob2UnsignedInt(getData(ibuffer));
    // -- get number of pixel entries
    npix = blob2Int(getData(ibuffer));
    // -- initialize matrix with zero before filling specified pixels
    for (int ix = 0; ix < 256; ++ix) {
      for (int iy = 0; iy < 250; ++iy) {
        a.matrix[ix][iy] = 0;
      }
    }
    for (int i = 0; i < npix; ++i) {
      int icol           = blob2Int(getData(ibuffer));
      int irow           = blob2Int(getData(ibuffer));
      unsigned int iqual = blob2UnsignedInt(getData(ibuffer));
      a.matrix[icol][irow] = static_cast<char>(iqual);
    }
    // cout << "inserting " << a.id << " with size = " << sizeof(a) << endl;
    fMapConstants.insert(make_pair(a.id, a));
  }

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}


// ----------------------------------------------------------------------
calPixelQuality::Status calPixelQuality::getStatus(unsigned int chipid, int icol, int irow) {
  if (fMapConstants.find(chipid) == fMapConstants.end()) {
    return Status::ChipNotFound; // -- chip not found
  }
  return static_cast<Status>(fMapConstants[chipid].matrix[icol][irow]);
}


// ----------------------------------------------------------------------
void calPixelQuality::printPixelQuality(unsigned int chipid, int minimumStatus) {
  for (int ix = 0; ix < 256; ++ix) {
    for (int iy = 0; iy < 250; ++iy) {
      if (fMapConstants[chipid].matrix[ix][iy] > minimumStatus) {
        cout << "chipID = " << chipid
             << " x/y = " << ix << "/" << iy
             << " status = " << static_cast<unsigned int>(fMapConstants[chipid].matrix[ix][iy])
             << endl;
      }
    }
  }
}


// ----------------------------------------------------------------------
void calPixelQuality::printBLOB(std::string sblob, int verbosity) {

  std::vector<char> buffer(sblob.begin(), sblob.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  cout << "calPixelQuality::printBLOB(string," << verbosity << ")" << endl;
  cout << "   header: " << hex << header << dec << endl;

  string summary("calPixelQuality chips ");
  int nnp(0);
  while (ibuffer != buffer.end()) {
    // -- chipID
    unsigned int chipID = blob2UnsignedInt(getData(ibuffer));
    // -- get number of pixel entries
    int npix = blob2Int(getData(ibuffer));
    if (0 == verbosity) {
      summary += (to_string(chipID) + " ");
      nnp += npix;
    } else {
      cout << "   chipID: " << chipID << " npix: " << npix << endl;
    }
    for (int i = 0; i < npix; ++i) {
      int icol           = blob2Int(getData(ibuffer));
      int irow           = blob2Int(getData(ibuffer));
      unsigned int iqual = blob2UnsignedInt(getData(ibuffer));
      if (verbosity > 0) cout << "      icol/irow = "
                                << icol << "/" << irow
                                << " iqual = " << iqual
                                << endl;
    }
  }
  if (0 == verbosity) {
    cout << summary << " with " << nnp << " noisy pixels" << endl;
  }
}


// ----------------------------------------------------------------------
string calPixelQuality::makeBLOB() {
  stringstream s;
  unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  // -- format of m
  // chipID => [npix, n*(col, row, iqual)]
  for (auto it: fMapConstants) {
    s << dumpArray(uint2Blob(it.first));
    constants a = it.second;

    int npix(0);
    for (int ic = 0; ic < 256; ++ic) {
      for (int ir = 0; ir < 250; ++ir) {
        if (a.matrix[ic][ir] > 0) ++npix;
      }
    }

    s << dumpArray(int2Blob(npix));

    for (int ic = 0; ic < 256; ++ic) {
      for (int ir = 0; ir < 250; ++ir) {
        if (a.matrix[ic][ir] > 0) {
          s << dumpArray(int2Blob(ic));
          s << dumpArray(int2Blob(ir));
          s << dumpArray(int2Blob(a.matrix[ic][ir]));
        }
      }
    }

  }
  return s.str();
}


// ----------------------------------------------------------------------
string calPixelQuality::makeBLOB(const map<unsigned int, vector<double>>& m) {
  stringstream s;
  unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  // -- format of m
  // chipID => [npix, n*(col, row, iqual)]
  for (auto it: m) {
    s << dumpArray(uint2Blob(it.first));
    int npix = it.second.size()/3;
    s << dumpArray(int2Blob(npix));
    for (int ipix = 0; ipix < npix; ++ipix) {
      int idx   = ipix*3;
      int icol  = static_cast<int>(it.second[idx]);
      idx       = ipix*3 + 1;
      int irow  = static_cast<int>(it.second[idx]);
      idx       = ipix*3 + 2;
      int iqual = static_cast<int>(it.second[idx]);

      s << dumpArray(int2Blob(icol));
      s << dumpArray(int2Blob(irow));
      s << dumpArray(int2Blob(iqual));
    }
  }
  return s.str();
}


// ----------------------------------------------------------------------
void calPixelQuality::writeCsv(string filename) {
  string spl("");
  ofstream OUT(filename);
  if (!OUT.is_open()) {
    cout << ("calPixelQuality::writeCsv> Error, file "
             + filename + " not opened for output")
         << endl;
  }

  OUT << "# Format: chipID,icol1,irow1,iqual1,icol2,irow2,iqual2,..."
      << endl;

  for (auto it: fMapConstants) {
    stringstream s, spixel;
    s << it.first;
    int cnt(0);
    for (int icol = 0; icol < 256; ++icol) {
      for (int irow = 0; irow < 250; ++irow) {
        if (it.second.matrix[icol][irow] != 0) {
          if (cnt > 0) spixel << ",";
          spixel << icol << "," << irow << ","
                 << static_cast<int>(it.second.matrix[icol][irow]);
          ++cnt;
        }
      }
    }
    OUT << s.str();
    if (cnt > 0) OUT << "," << spixel.str();
    OUT << endl;
  }
  OUT.close();
}


// ----------------------------------------------------------------------
void calPixelQuality::readCsv(string filename) {
  cout << "calPixelQuality::readCsv> reset fMapConstants" << endl;
  fMapConstants.clear();

  string spl("");
  ifstream INS(filename);
  if (!INS.is_open()) {
    cout << ("calPixelQuality::readCsv> Error, file "
             + filename + " not found")
         << endl;
  }

  vector<string> vline;
  while (getline(INS, spl)) {
    if (string::npos == spl.find("#")) {
      vline.push_back(spl);
    }
  }
  INS.close();

  for (unsigned int it = 0; it < vline.size(); ++it) {
    constants a;
    vector<string> tokens = split(vline[it], ',');
    // -- chipID
    a.id = stoi(tokens[0]);
    // -- initialize
    for (int ix = 0; ix < 256; ++ix) {
      for (int iy = 0; iy < 250; ++iy) {
        a.matrix[ix][iy] = 0;
      }
    }
    for (unsigned ipix = 1; ipix < tokens.size(); ipix += 3) {
      int icol           = stoi(tokens[ipix]);
      int irow           = stoi(tokens[ipix+1]);
      unsigned int iqual = stoi(tokens[ipix+2]);
      a.matrix[icol][irow] = static_cast<char>(iqual);
    }
    fMapConstants.insert(make_pair(a.id, a));
  }

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}
