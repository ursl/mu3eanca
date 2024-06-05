#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <array>
#include <unistd.h>
#include <cstdarg>
#include <fstream>

#include <string>

// ======================================================================
// string utilities
// ======================================================================
void replaceAll(std::string &s, const std::string &from, const std::string &to, size_t start_pos = 0);
void cleanupString(std::string &);
bool bothAreSpaces(char lhs, char rhs);
void rmSubString(std::string &sinput, const std::string &remove);
void rmPath(std::string &sInput);
void rtrim(std::string &s);
void ltrim(std::string &s);

std::string timeStamp(int format = 0);

// ======================================================================
// I/O for JSON files (to remove dependency on bsoncxx, drivers)
// ======================================================================
std::string jsFormat(std::vector<std::string>);
std::string jsFormat(std::vector<int>);
// -- get value index by a single key
std::string jsonGetString(std::string& jstring, std::string key);
// -- get value index by a vector of keys (e.g. "magnet,field,strength")
std::string jsonGetString(std::string& jstring, std::vector<std::string> keys);
// -- this is for the runlog where the values are not string, but directly numbers
std::string jsonGetValue(std::string& jstring, std::string key);
// -- this is for the run/detector.json, where the values are not strings, but directly numbers
std::string jsonGetValue(std::string& jstring, std::vector<std::string> keys);
// -- normal base64 encoded (from rest/mongodb API)
std::string jsonGetCfgString(std::string& jstring, std::string key);
// -- escaped " (\") embedded in string (from JSON API)
std::string jsonGetCfgStringEsc(std::string& jstring, std::string key);
std::string jsonGetVector(std::string& jstring, std::string key);
std::vector<std::string> jsonGetValueVector(std::string& jstring, std::string key);
std::vector<std::string> jsonGetVectorVector(std::string& jstring, std::string key);

// ======================================================================
// BLOB utilities
// ======================================================================
int blob2Int(std::array<char,8> v);
unsigned int blob2UnsignedInt(std::array<char,8> v);
double blob2Double(std::array<char,8> v);

std::array<char,8> int2Blob(int a);
std::array<char,8> uint2Blob(unsigned int a);
std::array<char,8> double2Blob(double a);

std::array<char,8> getData(std::vector<char>::iterator &it);

std::string dumpArray(std::array<char,8> v);
void printArray(std::ofstream &OS, std::array<char,8> v);


std::vector<std::string>  split(const std::string &s, char delim);
void split(const std::string &s, char delim, std::vector<std::string> &elems); // helper function for above


#endif
