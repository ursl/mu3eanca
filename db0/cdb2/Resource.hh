#ifndef RESOURCE_h
#define RESOURCE_h

#include <string>
#include <iostream>
#include <regex>

class Resource {
public:
    Resource();
    ~Resource();

    // -- query functions
    std::string getName() const;
    std::string getType() const;
    std::string getStatus() const;
    std::string getComments() const;
    
    // -- print functions
    void print() const;
    std::string printSummary() const;
    std::string json() const;
    
    // -- setters
    void setName(const std::string& name);
    void setType(const std::string& type);
    void setStatus(const std::string& status);
    void setComments(const std::string& comments);
    void fillFromJson(const std::string& jsonString);

    // -- JSON parsing
    std::string extractValue(const std::string& json, const std::string& key);
    size_t parse(const std::string &jsonString, size_t startPos = 0);

private:
    std::string fName;      // Name/identifier of the resource
    std::string fType;      // Type of resource (e.g., "hardware", "software", "service")
    std::string fStatus;    // Current status of the resource
    std::string fComments;  // Additional comments or notes
    std::string fJSONString; // Raw JSON string if loaded from JSON
};

#endif 