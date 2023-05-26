#include "cdb.hh"

#include <iostream>

using namespace std;

// ----------------------------------------------------------------------
cdb::cdb(string name, string uri) : fName(name), fURI(uri) {

}

// ----------------------------------------------------------------------
cdb::~cdb() {
  cout << "this is the end of " << fName << "." << endl;
}
