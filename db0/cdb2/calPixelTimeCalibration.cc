#include "cdbUtil.hh"

#include <iostream>
#include <sstream>
#include "calPixelTimeCalibration.hh"

using namespace std;

// ----------------------------------------------------------------------
calPixelTimeCalibration::calPixelTimeCalibration(cdbAbs *db) : calAbs(db) {
}

// ----------------------------------------------------------------------
calPixelTimeCalibration::calPixelTimeCalibration(cdbAbs *db, string tag) : calAbs(db, tag) {
  cout << "calPixelTimeCalibration created and registered with tag ->" << fTag << "<-"
       << endl;
}


// ----------------------------------------------------------------------
calPixelTimeCalibration::~calPixelTimeCalibration() {
  cout << "this is the end of calPixelTimeCalibration with tag ->" << fTag << "<-" << endl;
}


// ----------------------------------------------------------------------
void calPixelTimeCalibration::calculate(string hash) {
  if (fVerbose > 0) cout << "calPixelTimeCalibration::calculate() with "
       << "fHash ->" << hash << "<-"
       << endl;
  string spl = fTagIOVPayloadMap[hash].fBLOB;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  if (fVerbose > 0) cout << "calPixelTimeCalibration header: " << hex << header << dec << endl;

  int npix(0), ncol(0);
  while (ibuffer != buffer.end()) {
    constants a;
  }

}

