#ifndef DATAQUALITY_h
#define DATAQUALITY_h

#include <string>
#include <iostream>
#include <regex>

struct DataQuality {
    int mu3e;
    int beam;
    int vertex;
    int pixel;
    int tiles;
    int fibres;
    int calibration;
    int links;
    std::string version;

    DataQuality();
    std::string extractValue(const std::string& json, const std::string& key);
    size_t parse(const std::string &jsonString, size_t startPos = 0);
 
    std::string print() const;
    std::string json() const;
};

#endif 