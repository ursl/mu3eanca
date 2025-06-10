#include "cdbAbs.hh"
#include "cdbRest.hh"
#include "runRecord.hh"
#include "cdbUtil.hh"
#include "Mu3eConditions.hh"

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string.h>
#include <dirent.h>  /// for directory reading
#include <sys/time.h>
#include <unistd.h>
#include <cctype> // for toupper()
#include <algorithm> // for transform()
#include <nlohmann/json.hpp>  // Add JSON library

#include <curl/curl.h>

using json = nlohmann::ordered_json;  // Use ordered_json instead of json
using namespace std;
// ----------------------------------------------------------------------
// -- syncRunDB
// -- ---------
// -- 
// -- produce updates to either DataQuality or RunInfo attributes of run records in RDB
// -- updates a mongoDB server, does not run on the JSON backend server
// --
// -- Usage: bin/syncRDB -m mode -f firstRun -l lastRun [-t ../../db1/rest/runInfoTemplate.json] [-h localhost]
// --
// -- Examples: bin/syncRDB -m 2 -g tkar -c cosmic -s significant -h pc11740
//              bin/syncRDB -m 3 -k "EOR.Comments" -v "New comment" -h <hostname>
//              bin/syncRDB -m 3 -k "EOR.Comments" -v "+= Additional comment" -h <hostname>
// --
// -- -m mode: 0: upload template (magic words: dqTemplate.json or runInfoTemplate.json) to run records
// --          1: parse shift comments and set runInfo field "Significant"
// --          2: select runs from RDB based on selection string and class string
// --          3: modify field in runRecord and upload modified record to RDB
// --          4: add field in runRecord and upload modified record to RDB
// --          5: delete field in runRecord and upload modified record to RDB
// --          6: if RunInfo.Class exists, copy its value into "BOR.Run Class". If RunInfo.Class is unset, copy from "Bor.Run Class".
// --          7: capitalize RunInfo.Class and if its value is "calib" change it to "Calibration"
// --          8: set RunInfo.Significant to value
// --          9: check if Resources exist and remove all documents with description XXX in that array
// --
// -- History:
// --   2025/06/10: add mode 9
// --   2025/06/05: add modes 3, 4, 5, and 6 and 7. Add reading of certification files.
// --   2025/05/12: replace junk with significant
// --   2025/05/20: add mode 2
// --   2025/05/08: first shot
// 
// ------------------------------------------------------------------------

// Forward declarations
void rdbMode1(runRecord &, bool);
void rdbMode0(runRecord &, bool);
void rdbMode2(string &, string &, string &, bool);
void rdbMode3(int irun, string key, string value, bool debug);
void rdbMode4(int irun, string key, string value, bool debug);
void rdbMode5(int irun, string key, bool debug);
void rdbMode6(int irun, bool debug);
void rdbMode7(int irun, bool debug);
void rdbMode8(int irun, string value, bool debug);
void rdbMode9(int irun, string description, bool debug);
string getCurrentDateTime();

// ----------------------------------------------------------------------
static size_t cdbRestWriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}


string runInfoTemplateFile = "../../db1/rest/runInfoTemplate.json";
string rdbUpdateString(":5050/rdb/addAttribute");
string rdbGetString(":5050/rdb/run");
string rdbPutString(":5050/rdb/runrecords");
vector<string> runInfoTemplateFileLines;

// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {

  // -- command line arguments
  string hostString("pc11740");
  string urlString(":5050/cdb");
  string selectionString("significant"), classString("cosmic"), goodString("");
  string key("unset"), value("unset");
  bool debug(false);
  int firstRun(0), lastRun(-1), mode(0);
  string runfile("unset");
  for (int i = 0; i < argc; i++) {
    // -- runfile
    if (!strcmp(argv[i], "-r"))    {runfile = string(argv[++i]);}

    if (!strcmp(argv[i], "-d"))    {debug = true;}
    if (!strcmp(argv[i], "-m"))    {mode    = atoi(argv[++i]);}
    // -- run range selection
    if (!strcmp(argv[i], "-f"))    {firstRun = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-l"))    {lastRun = atoi(argv[++i]);}
    // -- stuff
    if (!strcmp(argv[i], "-h"))    {hostString = string(argv[++i]);}
    if (!strcmp(argv[i], "-t"))    {runInfoTemplateFile = string(argv[++i]);}
    // -- selection for runlist output (mode = 2)
    if (!strcmp(argv[i], "-g"))    {goodString = string(argv[++i]);}
    if (!strcmp(argv[i], "-c"))    {classString = string(argv[++i]);}
    if (!strcmp(argv[i], "-s"))    {selectionString = string(argv[++i]);}
    // -- key and value for mode = 3
    if (!strcmp(argv[i], "-k"))    {key = string(argv[++i]);}
    if (!strcmp(argv[i], "-v"))    {value = string(argv[++i]);}
  }

  urlString = hostString + urlString; 
  rdbUpdateString = hostString + rdbUpdateString;
  rdbGetString = hostString + rdbGetString;
  rdbPutString = hostString + rdbPutString;

  // -- read in template for mode = 0
  if (0 == mode) {
  ifstream file(runInfoTemplateFile);
  if (file.is_open()) {
    string line;
    while (getline(file, line)) {
      runInfoTemplateFileLines.push_back(line);
    }
    file.close();
  } else {
      cout << "Error: Unable to open file " << runInfoTemplateFile << endl;
    }
  }

  cdbAbs *pDB(0);

  pDB = new cdbRest("mcidealv6.1", urlString, 0);
  Mu3eConditions *pDC = Mu3eConditions::instance("mcidealv6.1", pDB);


  if (2 == mode) {
    rdbMode2(selectionString, classString, goodString, debug);
    delete pDB;
    return 0; 
  }


  vector<string> vRunNumbers;
  if (runfile == "unset") {
    vRunNumbers = pDB->getAllRunNumbers();
  } else {
    ifstream file(runfile);
    string line;
    string fileContent;
    while (getline(file, line)) {
      fileContent += line + "\n";
    }
    file.close();
    replaceAll(fileContent, "\n", "");
    replaceAll(fileContent, " ", "");
    replaceAll(fileContent, "{", "");
    replaceAll(fileContent, "}", "");
    vRunNumbers = split(fileContent, ',');
  } 

  cout << "vRunNumbers.size() ->" << vRunNumbers.size() << "<-" << endl;

  for (int it = 0; it < vRunNumbers.size(); ++it) {
    int irun = stoi(vRunNumbers[it]);
    if (irun < firstRun) continue;
    if ((lastRun > 0) && (irun > lastRun)) continue;
    if (3 == mode) {
      rdbMode3(irun, key, value, debug);
      continue;
    }
    if (4 == mode) {
      rdbMode4(irun, key, value, debug);
      continue;
    }
    if (5 == mode) {
      rdbMode5(irun, key, debug);
      continue;
    }
    if (6 == mode) {
      rdbMode6(irun, debug);
      continue;
    }
    if (7 == mode) {
      rdbMode7(irun, debug);
      continue;
    }
    if (8 == mode) {
      rdbMode8(irun, value, debug);
      continue;
    }
    if (9 == mode) {
      rdbMode9(irun, value, debug);
      continue;
    }

    runRecord rr = pDB->getRunRecord(irun);
    if (0 == mode) rdbMode0(rr, debug);
    if (1 == mode) rdbMode1(rr, debug);

  }


  delete pDB; 
} 

// ----------------------------------------------------------------------
// -- upload template {runInfo,dataQuality} to run records 
// -- IFF they do not yet contain that attribute
void rdbMode0(runRecord &rr, bool debug) {
  if (!rr.fBOREORValid) {
    cout << "incomplete run record in RDB, skipping ................................. " << endl;
    cout << rr.printSummary() << endl;
    return;
  }

  // -- check whether this is about uploading the dataQuality template and whether that exists
  if (string::npos != runInfoTemplateFile.find("dqTemplate")) {
    if (rr.fDataQualityIdx < 0) {
      cout << "no DataQuality attribute found for run number: " << rr.fBORRunNumber << endl;
      int irun = rr.fBORRunNumber;
      stringstream ss;
      ss << "curl -X PUT -H \"Content-Type: application/json\" --data-binary @" << runInfoTemplateFile << " " << rdbUpdateString << "/" << irun;
      cout << "Updating for run number: " << irun << endl; 
      cout << ss.str() << endl;      
      if (!debug) {
        system(ss.str().c_str());
      }
    }
  }

  // -- check whether this is about uploading the RunInfo template and whether that exists
  if (string::npos != runInfoTemplateFile.find("runInfoTemplate")) {
    if (rr.fRunInfoIdx < 0) {
      cout << "no runInfo attribute found for run number: " << rr.fBORRunNumber << endl;
      int irun = rr.fBORRunNumber;
      stringstream ss;
      ss << "curl -X PUT -H \"Content-Type: application/json\" --data-binary @" << runInfoTemplateFile << " " << rdbUpdateString << "/" << irun;
      cout << "Updating for run number: " << irun << endl; 
      cout << ss.str() << endl;      
      if (!debug) {
        system(ss.str().c_str());
      }
    }
  }
}

