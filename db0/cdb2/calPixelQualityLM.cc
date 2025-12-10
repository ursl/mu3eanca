#include "cdbUtil.hh"

#include <iostream>
#include <sstream>
#include "calPixelQualityLM.hh"

using namespace std;

// ----------------------------------------------------------------------
calPixelQualityLM::calPixelQualityLM(cdbAbs *db) : calAbs(db) {
}


// ----------------------------------------------------------------------
string calPixelQualityLM::statusToString(Status status) {
  // -- Update this when adding new Status values
  switch (status) {
    case ChipNotFound: return "ChipNotFound";
    case Good: return "Good";
    case Noisy: return "Noisy";
    case Suspect: return "Suspect";
    case DeclaredBad: return "DeclaredBad";
    case LVDSErrorLink: return "LVDSErrorLink";
    case LVDSErrorOtherLink: return "LVDSErrorOtherLink";
    case LVDSErrorTopBottomEdge: return "LVDSErrorTopBottomEdge";
    case DeadChip: return "DeadChip";
    case NoHits: return "NoHits";
    case Masked: return "Masked";
    default: return "Unknown";
  }
}

// ----------------------------------------------------------------------
string calPixelQualityLM::getStatusDocumentation() {
  // -- List of all Status enum values - update this when adding new Status values
  const Status allStatuses[] = {
    ChipNotFound,
    Good,
    Noisy,
    Suspect,
    DeclaredBad,
    LVDSErrorLink,
    LVDSErrorOtherLink,
    LVDSErrorTopBottomEdge,
    DeadChip,
    NoHits,
    Masked
  };
  
  stringstream ss;
  bool first = true;
  for (Status s : allStatuses) {
    if (!first) ss << ", ";
    ss << static_cast<int>(s) << "=" << statusToString(s);
    first = false;
  }
  ss << ". M-link=nhit/ovfl";
  return ss.str();
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
  if (0) cout << "calPixelQualityLM created and registered with tag ->" << fTag << "<-"
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
       << "fHash ->" << hash << "<-";
  fMapConstants.clear();
  string spl = fTagIOVPayloadMap[hash].fBLOB;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  cout << " header: " << hex << header << dec;

  int npix(0), ncol(0);
  while (ibuffer != buffer.end()) {
    constants a;
    a.id = blob2UnsignedInt(getData(ibuffer));
    a.ckdivend = blob2UnsignedInt(getData(ibuffer));
    a.ckdivend2 = blob2UnsignedInt(getData(ibuffer));

    a.linkA = blob2UnsignedInt(getData(ibuffer));
    a.linkB = blob2UnsignedInt(getData(ibuffer));
    a.linkC = blob2UnsignedInt(getData(ibuffer));
    a.linkM = blob2UnsignedInt(getData(ibuffer));

    // -- get number of column entries
    ncol = blob2Int(getData(ibuffer));
    if (ncol > 0) { 
      for (int i = 0; i < ncol; ++i) {
        int icol            = blob2Int(getData(ibuffer));
        unsigned int iqual  = blob2UnsignedInt(getData(ibuffer));
        a.mcol.insert({ icol, static_cast<char>(iqual) });
      }
    }

    // -- get number of pixel entries
    npix = blob2Int(getData(ibuffer));
    if (npix > 0) { 
      for (int i = 0; i < npix; ++i) {
        int icol            = blob2Int(getData(ibuffer));
        int irow            = blob2Int(getData(ibuffer));
        unsigned int iqual = blob2UnsignedInt(getData(ibuffer));
        int idx = icol*250 + irow;
        a.mpixel.insert({ idx, static_cast<char>(iqual) });
      }
    }
    fMapConstants.insert(make_pair(a.id, a));
  }
  cout << " inserted " << fMapConstants.size() << " constants" << endl;

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}


// ----------------------------------------------------------------------
calPixelQualityLM::Status calPixelQualityLM::getColStatus(unsigned int chipid, int icol) {
  if (fMapConstants.find(chipid) == fMapConstants.end()) {
    return Status::ChipNotFound; // -- chip not found
  }
  return static_cast<Status>(fMapConstants[chipid].mcol[icol]);
}

