#include "cdbUtil.hh"

#include <iostream>
#include <iomanip>
#include <sstream>
#include "calPixelTimeCalibration.hh"

using namespace std;

// ----------------------------------------------------------------------
calPixelTimeCalibration::calPixelTimeCalibration(cdbAbs *db) : calAbs(db) {
}

// ----------------------------------------------------------------------
calPixelTimeCalibration::calPixelTimeCalibration(cdbAbs *db, string tag) : calAbs(db, tag) {
  if (0) cout << "calPixelTimeCalibration created and registered with tag ->" << fTag << "<-"
       << endl;
}


// ----------------------------------------------------------------------
calPixelTimeCalibration::~calPixelTimeCalibration() {
  cout << "this is the end of calPixelTimeCalibration with tag ->" << fTag << "<-" << endl;
}


// ----------------------------------------------------------------------
void calPixelTimeCalibration::calculate(string hash) {
  cout << "calPixelTimeCalibration::calculate() with "
       << "fHash ->" << hash << "<-";
  string spl = fTagIOVPayloadMap[hash].fBLOB;

  std::vector<char> buffer(spl.begin(), spl.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  cout << " header: " << hex << header << dec;

  int globalChipID(0);
  fMapConstants.clear();
  while (ibuffer != buffer.end()) {
    for (uint chip = 0; chip < NCALIBRATIONCHIPS; chip++){
      std::array<std::array<constants, NTOTBINS>, NSECTOR> arr;
      for(uint sector = 0; sector < NSECTOR; sector++){
        for(uint tot = 0; tot < NTOTBINS; tot++){
          globalChipID = blob2Int(getData(ibuffer));
          int s = blob2Int(getData(ibuffer));
          int b = blob2Int(getData(ibuffer));
          arr[sector][tot].mean = blob2Double(getData(ibuffer));
          arr[sector][tot].meanerr = blob2Double(getData(ibuffer));
          arr[sector][tot].sigma = blob2Double(getData(ibuffer));
          arr[sector][tot].sigmaerr = blob2Double(getData(ibuffer));
        }
      }
      fMapConstants.insert(make_pair(globalChipID, arr));
    }
  }
  cout << " inserted " << fMapConstants.size() << " constants" << endl;

}

// ----------------------------------------------------------------------
void calPixelTimeCalibration::printBLOB(std::string sblob, int verbosity) {

  std::vector<char> buffer(sblob.begin(), sblob.end());
  std::vector<char>::iterator ibuffer = buffer.begin();

  unsigned int header = blob2UnsignedInt(getData(ibuffer));
  cout << "calPixelTimeCalibration::printBLOB(string)" << endl;
  cout << "   header: " << hex << header << dec << " (note: chip sector tot mean meanerr sigma sigmaerr)" << endl;

  int c(0), s(0), b(0);
  while (ibuffer != buffer.end()) {
    for (uint sector = 0; sector < NSECTOR; sector++){
      for (uint tot = 0; tot < NTOTBINS; tot++){
        constants a;  
        c = blob2Int(getData(ibuffer));
        s = blob2Int(getData(ibuffer));
        b = blob2Int(getData(ibuffer));
        a.mean = blob2Double(getData(ibuffer));
        a.meanerr = blob2Double(getData(ibuffer));
        a.sigma = blob2Double(getData(ibuffer));
        a.sigmaerr = blob2Double(getData(ibuffer));
        cout << c << " " << s << " " << b << " " 
             << setprecision(6) << fixed     
             << a.mean << " " << a.meanerr << " " << a.sigma << " " << a.sigmaerr 
             << endl;
      }
    }
  }
}


// ----------------------------------------------------------------------
string calPixelTimeCalibration::makeBLOB() {
  stringstream s;
  unsigned int header(0xdeadface);
  s << dumpArray(uint2Blob(header));

  for (auto it: fMapConstants) {
    std::array<std::array<constants, NTOTBINS>, NSECTOR> arr = it.second;
    for(uint sector = 0; sector < NSECTOR; sector++){
      for(uint tot = 0; tot < NTOTBINS; tot++){
        constants a = arr[sector][tot];
        s << dumpArray(int2Blob(it.first));
        s << dumpArray(int2Blob(sector));
        s << dumpArray(int2Blob(tot));
        s << dumpArray(double2Blob(a.mean));
        s << dumpArray(double2Blob(a.meanerr));
        s << dumpArray(double2Blob(a.sigma));
        s << dumpArray(double2Blob(a.sigmaerr));
      }
    }
  }
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