// ----------------------------------------------------------------------
// -- set the following flags by parsing the shift comments
void rdbMode1(runRecord &rr, bool debug) {
  cout << "hallo" << endl;
  RunInfo ri = rr.getRunInfo();

  string xstring = rr.fEORComments;
  transform(xstring.begin(), xstring.end(), xstring.begin(), [](unsigned char c) { return tolower(c); });

  string significantFromComments = "not found";
  string classFromComments = "not found";

  cout << "run number: " << rr.fBORRunNumber << ": ";

  // -- modify significantFromComments for special tags
  vector<string> vJunkIndicators = {"bad", "unstable", "error", "problem", "dbx", "fail", "debug", "test", "dummy"};
  for (const auto &indicator : vJunkIndicators) {
    if (xstring.find(indicator) != string::npos) {
      cout << ", overriding junk significant indicator: " << indicator;
      classFromComments = "Junk";
      significantFromComments = "false";
      break;
    }
  }

  // -- modify classFromComments for special tags
  if (classFromComments == "not found") {
    vector<string> vClass2Indicators = {"mask", "tune", "tuning", "calib", "noise"};
    for (const auto &indicator : vClass2Indicators) {
      if (xstring.find(indicator) != string::npos) {
        significantFromComments = "false";
        break;
      }
    }
  }
  cout << endl;

  if (significantFromComments == "not found") {
    significantFromComments = "false";
  } else {
    if (significantFromComments != ri.significant) {
      ri.significant = significantFromComments;
    }
  }
    
  // if (classFromComments == "not found") { 
  //   classFromComments = "Junk";
  // } else {
  //   if (ri.Class == "unset") {
  //     ri.Class = classFromComments;
  //   } else {
  //     if (ri.Class != classFromComments) {
  //       if (ri.comments == "unset") ri.comments = "";
  //       //ri.comments += " from " + ri.Class + " to: " + classFromComments;
  //       //ri.Class = classFromComments;
  //     }
  //   }
  // }
  
  // -- write to file
  stringstream ss;
  ss << "rdb/runInfo_" << rr.fBORRunNumber << ".json";
  ofstream ofs(ss.str());
  // for (const auto &line : newRunInfo) {
  //   ofs << line << endl;
  // }
  ofs << ri.json() << endl;
  ofs.close();

  if (!debug) {
    // curl -X PUT -H "Content-Type: application/json" --data-binary @/Users/ursl/mu3e/mu3eanca/db1/rest/runInfoTemplate.json http://localhost:5050/rdb/addAttribute/513
    int irun = rr.fBORRunNumber;
    stringstream ss;
    ss << "curl -X PUT -H \"Content-Type: application/json\" --data-binary @rdb/runInfo_" << irun << ".json " << rdbUpdateString << "/" << irun;
    cout << "Updating for run number: " << irun << endl; 
    cout << ss.str() << endl;      
    system(ss.str().c_str());
  }
}

// ----------------------------------------------------------------------
// -- select runs from RDB based on selection string and class string
void rdbMode2(string &selectionString, string &classString, string &goodString, bool debug) {
  Mu3eConditions *pDC = Mu3eConditions::instance();
  vector<string> vRunNumbers = pDC->getAllRunNumbers();
  vector<int> vSelectedRuns;
  for (const auto &runNumber : vRunNumbers) {
    int irun = stoi(runNumber); 
    runRecord rr = pDC->getRunRecord(irun);
    if (!rr.fBOREORValid) continue;
    if (selectionString == "significant") {
      if (rr.isSignificant()) {
        if (classString != "") {
          if (rr.getRunInfoClass() == classString) {
            if (goodString != "") {
              string commentsLower = rr.getRunInfoComments();
              string eorCommentsLower = rr.fEORComments;
              string goodStringLower = goodString;
              transform(commentsLower.begin(), commentsLower.end(), commentsLower.begin(), [](unsigned char c) { return tolower(c); });
              transform(eorCommentsLower.begin(), eorCommentsLower.end(), eorCommentsLower.begin(), [](unsigned char c) { return tolower(c); });
              transform(goodStringLower.begin(), goodStringLower.end(), goodStringLower.begin(), [](unsigned char c) { return tolower(c); });
              if ((string::npos != commentsLower.find(goodStringLower))
                || (string::npos != eorCommentsLower.find(goodStringLower))
              ) {
                cout << "added run number: " << runNumber << " selectionString: " << selectionString << " classString: " << classString << " goodString: " << goodString << endl;
                vSelectedRuns.push_back(irun);
              }
            } else {
              cout << "added run number: " << runNumber << " selectionString: " << selectionString << " classString: " << classString << endl;
              vSelectedRuns.push_back(irun);
            }
          }
        } 
      }
    }
  }

  ofstream ofs("selectedRuns-" + selectionString + (classString != "" ? string("-" + classString) : "")
                               + (goodString != "" ? string("-" + goodString) : "") + ".txt");
  ofs << "{"; 
  for (const auto &irun : vSelectedRuns) {
    ofs << irun;
    if (irun != vSelectedRuns.back()) ofs << ", ";
  }
  ofs << "}" << endl;
  ofs.close();
}