// ----------------------------------------------------------------------
void calPixelTimeCalibration::printBLOB(std::string sblob, int verbosity) {

  std::vector<char> buffer(sblob.begin(), sblob.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  cout << "calPixelTimeCalibration::printBLOB(string)" << endl;
  cout << "   header: " << hex << header << dec << " (note: 0 = good, 1 = noisy, 2 = suspect, 3 = declared bad, 9 = turned off)" << endl;

  while (ibuffer != buffer.end()) {
    unsigned int ichip = blob2UnsignedInt(getData(ibuffer));
    unsigned int isector = blob2UnsignedInt(getData(ibuffer));
    unsigned int itotbin = blob2UnsignedInt(getData(ibuffer));
    constants a;  
    a.mean = blob2Double(getData(ibuffer));
    a.meanerr = blob2Double(getData(ibuffer));
    a.sigma = blob2Double(getData(ibuffer));
    a.sigmaerr = blob2Double(getData(ibuffer));
    fMapConstants[ichip][isector][itotbin] = a;
  }
}


// ----------------------------------------------------------------------
string calPixelTimeCalibration::makeBLOB() {
  stringstream s;
  // unsigned int header(0xdeadface);
  // s << dumpArray(uint2Blob(header));

  // // -- format of m
  // // chipID => [npix, n*(col, row, iqual)]
  // for (auto it: fMapConstants) {
  //   s << dumpArray(uint2Blob(it.first));
  //   constants a = it.second;

  //   // s << dumpArray(uint2Blob(a.ckdivend)); // -- ckdivend
  //   // s << dumpArray(uint2Blob(a.ckdivend2)); // -- ckdivend2

  //   // s << dumpArray(uint2Blob(a.linkA)); // -- linkA
  //   // s << dumpArray(uint2Blob(a.linkB)); // -- linkB
  //   // s << dumpArray(uint2Blob(a.linkC)); // -- linkC
  //   // s << dumpArray(uint2Blob(a.linkM)); // -- linkM

  //   // // -- get number of column entries
  //   // int ncol = a.mcol.size();
  //   // s << dumpArray(int2Blob(ncol));
  //   // for (auto it: a.mcol) {
  //   //   int icol = it.first;
  //   //   unsigned int iqual = static_cast<unsigned int>(it.second);
  //   //   s << dumpArray(int2Blob(icol));
  //   //   s << dumpArray(uint2Blob(iqual));
  //   // }

  //   // // -- get number of pixel entries
  //   // int npix = a.mpixel.size();
  //   // s << dumpArray(int2Blob(npix));
  //   // for (auto it: a.mpixel) {
  //   //   int idx = it.first;
  //   //   int icol = idx/250;
  //   //   int irow = idx%250;
  //   //   unsigned int iqual = static_cast<unsigned int>(it.second);
  //   //   s << dumpArray(int2Blob(icol));
  //   //   s << dumpArray(int2Blob(irow));
  //   //   s << dumpArray(uint2Blob(iqual));
  //   // }

  // }
  return s.str();

}

// ----------------------------------------------------------------------
void calPixelTimeCalibration::readTxtFile(string filename) {

  FILE *cf = fopen(filename.c_str(), "r");
  fMapConstants.clear();

  int c(0), s(0), b(0);
  for(uint chip = 0; chip < NCALIBRATIONCHIPS; chip++){
    std::array<std::array<constants, NTOTBINS>, NSECTOR> arr;
    for(uint sector = 0; sector < NSECTOR; sector++){
      for(uint tot = 0; tot < NTOTBINS; tot++){
        fscanf(cf, "%i %i %i %lf %lf %lf %lf\n",
          &c,
          &s,
          &b,
          &(arr[sector][tot].mean),
          &(arr[sector][tot].meanerr),
          &(arr[sector][tot].sigma),
          &(arr[sector][tot].sigmaerr));
        if (s != sector || b != tot) {
          cout << "calPixelTimeCalibration::readTxtFile> Error, sector or tot mismatch: expected (" << sector << "," << tot << ") got (" << s << "," << b << ")" << endl;
        }
      }
    }
    fMapConstants.insert(make_pair(c, arr));
  }
  fclose(cf);
  cout << "calPixelTimeCalibration::readTxtFile> read " << fMapConstants.size() << " chips" << endl;
}

// ----------------------------------------------------------------------
void calPixelTimeCalibration::writeTxtFile(string filename) {
  FILE *cf = fopen(filename.c_str(), "w");

  for(auto it: fMapConstants){
    uint chip = it.first;
    std::array<std::array<constants, NTOTBINS>, NSECTOR> a = it.second;
    for(uint sector = 0; sector < NSECTOR; sector++){
      for(uint tot = 0; tot < NTOTBINS; tot++){
        fprintf(cf, "%i %i %i %lf %lf %lf %lf\n",
        chip, sector, tot,
        a[sector][tot].mean,
        a[sector][tot].meanerr,
        a[sector][tot].sigma,
        a[sector][tot].sigmaerr);
      }
    }
  }
  fclose(cf);
}

// ----------------------------------------------------------------------
const calPixelTimeCalibration::constants& calPixelTimeCalibration::getConstants(int ichip, int isector, int itotbin) const {
  auto it = fMapConstants.find(ichip);
  if (it == fMapConstants.end()) {
    static constants empty{0.0, 0.0, 0.0, 0.0};
    return empty;
  }
  // -- bounds checking for array indices
  if (isector < 0 || isector >= NSECTOR || itotbin < 0 || itotbin >= NTOTBINS) {
    static constants empty{0.0, 0.0, 0.0, 0.0};
    return empty;
  }
  return it->second[isector][itotbin];
}

// ----------------------------------------------------------------------
double calPixelTimeCalibration::getMean(int ichip, int isector, int itotbin) const {
  return getConstants(ichip, isector, itotbin).mean;
}

// ----------------------------------------------------------------------
double calPixelTimeCalibration::getMeanErr(int ichip, int isector, int itotbin) const {
  return getConstants(ichip, isector, itotbin).meanerr;
}

// ----------------------------------------------------------------------
double calPixelTimeCalibration::getSigma(int ichip, int isector, int itotbin) const {
  return getConstants(ichip, isector, itotbin).sigma;
}

// ----------------------------------------------------------------------
double calPixelTimeCalibration::getSigmaErr(int ichip, int isector, int itotbin) const {
  return getConstants(ichip, isector, itotbin).sigmaerr;
}

