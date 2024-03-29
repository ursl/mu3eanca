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
void replaceAll(std::string &s, const std::string &from, const std::string &to);
void cleanupString(std::string &);
bool bothAreSpaces(char lhs, char rhs);
void rmSubString(std::string &sinput, const std::string &remove);
void rmPath(std::string &sInput);


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