// ----------------------------------------------------------------------
// Helper function to convert string to appropriate type
json convertValueToType(const string& value, bool isAppend = false) {
    // Convert to lowercase for boolean comparison
    string valueLower = value;
    transform(valueLower.begin(), valueLower.end(), valueLower.begin(), ::tolower);
    
    // Check for boolean values
    if (valueLower == "true" || valueLower == "false") {
        return valueLower == "true";
    }
    
    // Check if the string contains a decimal point
    if (value.find('.') != string::npos) {
        try {
            double floatVal = std::stod(value);  // Use stod instead of stof for better precision
            return floatVal;
        } catch (...) {
            // If conversion fails, continue to other types
        }
    }
    
    // Try to convert to integer
    try {
        int intVal = std::stoi(value);
        return intVal;
    } catch (...) {
        // Not a number or boolean, return as string
        return value;
    }
}

// ----------------------------------------------------------------------
// -- modify field in runRecord and upload modified record to RDB
void rdbMode3(int irun, string key, string value, bool debug) {
  bool DBX(false);
  // Validate input parameters
  if (key == "unset" || value == "unset") {
    cerr << "Error: Both key and value must be set. Current values:" << endl;
    cerr << "  key: " << key << endl;
    cerr << "  value: " << value << endl;
    return;
  }

  // Check if this is an append operation
  bool isAppend = false;
  if (value.substr(0, 2) == "+=") {
    isAppend = true;
    value = value.substr(2);  // Remove the += prefix
  }

  string responseData;
  CURL* curl = curl_easy_init();
  if (curl) {
    std::string url = rdbGetString + "/" + std::to_string(irun) + "/json";
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cdbRestWriteCallback);  // Use existing callback
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
    
    // Set headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    if (debug) {
      cout << "Making request to: " << url << endl;
    } else {
      CURLcode res = curl_easy_perform(curl);
      if (res != CURLE_OK) {
        cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return;
      }
      
      if (DBX) cout << "responseData: ->" << responseData << "<-" << endl;
      try {
        // Parse the JSON response
        json j = json::parse(responseData);
        
        // Split the key into parts using dot notation
        vector<string> keyParts;
        stringstream keyStream(key);
        string part;
        while (getline(keyStream, part, '.')) {
          keyParts.push_back(part);
        }
        
        // Navigate to the nested key
        json* current = &j;
        for (size_t i = 0; i < keyParts.size() - 1; ++i) {
          if (current->contains(keyParts[i])) {
            current = &((*current)[keyParts[i]]);
          } else {
            cerr << "Key path not found: " << keyParts[i] << " in path " << key << endl;
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return;
          }
        }
        
        // Get the final key part
        string finalKey = keyParts.back();
        
        // Check if the final key exists
        if (current->contains(finalKey)) {
          // Store the old value for logging
          string oldValue = (*current)[finalKey].dump();
          
          // Handle the value update
          if (isAppend && (*current)[finalKey].is_string()) {
            // For append operation on strings
            string currentValue = (*current)[finalKey].get<string>();
            string newValue = currentValue + (currentValue.empty() ? "" : " ") + value;
            (*current)[finalKey] = newValue;
          } else {
            // Normal update (or append on non-string which becomes a normal update)
            (*current)[finalKey] = convertValueToType(value);
          }
          
          // Convert back to string
          responseData = j.dump(2);  // Pretty print with 2-space indentation
          
          cout << "Updated run " << irun << ": " << key << " from " << oldValue << " to " << (*current)[finalKey].dump() << endl;
          if (DBX) cout << "responseData: ->" << responseData << "<-" << endl;

          // Now we need to send the updated JSON back to the server
          curl_easy_reset(curl);
          curl_easy_setopt(curl, CURLOPT_URL, rdbPutString.c_str());
          curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
          curl_easy_setopt(curl, CURLOPT_POSTFIELDS, responseData.c_str());
          curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
          
          // Perform the update request
          res = curl_easy_perform(curl);
          if (res != CURLE_OK) {
            cerr << "Failed to update run record: " << curl_easy_strerror(res) << endl;
          } else {
            cout << "Successfully updated run record" << endl;
          }
        } else {
          cerr << "Key '" << finalKey << "' not found in path " << key << endl;
        }
      } catch (const json::parse_error& e) {
        cerr << "Failed to parse JSON response: " << e.what() << endl;
      } catch (const std::exception& e) {
        cerr << "Error processing JSON: " << e.what() << endl;
      }
    }
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }
}

