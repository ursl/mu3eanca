
#include "../common/json.h"
//#include "nlohmann/json.hpp"

//using json = nlohmann::json;

void test() {
  
  ifstream i("../common/sensors_mapping_220531.json");
  nlohmann::json jMap;
  i >> jMap;
   
}
