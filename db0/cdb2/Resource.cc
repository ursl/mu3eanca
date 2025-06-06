#include "Resource.hh"
#include <iostream>
#include <sstream>
#include <regex>

using namespace std;

Resource::Resource() :
    fName(""),
    fType(""),
    fStatus(""),
    fComments(""),
    fJSONString("") {
}

Resource::~Resource() {
}

// -- query functions
string Resource::getName() const {
    return fName;
}

string Resource::getType() const {
    return fType;
}

string Resource::getStatus() const {
    return fStatus;
}

string Resource::getComments() const {
    return fComments;
}

// -- setters
void Resource::setName(const string& name) {
    fName = name;
}

void Resource::setType(const string& type) {
    fType = type;
}

void Resource::setStatus(const string& status) {
    fStatus = status;
}

void Resource::setComments(const string& comments) {
    fComments = comments;
}

// -- print functions
void Resource::print() const {
    cout << printSummary() << endl;
}

string Resource::printSummary() const {
    stringstream ss;
    ss << "Resource: " << fName << " (Type: " << fType << ")"
       << "\n  Status: " << fStatus;
    if (!fComments.empty()) {
        ss << "\n  Comments: " << fComments;
    }
    return ss.str();
}

string Resource::json() const {
    string sjson = "{\"Resource\":{";
    sjson += "\"name\":\"" + fName + "\", ";
    sjson += "\"type\":\"" + fType + "\", ";
    sjson += "\"status\":\"" + fStatus + "\", ";
    sjson += "\"comments\":\"" + fComments + "\"}}";
    return sjson;
}

string Resource::extractValue(const string& json, const string& key) {
    string pattern = "\"" + key + "\":\"([^\"]*)\"";
    regex re(pattern);
    smatch match;
    if (regex_search(json, match, re) && match.size() > 1) {
        return match[1].str();
    }
    return "unset";
}

size_t Resource::parse(const string &jsonString, size_t startPos) {
    try {
        // Find the first occurrence of "Resource" starting from startPos
        size_t start = jsonString.find("\"Resource\"", startPos);
        if (start == string::npos) {
            return string::npos;
        }

        // Find the opening brace of the Resource object
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

        // Extract the Resource JSON object
        string resourceJson = jsonString.substr(start, end - start);

        // Extract values using regex
        fName = extractValue(resourceJson, "name");
        fType = extractValue(resourceJson, "type");
        fStatus = extractValue(resourceJson, "status");
        fComments = extractValue(resourceJson, "comments");

        return end;  // Return position after the parsed Resource
    } catch (const exception& e) {
        cout << "Error parsing Resource JSON: " << e.what() << endl;
        return string::npos;
    }
}

void Resource::fillFromJson(const string& jsonString) {
    fJSONString = jsonString;
    parse(jsonString);
} 