// ----------------------------------------------------------------------
// -- add field in runRecord and upload modified record to RDB
void rdbMode4(int irun, string key, string value, bool debug) {
  bool DBX(false);
  // Validate input parameters
  if (key == "unset" || value == "unset") {
    cerr << "Error: Both key and value must be set. Current values:" << endl;
    cerr << "  key: " << key << endl;
    cerr << "  value: " << value << endl;
    return;
  }

  string responseData;
  CURL* curl = curl_easy_init();
  if (curl) {
    std::string url = rdbGetString + "/" + std::to_string(irun) + "/json";
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cdbRestWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
    
    // Set headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    if (debug) {
      cout << "Making request to: " << url << endl;
    } else {
      CURLcode res = curl_easy_perform(curl);
      if (res != CURLE_OK) {
        cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return;
      }
      
      if (DBX) cout << "responseData: ->" << responseData << "<-" << endl;
      try {
        // Parse the JSON response
        json j = json::parse(responseData);
        
        // Split the key into parts using dot notation
        vector<string> keyParts;
        stringstream keyStream(key);
        string part;
        while (getline(keyStream, part, '.')) {
          keyParts.push_back(part);
        }
        
        // Navigate to the nested key
        json* current = &j;
        for (size_t i = 0; i < keyParts.size() - 1; ++i) {
          if (current->contains(keyParts[i])) {
            current = &((*current)[keyParts[i]]);
          } else {
            cerr << "Parent object not found: " << keyParts[i] << " in path " << key << endl;
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return;
          }
        }
        
        // Get the final key part
        string finalKey = keyParts.back();
        
        // Check if the key already exists
        if (current->contains(finalKey)) {
            cout << "Key '" << key << "' already exists with value: " << (*current)[finalKey].dump() << endl;
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return;
        }
        
        // Add the new key-value pair
        (*current)[finalKey] = convertValueToType(value);
        
        // Convert back to string
        responseData = j.dump(2);  // Pretty print with 2-space indentation
        
        cout << "Added new key-value pair: " << key << " = " << (*current)[finalKey].dump() << endl;
        if (DBX) cout << "responseData: ->" << responseData << "<-" << endl;

        // Send the updated JSON back to the server
        curl_easy_reset(curl);
        curl_easy_setopt(curl, CURLOPT_URL, rdbPutString.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, responseData.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        // Perform the update request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            cerr << "Failed to update run record: " << curl_easy_strerror(res) << endl;
        } else {
            cout << "Successfully added new key-value pair" << endl;
        }
      } catch (const json::parse_error& e) {
        cerr << "Run " << irun << ": Failed to parse JSON response: " << e.what() 
        << "->" << responseData << "<-"
        << endl;
      } catch (const std::exception& e) {
        cerr << "Run " << irun << ": Error processing JSON: " << e.what() << endl;
      }
    }
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }
}

// ----------------------------------------------------------------------
// -- delete field in runRecord and upload modified record to RDB
void rdbMode5(int irun, string key, bool debug) {
  bool DBX(false);
  // Validate input parameters
  if (key == "unset") {
    cerr << "Error: Key must be set. Current value: " << key << endl;
    return;
  }

  string responseData;
  CURL* curl = curl_easy_init();
  if (curl) {
    std::string url = rdbGetString + "/" + std::to_string(irun) + "/json";
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cdbRestWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
    
    // Set headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    if (debug) {
      cout << "Making request to: " << url << endl;
    } else {
      CURLcode res = curl_easy_perform(curl);
      if (res != CURLE_OK) {
        cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return;
      }
      
      if (DBX) cout << "responseData: ->" << responseData << "<-" << endl;
      try {
        // Parse the JSON response
        json j = json::parse(responseData);
        
        // Split the key into parts using dot notation
        vector<string> keyParts;
        stringstream keyStream(key);
        string part;
        while (getline(keyStream, part, '.')) {
          keyParts.push_back(part);
        }
        
        // Navigate to the nested key
        json* current = &j;
        for (size_t i = 0; i < keyParts.size() - 1; ++i) {
          if (current->contains(keyParts[i])) {
            current = &((*current)[keyParts[i]]);
          } else {
            cerr << "Parent object not found: " << keyParts[i] << " in path " << key << endl;
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return;
          }
        }
        
        // Get the final key part
        string finalKey = keyParts.back();
        
        // Check if the key exists
        if (!current->contains(finalKey)) {
            cout << "Key '" << key << "' does not exist, nothing to delete" << endl;
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return;
        }
        
        // Store the old value for logging
        string oldValue = (*current)[finalKey].dump();
        
        // Delete the key
        current->erase(finalKey);
        
        // Convert back to string
        responseData = j.dump(2);  // Pretty print with 2-space indentation
        
        cout << "Deleted key-value pair: " << key << " = " << oldValue << endl;
        if (DBX) cout << "responseData: ->" << responseData << "<-" << endl;

        // Send the updated JSON back to the server
        curl_easy_reset(curl);
        curl_easy_setopt(curl, CURLOPT_URL, rdbPutString.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, responseData.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        // Perform the update request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            cerr << "Failed to update run record: " << curl_easy_strerror(res) << endl;
        } else {
            cout << "Successfully deleted key-value pair" << endl;
        }
      } catch (const json::parse_error& e) {
        cerr << "Failed to parse JSON response: " << e.what() << endl;
      } catch (const std::exception& e) {
        cerr << "Error processing JSON: " << e.what() << endl;
      }
    }
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }
}