// ----------------------------------------------------------------------
int calPixelQualityLM::getNpixWithStatus(unsigned int chipid, Status status) {
  if (fVerbose > 0) cout << "calPixelQualityLM::getNpixWithStatus> chipid = " << chipid << " status = " << status << endl;
  if (fMapConstants.find(chipid) == fMapConstants.end()) {
    return -1; // -- chip not found
  }
  // -- n counts the defective pixels. If status == 0 (Good) was requested, then it is inverted at the end.
  int n(0);
  // -- check pixel status
  for (auto it: fMapConstants[chipid].mpixel) {
    if (fVerbose > 4) cout << "calPixelQualityLM::getNpixWithStatus> pixel = " << it.first << " status = " << static_cast<int>(it.second) << endl;
    if (0 == status) {
      // -- count all pixels with bad status
      if (it.second != 0) n++;
    } else {
      if (it.second == status) n++;
    }
  }
  // -- check column status
  for (auto it: fMapConstants[chipid].mcol) {
    if (fVerbose > 4) cout << "calPixelQualityLM::getNpixWithStatus> column = " << it.first << " status = " << static_cast<int>(it.second)  << endl;
    if (0 == status){
      // -- count all columns with bad status
      if (it.second != 0) n += 250;
    } else {
      if (it.second == status) n += 250; // -- all pixels in this column are defective
    }
  }

  // -- check link status
  if (0 == status) {
    if (fMapConstants[chipid].linkA != 0) {
      n += 250*89;
    }
    if (fMapConstants[chipid].linkB != 0) {
      n += 250*84;
    }
    if (fMapConstants[chipid].linkC != 0) {
      n += 250*83;
    }
  } else {
    if (fMapConstants[chipid].linkA == status) {
      n += 250*89;
    }
    if (fMapConstants[chipid].linkB == status) {
      n += 250*84;
    }
    if (fMapConstants[chipid].linkC == status) {
      n += 250*83;
    }
  }


  if (0 == status) {
    n = 256*250 - n;
    if (fVerbose > 0) cout << "calPixelQualityLM::getNpixWithStatus> chipID = " << chipid << " has " << n << " pixels with good status" << endl;
    //cout << "chipID = " << chipid << " has " << n << " pixels with good status" << endl;
  } else {
    //cout << "chipID = " << chipid << " has " << n << " pixels with status " << status << endl;
  }

  return n;
}

// ----------------------------------------------------------------------
calPixelQualityLM::Status calPixelQualityLM::getStatus(unsigned int chipid, int icol, int irow) {
  // -- first check dead links  
  if (fMapConstants[chipid].linkA > 0 && icol >= 0 && icol <= 87) {
    return static_cast<Status>(fMapConstants[chipid].linkA);
  } 
  if (fMapConstants[chipid].linkB > 0 && icol >= 88 && icol <= 171) {
    return static_cast<Status>(fMapConstants[chipid].linkB);
  }
  if (fMapConstants[chipid].linkC > 0 && icol >= 172 && icol <= 255) {
    return static_cast<Status>(fMapConstants[chipid].linkC);
  }
  // -- now check  column status
  if (fMapConstants[chipid].mcol.find(icol) != fMapConstants[chipid].mcol.end()) {
    return static_cast<Status>(fMapConstants[chipid].mcol[icol]);
  } 

  // -- finally check pixel status
  if (fMapConstants[chipid].mpixel.find(icol*250+irow) == fMapConstants[chipid].mpixel.end()) {
    return Status::Good;
  } else {
    return static_cast<Status>(fMapConstants[chipid].mpixel[icol*250+irow]);
  }
}

// ----------------------------------------------------------------------
int calPixelQualityLM::getCkdivend(unsigned int chipid) {
  if (fMapConstants.find(chipid) == fMapConstants.end()) {
    return -1; // -- chip not found
  }
  return fMapConstants[chipid].ckdivend;
}

// ----------------------------------------------------------------------
int calPixelQualityLM::getCkdivend2(unsigned int chipid) {
  if (fMapConstants.find(chipid) == fMapConstants.end()) {
    return -1; // -- chip not found
  }
  return fMapConstants[chipid].ckdivend2;
}


// ----------------------------------------------------------------------
bool calPixelQualityLM::isLinkBad(unsigned int chipid, int ilink) {
  if (fMapConstants.find(chipid) == fMapConstants.end()) {
    return true; // -- chip not found
  }
  bool result(false);
  switch (ilink) {
    case 0:
      if (fMapConstants[chipid].linkA > 0) {
        result = true;
        break;
      }
      break;
    case 1:
      if (fMapConstants[chipid].linkB > 0) {
        result = true;
        break;
      }
      break;
    case 2:
      if (fMapConstants[chipid].linkC > 0) {
        result = true;
        break;
      }
      break;
    default:
      result = true; // -- should not happen
      break;
  }
  return result;
}

