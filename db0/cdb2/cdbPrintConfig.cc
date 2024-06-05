#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <dirent.h>  /// for directory reading

#include <chrono>

#include "cdbUtil.hh"
#include "base64.hh"

#include "cfgPayload.hh"


using namespace std;

// ----------------------------------------------------------------------
// cdbPrintConfig /path/to/payload
// --------------
//
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------=
int main(int argc, char* argv[]) {

  string filename(""), pdir(""), hash("");
  
  // -- command line arguments
  if (argc < 2) {
    cout << "provide a payload file" << endl;
    return 0;
  } else {
    filename = argv[1];
  }
  
  pdir = filename.substr(0, filename.find_last_of("/")+1);
  hash = filename.substr(filename.find_last_of("/")+1);
  cout << "config ->" << filename  << "<-" << endl
       << "dir ->" << pdir << "<-" << endl
       << "hash ->" << hash << "<-" << endl;
       
  // -- read config
  ifstream INS;
  INS.open(filename);
  if (INS.fail()) {
    cout << "Error failed to open ->" << filename << "<-" << endl;
    return 0;
  }
  
  
  std::stringstream buffer;
  buffer << INS.rdbuf();
  INS.close();
  
  cfgPayload cfg;
  string jstring = buffer.str();
  cfg.fHash      = jsonGetValue(jstring, "cfgHash");
  cfg.fDate      = jsonGetValue(jstring, "cfgDate");
  cfg.fCfgString = jsonGetCfgString(jstring, "cfgString");
  
  cfg.print(true);
  
  return 0;
}
