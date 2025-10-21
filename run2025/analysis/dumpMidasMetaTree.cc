#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <math.h>
#include <string>


#include <iterator>
#include <map>
#include <set>
#include <vector>
#include <sstream>
#include "../../common/json.h"  // nlohmann::json
#include <nlohmann/json.hpp>    // for nlohmann::ordered_json

#include "util.hh"
#include "asicInfo.hh"

using namespace std;



// ----------------------------------------------------------------------
int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <file1.json> [file2.json] [file3.json] ..." << std::endl;
    return 2;
  }

}