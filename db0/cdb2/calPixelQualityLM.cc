#include "calPixelQualityM.hh"

#include "cdbUtil.hh"

#include <iostream>
#include <sstream>
#include "calPixelQualityLM.hh"


using namespace std;

// ----------------------------------------------------------------------
calPixelQualityLM::calPixelQualityLM(cdbAbs *db) : calAbs(db) {
}


// ----------------------------------------------------------------------
bool calPixelQualityLM::getNextID(uint32_t &ID) {
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
calPixelQualityLM::calPixelQualityLM(cdbAbs *db, string tag) : calAbs(db, tag) {
  cout << "calPixelQualityLM created and registered with tag ->" << fTag << "<-"
       << endl;
}


// ----------------------------------------------------------------------
calPixelQualityLM::~calPixelQualityLM() {
  for (auto it: fMapConstants) it.second.mpixel.clear();
  fMapConstants.clear();
  cout << "this is the end of calPixelQualityLM with tag ->" << fTag << "<-" << endl;
}


// ----------------------------------------------------------------------
void calPixelQualityLM::calculate(string hash) {
  cout << "calPixelQualityLM::calculate() with "
       << "fHash ->" << hash << "<-"
       << endl;
  fMapConstants.clear();
  string spl = fTagIOVPayloadMap[hash].fBLOB;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  cout << "calPixelQualityLM header: " << hex << header << dec << endl;

  int npix(0), ncol(0);
  while (ibuffer != buffer.end()) {
    constants a;
    a.id = blob2UnsignedInt(getData(ibuffer));
  
    a.linkA = blob2UnsignedInt(getData(ibuffer));
    a.linkB = blob2UnsignedInt(getData(ibuffer));
    a.linkC = blob2UnsignedInt(getData(ibuffer));
    a.linkM = blob2UnsignedInt(getData(ibuffer));

    // -- get number of column entries
    ncol = blob2Int(getData(ibuffer));
    for (int i = 0; i < npix; ++i) {
      int icol            = blob2Int(getData(ibuffer));
      unsigned int iqual = blob2UnsignedInt(getData(ibuffer));
      a.mcol.insert({ icol, static_cast<char>(iqual) });
    }

    // -- get number of pixel entries
    npix = blob2Int(getData(ibuffer));
    for (int i = 0; i < npix; ++i) {
      int icol            = blob2Int(getData(ibuffer));
      int irow            = blob2Int(getData(ibuffer));
      unsigned int iqual = blob2UnsignedInt(getData(ibuffer));
      int idx = icol*250 + irow;
      a.mpixel.insert({ idx, static_cast<char>(iqual) });
    }
    // cout << "inserting " << a.id << " with size = " << sizeof(a) << endl;
    fMapConstants.insert(make_pair(a.id, a));
  }

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}


// ----------------------------------------------------------------------
int calPixelQualityLM::getStatus(unsigned int chipid, int icol, int irow) {
  // -- first check dead links  
  if (fMapConstants[chipid].linkA > 0 && icol < 89) {
    return static_cast<int>(fMapConstants[chipid].linkA);
  } 
  if (fMapConstants[chipid].linkB > 0 && icol >= 89 && icol < 173) {
    return static_cast<int>(fMapConstants[chipid].linkB);
  }
  if (fMapConstants[chipid].linkC > 0 && icol >= 173) {
    return static_cast<int>(fMapConstants[chipid].linkC);
  }
  // -- now check dead columns
  if (fMapConstants[chipid].mcol.find(icol) != fMapConstants[chipid].mcol.end()) {
    return static_cast<int>(fMapConstants[chipid].mcol[icol]);
  } 

  // -- finally check dead pixels
  if (fMapConstants[chipid].mpixel.find(icol*250+irow) == fMapConstants[chipid].mpixel.end()) {
    return 0;
  } else {
    return static_cast<int>(fMapConstants[chipid].mpixel[icol*250+irow]);
  }
}


// ----------------------------------------------------------------------
void calPixelQualityLM::printPixelQuality(unsigned int chipid, int minimumStatus) {
  // FIXME
}


// ----------------------------------------------------------------------
void calPixelQualityLM::printBLOB(std::string sblob, int verbosity) {

  std::vector<char> buffer(sblob.begin(), sblob.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  cout << "calPixelQuality::printBLOB(string)" << endl;
  cout << "   header: " << hex << header << dec << endl;

  while (ibuffer != buffer.end()) {
    // -- chipID
    unsigned int chipID = blob2UnsignedInt(getData(ibuffer));
    // -- get link words
    unsigned int linkA = blob2UnsignedInt(getData(ibuffer));
    unsigned int linkB = blob2UnsignedInt(getData(ibuffer));
    unsigned int linkC = blob2UnsignedInt(getData(ibuffer));
    unsigned int linkM = blob2UnsignedInt(getData(ibuffer));
    cout << "   chipID: " << chipID << endl;
    cout << "     linkA: " << linkA
         << " linkB: " << linkB << " linkC: " << linkC
         << " linkM: " << linkM << endl;
    // -- get number of column entries
    int ncol = blob2Int(getData(ibuffer));
    cout << "   ncol: " << ncol << endl;
    for (int i = 0; i < ncol; ++i) {
      int icol           = blob2Int(getData(ibuffer));
      cout << "      icol = " << icol << endl;
    }
    // -- get number of pixel entries
    int npix = blob2Int(getData(ibuffer));
    cout << "   npix: " << npix << endl;
    for (int i = 0; i < npix; ++i) {
      int icol           = blob2Int(getData(ibuffer));
      int irow           = blob2Int(getData(ibuffer));
      unsigned int iqual = blob2UnsignedInt(getData(ibuffer));
      cout << "      icol/irow = " << icol << "/" << irow << " iqual = " << iqual << endl;
    }
  }
}


// ----------------------------------------------------------------------
map<unsigned int, vector<double> > calPixelQualityLM::decodeBLOB(string spl) {
  map<unsigned int, vector<double> > vmap;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  if (0xdeadface != header) {
    cout << "XXXXX ERRROR in calPixelQuality::decodeBLOB> header is wrong. Something is really messed up!" << endl;
  }
  while (ibuffer != buffer.end()) {
    // -- chipID
    unsigned int chipID = blob2UnsignedInt(getData(ibuffer));
    // -- get link words
    unsigned int linkA = blob2UnsignedInt(getData(ibuffer));
    unsigned int linkB = blob2UnsignedInt(getData(ibuffer));
    unsigned int linkC = blob2UnsignedInt(getData(ibuffer));
    unsigned int linkM = blob2UnsignedInt(getData(ibuffer));
    vector<double> vdet;
    // -- get number of pixel entries
    int npix = blob2Int(getData(ibuffer));
    for (int i = 0; i < npix; ++i) {
      int icol           = blob2Int(getData(ibuffer));
      int irow           = blob2Int(getData(ibuffer));
      unsigned int iqual = blob2UnsignedInt(getData(ibuffer));
      vdet.push_back(static_cast<double>(icol));
      vdet.push_back(static_cast<double>(irow));
      vdet.push_back(static_cast<double>(iqual));
    }
    vmap.insert(make_pair(chipID, vdet));
  }

  return vmap;
}

// ----------------------------------------------------------------------
string calPixelQualityLM::makeBLOB() {
  stringstream s;
  unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  // -- format of m
  // chipID => [npix, n*(col, row, iqual)]
  for (auto it: fMapConstants) {
    s << dumpArray(uint2Blob(it.first));
    constants a = it.second;

    s << dumpArray(uint2Blob(a.linkA)); // -- linkA
    s << dumpArray(uint2Blob(a.linkB)); // -- linkB
    s << dumpArray(uint2Blob(a.linkC)); // -- linkC
    s << dumpArray(uint2Blob(a.linkM)); // -- linkM

    // -- get number of column entries
    int ncol = a.mcol.size();
    s << dumpArray(int2Blob(ncol));
    for (auto it: a.mcol) {
      int icol = it.first;
      s << dumpArray(int2Blob(icol));
    }
    
    // -- get number of pixel entries
    int npix = a.mpixel.size();
    s << dumpArray(int2Blob(npix));
    for (auto it: a.mpixel) {
      int idx = it.first;
      int icol = idx/250;
      int irow = idx%250;
      unsigned int iqual = static_cast<unsigned int>(it.second);
      s << dumpArray(int2Blob(icol));
      s << dumpArray(int2Blob(irow));
      s << dumpArray(uint2Blob(iqual));
    }

  }
  return s.str();

}

// ----------------------------------------------------------------------
string calPixelQualityLM::makeBLOB(const map<unsigned int, vector<double>>& m) {
  stringstream s;
  unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  // -- format of m
  // chipID => linkA, linkB, linkC, linkM, [npix, n*(col, row, iqual)]
  for (auto it: m) {
    s << dumpArray(uint2Blob(it.first));
    // -- link words (dummy for now)
    s << dumpArray(uint2Blob(0));
    s << dumpArray(uint2Blob(0));
    s << dumpArray(uint2Blob(0));
    s << dumpArray(uint2Blob(0));
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
void calPixelQualityLM::readCsv(string filename) {
  cout << "calPixelQualityLM::readCsv> reset fMapConstants" << endl;
  fMapConstants.clear();

  string spl("");
  ifstream INS(filename);
  if (!INS.is_open()) {
    cout << ("calPixelQualityLM::readCsv> Error, file "
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
    a.linkA = stoi(tokens[1]);
    a.linkB = stoi(tokens[2]);
    a.linkC = stoi(tokens[3]);
    a.linkM = stoi(tokens[4]);
    // -- initialize column map
    int ncol = stoi(tokens[5]);
    a.mcol.clear();
    for (unsigned ipix = 6; ipix < tokens.size(); ++ipix) {
      int icol           = stoi(tokens[ipix]);
      a.mcol[icol] = static_cast<char>(1);
    }
    fMapConstants.insert(make_pair(a.id, a));
  }

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}