// ----------------------------------------------------------------------
// -- Synchronize RunInfo.Class with BOR.Run Class
void rdbMode6(int irun, bool debug) {
  bool DBX(false);
  string responseData;
  CURL* curl = curl_easy_init();
  if (curl) {
    std::string url = rdbGetString + "/" + std::to_string(irun) + "/json";
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cdbRestWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
    
    // Set headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    if (debug) {
      cout << "Making request to: " << url << endl;
    } else {
      CURLcode res = curl_easy_perform(curl);
      if (res != CURLE_OK) {
        cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return;
      }
      
      if (DBX) cout << "responseData: ->" << responseData << "<-" << endl;
      try {
        // Parse the JSON response
        json j = json::parse(responseData);
        
        // Find the RunInfo attribute
        json* runInfo = nullptr;
        if (j.contains("Attributes")) {
          for (auto& attr : j["Attributes"]) {
            if (attr.contains("RunInfo")) {
              runInfo = &attr["RunInfo"];
              break;
            }
          }
        }
        
        // Get the current values
        string runInfoClass = "unset";
        string borRunClass = "unset";
        
        if (runInfo && runInfo->contains("Class")) {
          runInfoClass = (*runInfo)["Class"].get<string>();
        }
        
        if (j.contains("BOR") && j["BOR"].contains("Run Class")) {
          borRunClass = j["BOR"]["Run Class"].get<string>();
        }
        
        bool needsUpdate = false;
        string targetValue;
        string targetKey;
        
        // Determine which value to use and where to put it
        if (runInfoClass != "unset") {
          // If RunInfo.Class exists, use it to update BOR.Run Class
          if (borRunClass != runInfoClass) {
            needsUpdate = true;
            targetValue = runInfoClass;
            targetKey = "BOR.Run Class";
          }
        } else if (borRunClass != "unset") {
          // If RunInfo.Class is unset but BOR.Run Class exists, use it to update RunInfo.Class
          if (runInfo) {
            needsUpdate = true;
            targetValue = borRunClass;
            targetKey = "Attributes[].RunInfo.Class";
          }
        }
        
        if (needsUpdate) {
          // Update the appropriate field
          if (targetKey == "BOR.Run Class") {
            j["BOR"]["Run Class"] = targetValue;
          } else {
            // Find and update the RunInfo attribute
            for (auto& attr : j["Attributes"]) {
              if (attr.contains("RunInfo")) {
                attr["RunInfo"]["Class"] = targetValue;
                break;
              }
            }
          }
          
          // Convert back to string
          responseData = j.dump(2);  // Pretty print with 2-space indentation
          
          cout << "Run " << irun << ": Synchronized " << targetKey << " to " << targetValue << endl;
          if (DBX) cout << "responseData: ->" << responseData << "<-" << endl;

          // Send the updated JSON back to the server
          curl_easy_reset(curl);
          curl_easy_setopt(curl, CURLOPT_URL, rdbPutString.c_str());
          curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
          curl_easy_setopt(curl, CURLOPT_POSTFIELDS, responseData.c_str());
          curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
          
          // Perform the update request
          res = curl_easy_perform(curl);
          if (res != CURLE_OK) {
            cerr << "Failed to update run record: " << curl_easy_strerror(res) << endl;
          } else {
            cout << "Successfully synchronized class values" << endl;
          }
        } else {
          cout << "Run " << irun << ": No synchronization needed" << endl;
        }
      } catch (const json::parse_error& e) {
        cerr << "Run " << irun << ": Failed to parse JSON response: " << e.what() 
             << "->" << responseData << "<-"
             << endl;
      } catch (const std::exception& e) {
        cerr << "Run " << irun << ": Error processing JSON: " << e.what() << endl;
      }
    }
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }
}