// ----------------------------------------------------------------------
bool calPixelQualityLM::isLinkDead(unsigned int chipid, int ilink) {
  if (fMapConstants.find(chipid) == fMapConstants.end()) {
    return true; // -- chip not found
  }
  switch (ilink) {
    case 0:
      if (fMapConstants[chipid].linkA == 8 || fMapConstants[chipid].linkA == 9) {
        return true;
      }
      return false;
    case 1:
      if (fMapConstants[chipid].linkB == 8 || fMapConstants[chipid].linkB == 9) {
        return true;
      }
      return false;
    case 2:
      if (fMapConstants[chipid].linkC == 8 || fMapConstants[chipid].linkC == 9) {
        return true;
      }
      return false;
    default:
      return true; // -- should not happen
  }
}

// ----------------------------------------------------------------------
bool calPixelQualityLM::isChipDead(unsigned int chipid, int row, int col) {
  if (fMapConstants.find(chipid) == fMapConstants.end()) {
    return true; // -- chip not found
  }
  int cntDeadLinks(0); 
  for (int ilink = 0; ilink < 3; ++ilink) { 
    if (isLinkDead(chipid, ilink)) {
      cntDeadLinks++;
    }
  }
  return cntDeadLinks == 3;
}


// ----------------------------------------------------------------------
void calPixelQualityLM::printPixelQuality(unsigned int chipid, int minimumStatus) {
  // FIXME
}


// ----------------------------------------------------------------------
void calPixelQualityLM::printBLOB(std::string sblob, int verbosity) {
  cout << printBLOBString(sblob, verbosity) << endl;
}

