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
  if (fVerbose > 0) cout << "calPixelQualityLM::calculate() with "
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
  cout << "calPixelQuality::printBLOB(string)" << endl;
  cout << "   header: " << hex << header << dec << " (note: 0 = good, 1 = noisy, 2 = suspect, 3 = declared bad, 9 = turned off)" << endl;

  while (ibuffer != buffer.end()) {
    // -- chipID
    unsigned int chipID = blob2UnsignedInt(getData(ibuffer));
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


  int ibin, ichip, isector; 
  double mean, meanerr, sigma, sigmaerr;
  FILE *cf = fopen(filename.c_str(), "r");

  for(uint bin =0; bin < NTOTBINS; bin++){
    fscanf(cf, "%i %lf %lf %lf %lf\n",
        &ibin,
        &mean,
        &meanerr,
        &sigma,
        &sigmaerr);
}
for(uint chip =0; chip < NCALIBRATIONCHIPS; chip++){
    for(uint sector = 0; sector < NSECTOR; sector++){
        fscanf(cf, "%i %i %lf %lf %lf %lf\n",
        &ichip,
        &isector,
        &mean,
        &meanerr,
        &sigma,
        &sigmaerr
    );
    }
}
for(uint chip =0; chip < NCALIBRATIONCHIPS; chip++){
    for(uint tot = 0; tot < NTOTBINS; tot++){
        fscanf(cf, "%i %i %lf %lf %lf %lf\n",
        &ichip,
        &ibin,
        &mean,
        &meanerr,
        &sigma,
        &sigmaerr
    );
    }
}
 
for(uint chip = 0; chip < NCALIBRATIONCHIPS; chip++){
    for(uint sector = 0; sector < NSECTOR; sector++){
        for(uint tot = 0; tot < NTOTBINS; tot++){
            fscanf(cf, "%i %i %i %lf %lf %lf %lf\n",
            &(fArrayConstants[chip][sector][tot].chipnr),
            &(fArrayConstants[chip][sector][tot].sector),
            &(fArrayConstants[chip][sector][tot].totbin),
            &(fArrayConstants[chip][sector][tot].mean),
            &(fArrayConstants[chip][sector][tot].meanerr),
            &(fArrayConstants[chip][sector][tot].sigma),
            &(fArrayConstants[chip][sector][tot].sigmaerr)
            );
        }
    }
}
fclose(cf);
 

}

// ----------------------------------------------------------------------
void calPixelTimeCalibration::writeTxtFile(string filename) {
  FILE *cf = fopen(filename.c_str(), "w");

  for(uint chip = 0; chip < NCALIBRATIONCHIPS; chip++){
    for(uint sector = 0; sector < NSECTOR; sector++){
      for(uint tot = 0; tot < NTOTBINS; tot++){
        fprintf(cf, "%i %i %i %lf %lf %lf %lf\n",
        fArrayConstants[chip][sector][tot].chipnr,
        fArrayConstants[chip][sector][tot].sector,
        fArrayConstants[chip][sector][tot].totbin,
        fArrayConstants[chip][sector][tot].mean,
        fArrayConstants[chip][sector][tot].meanerr,
        fArrayConstants[chip][sector][tot].sigma,
        fArrayConstants[chip][sector][tot].sigmaerr);
      }
    }
  }
  fclose(cf);
}

