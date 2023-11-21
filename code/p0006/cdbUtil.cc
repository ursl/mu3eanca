#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <unistd.h>
#include <cstring>

#include "cdbUtil.hh"

using namespace std;


// ----------------------------------------------------------------------
int blob2Int(std::array<char,8> v) {
  char data[8] = {0}; 
  for (int i = 0; i < 8; ++i) data[i] = v[i];
  int a(0);
  memcpy(&a, data, sizeof a);
  return a;
}


// ----------------------------------------------------------------------
unsigned int blob2UnsignedInt(std::array<char,8> v) {
  char data[8] = {0}; 
  for (int i = 0; i < 8; ++i) data[i] = v[i];
  unsigned int a(0);
  memcpy(&a, data, sizeof a);
  return a;
}


// ----------------------------------------------------------------------
double blob2Double(std::array<char,8> v) {
  char data[8] = {0}; 
  for (int i = 0; i < 8; ++i) data[i] = v[i];
  double a(0.0);
  memcpy(&a, data, sizeof a);
  return a;
}


// ----------------------------------------------------------------------
std::array<char,8> int2Blob(int a) {
  char data[8] = {0}; 
  memcpy(data, &a, sizeof a);
  array<char,8> v; 
  for (int i = 0; i < 8; ++i) v[i] = data[i]; 
  return v;
}

// ----------------------------------------------------------------------
std::array<char,8> uint2Blob(unsigned int a) {
  char data[8] = {0}; 
  memcpy(data, &a, sizeof a);
  array<char,8> v; 
  for (int i = 0; i < 8; ++i) v[i] = data[i]; 
  return v;
}


// ----------------------------------------------------------------------
std::array<char,8> double2Blob(double a) {
  char data[8] = {0}; 
  memcpy(data, &a, sizeof a);
  array<char,8> v; 
  for (int i = 0; i < 8; ++i) v[i] = data[i]; 
  return v;
}


// ----------------------------------------------------------------------
std::array<char,8> getData(vector<char>::iterator &it) {
  array<char,8> v;
  for (unsigned int i = 0; i < 8; ++i) {
    v[i] = *it;
    ++it;
  }
  return v;
}


// ----------------------------------------------------------------------
string dumpArray(std::array<char,8> v) {
  stringstream sstr;
  for (auto it: v) sstr << it; 
  return sstr.str();
}


// ----------------------------------------------------------------------
void printArray(ofstream &OS, std::array<char,8> v) {
  for (auto it: v) OS << it; 
}


// ----------------------------------------------------------------------
void replaceAll(string &str, const string &from, const string &to) {
  if (from.empty()) return;
  size_t start_pos = 0;
  while((start_pos = str.find(from, start_pos)) != string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
  }
}


// ----------------------------------------------------------------------
void cleanupString(string &s) {
  replaceAll(s, "\t", " ");
  string::size_type s1 = s.find("#");
  if (string::npos != s1) s.erase(s1);
  if (0 == s.length()) return;
  string::iterator new_end = unique(s.begin(), s.end(), bothAreSpaces);
  s.erase(new_end, s.end());
  if (s.substr(0, 1) == string(" ")) s.erase(0, 1);
  if (s.substr(s.length()-1, 1) == string(" ")) s.erase(s.length()-1, 1);
}


// ----------------------------------------------------------------------
bool bothAreSpaces(char lhs, char rhs) {
  return (lhs == rhs) && (lhs == ' ');
}


// ----------------------------------------------------------------------=
void rmSubString(string &sInput, const string &sub) {
  string::size_type foundpos = sInput.find(sub);
  if (foundpos != string::npos)  {
    sInput.erase(sInput.begin() + foundpos, sInput.begin() + foundpos + sub.length());
  }
}

// ----------------------------------------------------------------------=
void rmPath(string &sInput) {
  string::size_type foundpos = sInput.find("/");
  while(string::npos != foundpos) {
    sInput.erase(sInput.begin(), sInput.begin() + foundpos + 1);
    foundpos = sInput.find("/");
  }
}


// ----------------------------------------------------------------------
vector<string> split(const string &s, char delim) {
  vector<string> elems;
  split(s, delim, elems);
  return elems;
}


// ----------------------------------------------------------------------
void /*vector<string>&*/ split(const string &s, char delim, vector<string> &elems) {
  stringstream ss(s);
  string item;
  while (getline(ss, item, delim)) {
    elems.push_back(item);
  }
  //  return elems;
}