// ----------------------------------------------------------------------
string calPixelQualityLM::printBLOBString(std::string sblob, int verbosity) {
  stringstream ss;

  std::vector<char> buffer(sblob.begin(), sblob.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  ss << "calPixelQuality::printBLOB(string)" << endl;
  ss << "   header: " << hex << header << dec << endl;

  while (ibuffer != buffer.end()) {
    // -- chipID
    unsigned int chipID = blob2UnsignedInt(getData(ibuffer));
    // -- ckdivend and ckdivend2
    unsigned int ckdivend = blob2UnsignedInt(getData(ibuffer));
    unsigned int ckdivend2 = blob2UnsignedInt(getData(ibuffer));
    // -- get link words
    unsigned int linkA = blob2UnsignedInt(getData(ibuffer));
    unsigned int linkB = blob2UnsignedInt(getData(ibuffer));
    unsigned int linkC = blob2UnsignedInt(getData(ibuffer));
    unsigned int linkM = blob2UnsignedInt(getData(ibuffer));
    ss << "   chipID: " << chipID << " ckdivend/ckdivend2: " << ckdivend << "/" << ckdivend2;
    ss << " link status A/B/C/M: " << linkA  << "/" << linkB << "/" << linkC  << "/" << linkM << endl;
    // -- get number of column entries
    int ncol = blob2Int(getData(ibuffer));
    if (ncol > 0) { 
      ss << "            column status " << ncol << " (col/qual): ";
      for (int i = 0; i < ncol; ++i) {
        int icol           = blob2Int(getData(ibuffer));
        unsigned int iqual = blob2UnsignedInt(getData(ibuffer));
        ss << icol << "/" << iqual << (i < ncol-1? ", ":"");
      }
      ss << endl;
    }
    // -- get number of pixel entries
    int npix = blob2Int(getData(ibuffer));
    if (npix > 0) { 
      ss << "            defective pixels " << npix << " (col/row/qual): " ;
      for (int i = 0; i < npix; ++i) {
        int icol           = blob2Int(getData(ibuffer));
        int irow           = blob2Int(getData(ibuffer));
        unsigned int iqual = blob2UnsignedInt(getData(ibuffer));
        ss << icol << "/" << irow << "/" << iqual << (i < npix-1? ", ":"");
      }
      ss << endl;
    }
  }
  return ss.str();
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

    s << dumpArray(uint2Blob(a.ckdivend)); // -- ckdivend
    s << dumpArray(uint2Blob(a.ckdivend2)); // -- ckdivend2

    s << dumpArray(uint2Blob(a.linkA)); // -- linkA
    s << dumpArray(uint2Blob(a.linkB)); // -- linkB
    s << dumpArray(uint2Blob(a.linkC)); // -- linkC
    s << dumpArray(uint2Blob(a.linkM)); // -- linkM

    // -- get number of column entries
    int ncol = a.mcol.size();
    s << dumpArray(int2Blob(ncol));
    for (auto it: a.mcol) {
      int icol = it.first;
      unsigned int iqual = static_cast<unsigned int>(it.second);
      s << dumpArray(int2Blob(icol));
      s << dumpArray(uint2Blob(iqual));
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
  // chipID => ckdivend,ckdivend2,linkA,linkB,linkC,linkM,ncol[,icol,iqual],npix[,icol,irow,iqual]
  for (auto it: m) {
    s << dumpArray(uint2Blob(it.first));
    // -- ckdivend and ckdivend2
    s << dumpArray(uint2Blob(31));
    s << dumpArray(uint2Blob(31));
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
  // cout << "calPixelQualityLM::readCsv> reset fMapConstants" << endl;
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
    a.ckdivend = stoi(tokens[1]);
    a.ckdivend2 = stoi(tokens[2]);
    a.linkA = stoi(tokens[3]);
    a.linkB = stoi(tokens[4]);
    a.linkC = stoi(tokens[5]);
    a.linkM = stoi(tokens[6]);
    // -- initialize column map
    int ncol = stoi(tokens[7]);
    a.mcol.clear();
    for (unsigned ipix = 0; ipix < ncol; ++ipix) {
      int icol           = stoi(tokens[8 + 2*ipix]);
      unsigned int iqual = stoi(tokens[8 + 2*ipix + 1]);
      a.mcol[icol] = static_cast<char>(iqual);
    }
    // -- initialize pixel map
    int npix = stoi(tokens[8 + 2*ncol]);
    a.mpixel.clear();
    for (unsigned ipix = 0; ipix < npix; ++ipix) {
      int icol           = stoi(tokens[9 + 2*ncol + ipix*3]);
      int irow           = stoi(tokens[9 + 2*ncol + ipix*3 + 1]);
      int iqual          = stoi(tokens[9 + 2*ncol + ipix*3 + 2]);
      a.mpixel[icol*250 + irow] = static_cast<char>(iqual);
    }
    fMapConstants.insert(make_pair(a.id, a));
  }

  // -- set iterator over all constants to the start of the map
  fMapConstantsIt = fMapConstants.begin();
}

// ----------------------------------------------------------------------
void calPixelQualityLM::writeCsv(string filename) {
  ofstream OUTS(filename);
  OUTS << "#chipID,ckdivend,ckdivend2,linkA,linkB,linkC,linkM,ncol[,icol,iqual],npix[,icol,irow,iqual] NB: 0 = good, 1 = noisy, 2 = suspect, 3 = declared bad, 9 = turned off" << endl;

 
  for (auto it: fMapConstants) {
    OUTS << it.first << "," << it.second.ckdivend << "," << it.second.ckdivend2 << "," 
         << it.second.linkA << "," << it.second.linkB << "," << it.second.linkC << "," 
         << it.second.linkM << ",";
    OUTS << it.second.mcol.size();
    for (auto itC: it.second.mcol) {
      OUTS << "," << itC.first;
      OUTS << "," << static_cast<int>(itC.second);
    }
    OUTS << "," << it.second.mpixel.size();
    for (auto itP: it.second.mpixel) {
      OUTS << "," << itP.first/250 << "," << itP.first%250 << "," << static_cast<int>(itP.second);
    }
    OUTS << endl;
  }
  OUTS.close();
}

// ----------------------------------------------------------------------
calPixelQualityLM::Status calPixelQualityLM::getLinkStatus(unsigned int chipid, int ilink) {
  if (fMapConstants.find(chipid) == fMapConstants.end()) {
    return Status::ChipNotFound; // -- chip not found
  }
  
  switch (ilink) {
    case 0: return static_cast<Status>(fMapConstants[chipid].linkA);
    case 1: return static_cast<Status>(fMapConstants[chipid].linkB);
    case 2: return static_cast<Status>(fMapConstants[chipid].linkC);
    case 3: return static_cast<Status>(fMapConstants[chipid].linkM);
    default: return Status::ChipNotFound; // -- invalid link number
  }
}