// ----------------------------------------------------------------------
// -- Capitalize RunInfo.Class and transform "calib" to "Calibration"
void rdbMode7(int irun, bool debug) {
  bool DBX(false);
  string responseData;
  CURL* curl = curl_easy_init();
  if (curl) {
    std::string url = rdbGetString + "/" + std::to_string(irun) + "/json";
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cdbRestWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
    
    // Set headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    if (debug) {
      cout << "Making request to: " << url << endl;
    } else {
      CURLcode res = curl_easy_perform(curl);
      if (res != CURLE_OK) {
        cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return;
      }
      
      if (DBX) cout << "responseData: ->" << responseData << "<-" << endl;
      try {
        // Parse the JSON response
        json j = json::parse(responseData);
        
        if (!j.contains("Attributes")) {
          cout << "Run " << irun << ": No Attributes found" << endl;
          curl_slist_free_all(headers);
          curl_easy_cleanup(curl);
          return;
        }

        bool anyChanges = false;
        int runInfoCount = 0;
        
        // Process all RunInfo instances in Attributes
        for (auto& attr : j["Attributes"]) {
          if (attr.contains("RunInfo") && attr["RunInfo"].contains("Class")) {
            runInfoCount++;
            string currentClass = attr["RunInfo"]["Class"].get<string>();
            string oldClass = currentClass;  // Store for logging
            
            // Transform the value
            if (currentClass == "calib") {
              currentClass = "Calibration";
            } else {
              // Capitalize the first letter and lowercase the rest
              if (!currentClass.empty()) {
                currentClass[0] = toupper(currentClass[0]);
                for (size_t i = 1; i < currentClass.length(); ++i) {
                  currentClass[i] = tolower(currentClass[i]);
                }
              }
            }
            
            // Update if the value has changed
            if (currentClass != oldClass) {
              attr["RunInfo"]["Class"] = currentClass;
              anyChanges = true;
              cout << "Run " << irun << ": Transformed RunInfo[" << runInfoCount 
                   << "].Class from '" << oldClass << "' to '" << currentClass << "'" << endl;
            } else {
              cout << "Run " << irun << ": RunInfo[" << runInfoCount 
                   << "].Class already in correct format: '" << currentClass << "'" << endl;
            }
          }
        }
        
        if (runInfoCount == 0) {
          cout << "Run " << irun << ": No RunInfo.Class found in any attribute" << endl;
          curl_slist_free_all(headers);
          curl_easy_cleanup(curl);
          return;
        }
        
        // Only update the server if any changes were made
        if (anyChanges) {
          // Convert back to string
          responseData = j.dump(2);  // Pretty print with 2-space indentation
          
          if (DBX) cout << "responseData: ->" << responseData << "<-" << endl;

          // Send the updated JSON back to the server
          curl_easy_reset(curl);
          curl_easy_setopt(curl, CURLOPT_URL, rdbPutString.c_str());
          curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
          curl_easy_setopt(curl, CURLOPT_POSTFIELDS, responseData.c_str());
          curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
          
          // Perform the update request
          res = curl_easy_perform(curl);
          if (res != CURLE_OK) {
            cerr << "Failed to update run record: " << curl_easy_strerror(res) << endl;
          } else {
            cout << "Successfully updated " << runInfoCount << " RunInfo.Class values" << endl;
          }
        } else {
          cout << "Run " << irun << ": No changes needed for any RunInfo.Class values" << endl;
        }
      } catch (const json::parse_error& e) {
        cerr << "Run " << irun << ": Failed to parse JSON response: " << e.what() 
             << "->" << responseData << "<-"
             << endl;
      } catch (const std::exception& e) {
        cerr << "Run " << irun << ": Error processing JSON: " << e.what() << endl;
      }
    }
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }
}

// ----------------------------------------------------------------------
// -- set RunInfo.Significant to value
void rdbMode8(int irun, string value, bool debug) {
  bool DBX(false);
  // Validate input parameters
  if (value == "unset") {
    cerr << "Error: Value must be set. Current value: " << value << endl;
    return;
  }

  // Convert value to boolean
  string valueLower = value;
  transform(valueLower.begin(), valueLower.end(), valueLower.begin(), ::tolower);

  string responseData;
  CURL* curl = curl_easy_init();
  if (curl) {
    std::string url = rdbGetString + "/" + std::to_string(irun) + "/json";
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cdbRestWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
    
    // Set headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    if (debug) {
      cout << "Making request to: " << url << endl;
    } else {
      CURLcode res = curl_easy_perform(curl);
      if (res != CURLE_OK) {
        cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return;
      }
      
      if (DBX) cout << "responseData: ->" << responseData << "<-" << endl;
      try {
        // Parse the JSON response
        json j = json::parse(responseData);
        
        if (!j.contains("Attributes")) {
          cout << "Run " << irun << ": No Attributes found" << endl;
          curl_slist_free_all(headers);
          curl_easy_cleanup(curl);
          return;
        }

        bool anyChanges = false;
        int runInfoCount = 0;
        
        // -- Process all RunInfo instances in Attributes and change IFF unset (for bulk processing)
        for (auto& attr : j["Attributes"]) {
          if (attr.contains("RunInfo")) {
            runInfoCount++;
            if (attr["RunInfo"]["Significant"] == "unset") {
              attr["RunInfo"]["Significant"] = valueLower;
              anyChanges = true;
              cout << "Run " << irun << ": Set RunInfo[" << runInfoCount << "].Significant to " << valueLower << endl;
            }
          }
        }
        
        if (runInfoCount == 0) {
          cout << "Run " << irun << ": No RunInfo found in any attribute" << endl;
          curl_slist_free_all(headers);
          curl_easy_cleanup(curl);
          return;
        }
        
        // Only update the server if any changes were made
        if (anyChanges) {
          // Convert back to string
          responseData = j.dump(2);  // Pretty print with 2-space indentation
          
          if (DBX) cout << "responseData: ->" << responseData << "<-" << endl;

          // Send the updated JSON back to the server
          curl_easy_reset(curl);
          curl_easy_setopt(curl, CURLOPT_URL, rdbPutString.c_str());
          curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
          curl_easy_setopt(curl, CURLOPT_POSTFIELDS, responseData.c_str());
          curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
          
          // Perform the update request
          res = curl_easy_perform(curl);
          if (res != CURLE_OK) {
            cerr << "Failed to update run record: " << curl_easy_strerror(res) << endl;
          } else {
            cout << "Successfully updated " << runInfoCount << " RunInfo.Significant values" << endl;
          }
        } else {
          cout << "Run " << irun << ": No changes needed for any RunInfo.Significant values" << endl;
        }
      } catch (const json::parse_error& e) {
        cerr << "Run " << irun << ": Failed to parse JSON response: " << e.what() 
             << "->" << responseData << "<-"
             << endl;
      } catch (const std::exception& e) {
        cerr << "Run " << irun << ": Error processing JSON: " << e.what() << endl;
      }
    }
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }
}

