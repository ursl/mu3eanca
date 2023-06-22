#include "cdbAbs.hh"

#include <iostream>
#include <sstream>

#include "calAbs.hh"

using namespace std;

// ----------------------------------------------------------------------
cdbAbs::cdbAbs(string globaltag, string uri, int verbose) : fGT(globaltag), fURI(uri), fVerbose(verbose) {

}

// ----------------------------------------------------------------------
cdbAbs::~cdbAbs() {
  if (fVerbose > 0) cout << "this is the end of CDBABS with global tag " << fGT << "." << endl;
}


// ----------------------------------------------------------------------
void cdbAbs::init() {
  if (fVerbose > 0)  cout << "cdbAbs::init() for GT = " << fGT << endl;
  readGlobalTags();
	readTags();
	readIOVs();
}


// ----------------------------------------------------------------------
int cdbAbs::whichIOV(int runnumber, string tag) {
	int iov(-1);
  for (auto it : fIOVs[tag]) {
    if (it > runnumber) {
			return iov;
    } else {
			iov = it;
		}
  }
	return iov; 
}


// ----------------------------------------------------------------------
string cdbAbs::getHash(int runnumber, string tag) {
  int iov = whichIOV(runnumber, tag);
  // -- hash is a misnomer here
  std::stringstream ssHash;
  ssHash << "tag_" << tag << "_iov_" << iov;
  if (fVerbose > 4) cout << "cdbAbs::getHash(" << runnumber << ", " << tag << ") = " << ssHash.str() << endl;
  return ssHash.str();
}


// ----------------------------------------------------------------------
void cdbAbs::setRunNumber(int runnumber) {
  if (fVerbose > 0)   cout << "cdbAbs::setRunNumber(" << runnumber << "), old runnumber = " 
                           << fRunNumber
                           << " fCalibrations.size() = " << fCalibrations.size()
                           << endl;
  
	if (runnumber != fRunNumber) {
		fRunNumber = runnumber;
    // -- call update for all registered calibrations
    //    each calibration will check with its tag/IOV whether an update is required
    for (auto it: fCalibrations) {
      it.second->update();
    }
	}
}


// ----------------------------------------------------------------------
void cdbAbs::print(std::vector<int> v, int istart) {
	for (unsigned int i = istart; i < v.size(); ++i) {
		cout << v[i] << " ";
	}
	cout << endl;
}

// ----------------------------------------------------------------------
void cdbAbs::print(std::vector<std::string> v, int istart) {
	for (unsigned int i = istart; i < v.size(); ++i) {
		cout << v[i] << " ";
	}
	cout << endl;
}

// ----------------------------------------------------------------------
void cdbAbs::print(std::map<std::string, std::vector<std::string>> m) {
	for (auto it: m) {
		cout << it.first << ": ";
		print(it.second);
	}
	cout << endl;
}

// ----------------------------------------------------------------------
void cdbAbs::print(std::map<std::string, std::vector<int>> m) {
	for (auto it: m) {
		cout << it.first << ": ";
		print(it.second);
	}
	cout << endl;
}


// ----------------------------------------------------------------------
void cdbAbs::registerCalibration(string tag, calAbs *c) {
  cout << "cdbAbs::registerCalibration name ->" << c->getName()
       << "<- with tag ->" << tag << "<-"
       << endl;
  fCalibrations.insert(make_pair(tag, c));
  cout << "   done" << endl;
}
