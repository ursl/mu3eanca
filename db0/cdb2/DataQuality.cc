#include "DataQuality.hh"
#include <sstream>
#include <regex>
#include <iostream>

using namespace std;

// ----------------------------------------------------------------------
DataQuality::DataQuality() :
  mu3e(0),
  beam(0),
  vertex(0),
  pixel(0),
  tiles(0),
  fibres(0),
  calibration(0),
  links(0),
  version("unset") {
}

// ----------------------------------------------------------------------
string DataQuality::print() const {
  return "DataQuality: " + to_string(mu3e) + " " + to_string(beam) + " " + to_string(vertex) + " " + to_string(pixel) + " " + to_string(tiles) + " " + to_string(fibres) + " " + to_string(calibration) + " " + to_string(links) + " " + version;
}

// ----------------------------------------------------------------------
string DataQuality::json() const {
  string sjson = "{\"DataQuality\":{";
  sjson += "\"mu3e\":\"" + to_string(mu3e) + "\", ";
  sjson += "\"beam\":\"" + to_string(beam) + "\", ";
  sjson += "\"vertex\":\"" + to_string(vertex) + "\", ";
  sjson += "\"pixel\":\"" + to_string(pixel) + "\", ";
  sjson += "\"tiles\":\"" + to_string(tiles) + "\", ";
  sjson += "\"fibres\":\"" + to_string(fibres) + "\", ";
  sjson += "\"calibration\":\"" + to_string(calibration) + "\", ";
  sjson += "\"links\":\"" + to_string(links) + "\", ";
  sjson += "\"version\":\"" + version + "\"}}";
  return sjson;
}

// ----------------------------------------------------------------------
string DataQuality::extractValue(const string& json, const string& key) {
    // -- try quoted string first: "key":"value"
    string pattern = "\"" + key + "\":\"([^\"]*)\"";
    regex re(pattern);
    smatch match;
    if (regex_search(json, match, re) && match.size() > 1) {
        return match[1].str();
    }
    // -- try numeric value: "key":123 or "key":-123
    pattern = "\"" + key + "\":(-?\\d+)";
    re = regex(pattern);
    if (regex_search(json, match, re) && match.size() > 1) {
        return match[1].str();
    }
    return "unset";
}

// ----------------------------------------------------------------------
size_t DataQuality::parse(const string &jsonString, size_t startPos) {
    try {
        // Find the first occurrence of "RunInfo" starting from startPos
        size_t start = jsonString.find("\"DataQuality\"", startPos);
        if (start == string::npos) {
            return string::npos;
        }

        // Find the opening brace of the RunInfo object
        start = jsonString.find("{", start);
        if (start == string::npos) {
            return string::npos;
        }

        // Find the matching closing brace
        int braceCount = 1;
        size_t end = start + 1;
        while (braceCount > 0 && end < jsonString.length()) {
            if (jsonString[end] == '{') braceCount++;
            if (jsonString[end] == '}') braceCount--;
            end++;
        }
        if (braceCount != 0) {
            return string::npos;
        }

        // Extract the DataQuality JSON object
        string dqJson = jsonString.substr(start, end - start);

        // Extract values using regex, handling "unset" values
        string val;
        val = extractValue(dqJson, "mu3e");
        mu3e = (val != "unset") ? stoi(val) : 0;
        val = extractValue(dqJson, "beam");
        beam = (val != "unset") ? stoi(val) : 0;
        val = extractValue(dqJson, "vertex");
        vertex = (val != "unset") ? stoi(val) : 0;
        val = extractValue(dqJson, "pixel");
        pixel = (val != "unset") ? stoi(val) : 0;
        val = extractValue(dqJson, "tiles");
        tiles = (val != "unset") ? stoi(val) : 0;
        val = extractValue(dqJson, "fibres");
        fibres = (val != "unset") ? stoi(val) : 0;
        val = extractValue(dqJson, "calibration");
        calibration = (val != "unset") ? stoi(val) : 0;
        // -- goodLinks -> links
        val = extractValue(dqJson, "links");
        if (val == "unset") {
          val = extractValue(dqJson, "goodLinks");
          links = (val != "unset") ? stoi(val) : 0;
        } else {
          links = stoi(val);
        }
        version = extractValue(dqJson, "version");

        return end;  // Return position after the parsed DataQuality
    } catch (const exception& e) {
        cout << "Error parsing DataQuality JSON: " << e.what() << endl;
        return string::npos;
    }
}