// ----------------------------------------------------------------------
// -- Remove resources with specific description
void rdbMode9(int irun, string description, bool debug) {
  bool DBX(false);
  // Validate input parameters
  if (description == "unset") {
    cerr << "Error: Description must be set. Current value: " << description << endl;
    return;
  }

  string responseData;
  CURL* curl = curl_easy_init();
  if (curl) {
    std::string url = rdbGetString + "/" + std::to_string(irun) + "/json";
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cdbRestWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
    
    // Set headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    if (debug) {
      cout << "Making request to: " << url << endl;
    } else {
      CURLcode res = curl_easy_perform(curl);
      if (res != CURLE_OK) {
        cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return;
      }
      
      if (DBX) cout << "responseData: ->" << responseData << "<-" << endl;
      try {
        // Parse the JSON response
        json j = json::parse(responseData);
        
        // Check if Resources array exists
        if (!j.contains("Resources")) {
          cout << "Run " << irun << ": No Resources array found" << endl;
          curl_slist_free_all(headers);
          curl_easy_cleanup(curl);
          return;
        }

        // Count resources before removal
        size_t initialCount = j["Resources"].size();
        
        // Remove resources with matching description
        j["Resources"].erase(
          std::remove_if(j["Resources"].begin(), j["Resources"].end(),
            [&description](const json& resource) {
              return resource.contains("description") && 
                     resource["description"].get<string>() == description;
            }
          ),
          j["Resources"].end()
        );

        // Count resources after removal
        size_t finalCount = j["Resources"].size();
        size_t removedCount = initialCount - finalCount;

        if (removedCount > 0) {
          // Add to history
          string currentdate = getCurrentDateTime();
          json historyEntry = {
            {"date", currentdate},
            {"comment", "Removed " + to_string(removedCount) + " resources with description: " + description}
          };

          if (j.contains("History")) {
            j["History"].push_back(historyEntry);
          } else {
            j["History"] = json::array({historyEntry});
          }

          // Convert back to string
          responseData = j.dump(2);  // Pretty print with 2-space indentation
          
          cout << "Run " << irun << ": Removed " << removedCount << " resources with description '" << description << "'" << endl;
          if (DBX) cout << "responseData: ->" << responseData << "<-" << endl;

          // Send the updated JSON back to the server
          curl_easy_reset(curl);
          curl_easy_setopt(curl, CURLOPT_URL, rdbPutString.c_str());
          curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
          curl_easy_setopt(curl, CURLOPT_POSTFIELDS, responseData.c_str());
          curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
          
          // Perform the update request
          res = curl_easy_perform(curl);
          if (res != CURLE_OK) {
            cerr << "Failed to update run record: " << curl_easy_strerror(res) << endl;
          } else {
            cout << "Successfully removed resources" << endl;
          }
        } else {
          cout << "Run " << irun << ": No resources found with description '" << description << "'" << endl;
        }
      } catch (const json::parse_error& e) {
        cerr << "Run " << irun << ": Failed to parse JSON response: " << e.what() 
             << "->" << responseData << "<-"
             << endl;
      } catch (const std::exception& e) {
        cerr << "Run " << irun << ": Error processing JSON: " << e.what() << endl;
      }
    }
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }
}

// Helper function to get current date/time in the required format
string getCurrentDateTime() {
  auto now = chrono::system_clock::now();
  auto now_time_t = chrono::system_clock::to_time_t(now);
  auto now_tm = localtime(&now_time_t);
  
  stringstream ss;
  ss << now_tm->tm_year + 1900 << "/"
     << setw(2) << setfill('0') << now_tm->tm_mon + 1 << "/"
     << setw(2) << setfill('0') << now_tm->tm_mday << " "
     << setw(2) << setfill('0') << now_tm->tm_hour << ":"
     << setw(2) << setfill('0') << now_tm->tm_min << ":"
     << setw(2) << setfill('0') << now_tm->tm_sec;
  
  return ss.str();
}

