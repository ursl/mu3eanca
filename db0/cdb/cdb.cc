#include "cdb.hh"

#include <iostream>

using namespace std;

// ----------------------------------------------------------------------
cdb::cdb(string globaltag, string uri) : fGT(globaltag), fURI(uri) {

}

// ----------------------------------------------------------------------
cdb::~cdb() {
  cout << "this is the end of CDB with global tag " << fGT << "." << endl;
}


// ----------------------------------------------------------------------
void cdb::init() {
  readGlobalTags();
	readTags();
	readIOVs();
}


// ----------------------------------------------------------------------
int cdb::whichIOV(int runnumber, string tag) {
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
void cdb::setRunNumber(int runnumber) {
  cout << "cdb::setRunNumber(" << runnumber << "), old runnumber = " 
			 << fRunNumber
			 << endl;
	if (runnumber != fRunNumber) {
		cout  << " do something!" << endl;
		fRunNumber = runnumber;
	}
}


// ----------------------------------------------------------------------
void cdb::print(std::vector<int> v, int istart) {
	for (unsigned int i = istart; i < v.size(); ++i) {
		cout << v[i] << " ";
	}
	cout << endl;
}

// ----------------------------------------------------------------------
void cdb::print(std::vector<std::string> v, int istart) {
	for (unsigned int i = istart; i < v.size(); ++i) {
		cout << v[i] << " ";
	}
	cout << endl;
}

// ----------------------------------------------------------------------
void cdb::print(std::map<std::string, std::vector<std::string>> m) {
	for (auto it: m) {
		cout << it.first << ": ";
		print(it.second);
	}
	cout << endl;
}

// ----------------------------------------------------------------------
void cdb::print(std::map<std::string, std::vector<int>> m) {
	for (auto it: m) {
		cout << it.first << ": ";
		print(it.second);
	}
	cout << endl;
}
