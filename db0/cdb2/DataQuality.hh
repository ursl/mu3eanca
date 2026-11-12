#ifndef DATAQUALITY_h
#define DATAQUALITY_h

#include <string>
#include <iostream>
#include <regex>

// RDB stores each subsystem as a string. The first three values are already in the database:
//   "-1" unset, "0" bad, "1" good.
// "2" is data present: hits were seen, and nobody has marked the subsystem good or bad.
// "3" is no data: the histograms were read and contain no hits.
// Unset means the subsystem has not been judged. No data is a judgment.
// 1 stays "good", so existing records and the Good/Bad filters keep their meaning.
static const int DQ_UNSET   = -1;
static const int DQ_BAD     = 0;
static const int DQ_PRESENT = 2;
static const int DQ_NODATA  = 3;
static const int DQ_GOOD    = 1;

struct DataQuality {
    int mu3e;
    int beam;
    int vertex;
    int pixel;
    int tiles;
    int fibres;
    int calibration;
    int links;
    int deff;
    std::string version;

    DataQuality();
    std::string extractValue(const std::string& json, const std::string& key);
    size_t parse(const std::string &jsonString, size_t startPos = 0);
 
    std::string print() const;
    std::string json() const;
};

#endif 