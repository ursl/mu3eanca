#include "cdbAbs.hh"

#include <iostream>
#include <sstream>

#include "calAbs.hh"

using namespace std;

// ----------------------------------------------------------------------
cdbAbs::cdbAbs(string uri, int verbose) : fURI(uri), fVerbose(verbose) {
  
}


// ----------------------------------------------------------------------
cdbAbs::~cdbAbs() {
  if (fVerbose > 0) cout << "this is the end of CDBABS." << endl;
}


// ----------------------------------------------------------------------
void cdbAbs::init() {
  if (fVerbose > 0)  cout << "cdbAbs::init() " << endl;
  // readGlobalTags();
	// readTags();
	// readIOVs();
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


