#include "RunInfo.hh"
#include <sstream>
#include <regex>
#include <iostream>

using namespace std;
// ----------------------------------------------------------------------
RunInfo::RunInfo() :
        Class("unset"),
        significant("unset"),
        comments("unset"),
        components("unset"),
        componentsOut("unset"),
        midasVersion("unset"),
        midasGitRevision("unset"),
        daqVersion("unset"),
        daqGitRevision("unset"),
        vtxVersion("unset"),
        vtxGitRevision("unset"),
        pixVersion("unset"),
        pixGitRevision("unset"),
        tilVersion("unset"),
        tilGitRevision("unset"),
        fibVersion("unset"),
        fibGitRevision("unset"),
        version("unset") {
  }

// ----------------------------------------------------------------------
string RunInfo::print() const {
    return "RunInfo: " + Class + " " + significant + " " + comments ;
}

// ----------------------------------------------------------------------
string RunInfo::json() const {
    string sjson = "{\"RunInfo\":{";
    sjson += "\"Class\":\"" + Class + "\", ";
    sjson += "\"Significant\":\"" + significant + "\", ";
    sjson += "\"Comments\":\"" + comments + "\", ";
    sjson += "\"Components\":\"" + components + "\", ";
    sjson += "\"ComponentsOut\":\"" + componentsOut + "\", ";
    sjson += "\"MidasVersion\":\"" + midasVersion + "\", ";
    sjson += "\"MidasGitRevision\":\"" + midasGitRevision + "\", ";
    sjson += "\"DAQVersion\":\"" + daqVersion + "\", ";
    sjson += "\"DAQGitRevision\":\"" + daqGitRevision + "\", ";
    sjson += "\"VtxVersion\":\"" + vtxVersion + "\", ";
    sjson += "\"VtxGitRevision\":\"" + vtxGitRevision + "\", ";
    sjson += "\"PixVersion\":\"" + pixVersion + "\", ";
    sjson += "\"PixGitRevision\":\"" + pixGitRevision + "\", ";
    sjson += "\"TilVersion\":\"" + tilVersion + "\", "; 
    sjson += "\"TilGitRevision\":\"" + tilGitRevision + "\", "; 
    sjson += "\"FibVersion\":\"" + fibVersion + "\", ";
    sjson += "\"FibGitRevision\":\"" + fibGitRevision + "\", ";
    sjson += "\"Version\":\"" + version + "\"}}";
    return sjson;
}

// ----------------------------------------------------------------------
string RunInfo::extractValue(const string& json, const string& key) {
    string pattern = "\"" + key + "\":\"([^\"]*)\"";
    regex re(pattern);
    smatch match;
    if (regex_search(json, match, re) && match.size() > 1) {
        return match[1].str();
    }
    return "unset";
}

// ----------------------------------------------------------------------
size_t RunInfo::parse(const string &jsonString, size_t startPos) {
    try {
        // Find the first occurrence of "RunInfo" starting from startPos
        size_t start = jsonString.find("\"RunInfo\"", startPos);
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

        // Extract the RunInfo JSON object
        string runInfoJson = jsonString.substr(start, end - start);

        // Extract values using regex
        Class = extractValue(runInfoJson, "Class");
        significant = extractValue(runInfoJson, "Significant");
        comments = extractValue(runInfoJson, "Comments");
        components = extractValue(runInfoJson, "Components");
        componentsOut = extractValue(runInfoJson, "ComponentsOut");
        midasVersion = extractValue(runInfoJson, "MidasVersion");
        midasGitRevision = extractValue(runInfoJson, "MidasGitRevision");
        daqVersion = extractValue(runInfoJson, "DAQVersion");
        daqGitRevision = extractValue(runInfoJson, "DAQGitRevision");
        vtxVersion = extractValue(runInfoJson, "VtxVersion");
        vtxGitRevision = extractValue(runInfoJson, "VtxGitRevision");
        pixVersion = extractValue(runInfoJson, "PixVersion");
        pixGitRevision = extractValue(runInfoJson, "PixGitRevision");
        tilVersion = extractValue(runInfoJson, "TilVersion");
        tilGitRevision = extractValue(runInfoJson, "TilGitRevision");
        fibVersion = extractValue(runInfoJson, "FibVersion");
        fibGitRevision = extractValue(runInfoJson, "FibGitRevision");
        version = extractValue(runInfoJson, "Version");

        return end;  // Return position after the parsed RunInfo
    } catch (const exception& e) {
        cout << "Error parsing RunInfo JSON: " << e.what() << endl;
        return string::npos;
    }
}


