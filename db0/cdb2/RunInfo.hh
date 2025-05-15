#ifndef RUNINFO_h
#define RUNINFO_h

#include <string>
#include <iostream>
#include <regex>

struct RunInfo {
  // -- uppercase to avoid conflict with reserved C++ word
    std::string Class;
    std::string significant;
    std::string comments;
    std::string components;
    std::string componentsOut;
    std::string midasVersion;
    std::string midasGitRevision;
    std::string daqVersion;
    std::string daqGitRevision;
    std::string vtxVersion;
    std::string vtxGitRevision;
    std::string pixVersion;
    std::string pixGitRevision;
    std::string tilVersion;
    std::string tilGitRevision;
    std::string fibVersion;
    std::string fibGitRevision;
    std::string version;

    RunInfo();
    std::string extractValue(const std::string& json, const std::string& key);
    size_t parse(const std::string &jsonString, size_t startPos = 0);
    std::string print() const;
    std::string json() const;
};

#endif 