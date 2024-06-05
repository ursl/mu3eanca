#include "cdbUtil.hh"

#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <unistd.h>
#include <cstring>


using namespace std;


// ----------------------------------------------------------------------
void rtrim(string &s) {
  s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
    return !isspace(ch);
  }).base(), s.end());
}


// ----------------------------------------------------------------------
void ltrim(string &s) {
  s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch) {
    return !isspace(ch);
  }));
}


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
string jsFormat(vector<string> v) {
  stringstream sstr;
  sstr << "[ ";
  for (unsigned int i = 0; i < v.size(); ++i) {
    if (i + 1 < v.size()) {
      sstr << "\"" << v[i] << "\", ";
    } else {
      sstr << "\"" << v[i] << "\"";
    }
  }
  sstr << " ]";
  return sstr.str();
}


// ----------------------------------------------------------------------
string jsFormat(vector<int> v) {
  stringstream sstr;
  sstr << "[ ";
  for (unsigned int i = 0; i < v.size(); ++i) {
    if (i + 1 < v.size()) {
      sstr << v[i] << ", ";
    } else {
      sstr << v[i];
    }
  }
  sstr << " ]";
  return sstr.str();
}

// ----------------------------------------------------------------------
//std::string jsFormat(std::vector<int>);
//std::string jsFormat(std::vector<double>);



// ----------------------------------------------------------------------
void replaceAll(string &str, const string &from, const string &to, size_t start_pos) {
  if (from.empty()) return;
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


// ----------------------------------------------------------------------
string jsonGetValue(string& jstring, string key) {
  string::size_type s0 = jstring.find(key);
  s0 = jstring.find(":", s0+key.length());
  string::size_type s1 = jstring.find(",", s0);
  string::size_type s2 = jstring.find("}", s0);
  string result("");
  if (string::npos != s1) {
    result = jstring.substr(s0+1, s1-s0-1);
  } else {
    result = jstring.substr(s0+1, s2-s0);
  }
  ltrim(result);
  rtrim(result);
  replaceAll(result, "\"", "");
  return result;
}


// ----------------------------------------------------------------------
string jsonGetString(string& jstring, string key) {
  const bool DBX(false);
  if (DBX) cout << "jsonGetString key = " << key << endl;
  string::size_type s0 = jstring.find(key);
  s0 = jstring.find(":", s0+key.length());
  s0 = jstring.find("\"", s0+1);
  string::size_type s1 = jstring.find(",", s0);
  string::size_type s2 = jstring.find("}", s0);
  if (DBX)   cout << "s0: " << s0 << " s1: " << s1 << " s2: " << s2 << endl;
  string result("");
  if ((string::npos != s1) && (s1 < s2)) {
    result = jstring.substr(s0, s1-s0);
  } else {
    result = jstring.substr(s0, s2-s0);
  }
  if (DBX) cout << "result ->" << result << "<-" << endl;
  ltrim(result);
  rtrim(result);
  replaceAll(result, "\"", "");
  if (DBX) cout << "result ->" << result << "<-" << endl;
  return result;
}


// ----------------------------------------------------------------------
string jsonGetString(string& jstring, vector<string> keys) {
  const bool DBX(false);
  if (DBX) {
    cout << "jsonGetString keys = ";
    for (auto it: keys) cout << it << ",";
    cout << endl;
  }
  bool found(false);
  string::size_type s0(0);
  for (unsigned int i = 0; i < keys.size(); ++i) {
    string key = string("\"") + keys[i] + string("\"");
    s0 = jstring.find(key, s0);
    if (string::npos == s0) break;
    s0 = jstring.find(":", s0+key.length());
  }
  s0 = jstring.find("\"", s0+1);
  string::size_type s1 = jstring.find(",", s0);
  string::size_type s2 = jstring.find("}", s0);
  if (DBX)   cout << "s0: " << s0 << " s1: " << s1 << " s2: " << s2 << endl;
  string result("");
  if ((string::npos != s1) && (s1 < s2)) {
    result = jstring.substr(s0, s1-s0);
  } else {
    result = jstring.substr(s0, s2-s0);
  }
  if (DBX) cout << "result ->" << result << "<-" << endl;
  ltrim(result);
  rtrim(result);
  replaceAll(result, "\"", "");
  if (DBX) cout << "result ->" << result << "<-" << endl;
  return result;
}


// ----------------------------------------------------------------------
string jsonGetValue(string& jstring, vector<string> keys) {
  const bool DBX(false);
  if (DBX) {
    cout << "jsonGetString keys = ";
    for (auto it: keys) cout << it << ",";
    cout << endl;
  }
  bool found(false);
  string::size_type s0(0);
  for (unsigned int i = 0; i < keys.size(); ++i) {
    string key = string("\"") + keys[i] + string("\"");
    s0 = jstring.find(key, s0);
    if (string::npos == s0) break;
  }
  s0 = jstring.find(":", s0+1);
  ++s0;
  string::size_type s1 = jstring.find(",", s0);
  string::size_type s2 = jstring.find("}", s0);
  if (DBX)   cout << "s0: " << s0 << " s1: " << s1 << " s2: " << s2 << endl;
  string result("");
  if ((string::npos != s1) && (s1 < s2)) {
    result = jstring.substr(s0, s1-s0);
  } else {
    result = jstring.substr(s0, s2-s0);
  }
  if (DBX) cout << "result ->" << result << "<-" << endl;
  ltrim(result);
  rtrim(result);
  replaceAll(result, "\"", "");
  if (DBX) cout << "result ->" << result << "<-" << endl;
  return result;
}


// ----------------------------------------------------------------------
string jsonGetCfgString(std::string& jstring, std::string key) {
  string::size_type s0 = jstring.find(key);
  s0 = jstring.find(":", s0);
  s0 = jstring.find("\"", s0);
  string result = jstring.substr(s0);
  ltrim(result);
  
  // -- there is another } to trim
  result.pop_back();
  replaceAll(result, "\"", "");
  return result;
}


// ----------------------------------------------------------------------
string jsonGetCfgStringEsc(std::string& jstring, std::string key) {
  string::size_type s0 = jstring.find(key);
  s0 = jstring.find(":", s0);
  s0 = jstring.find("\"", s0);
  string result = jstring.substr(s0);
  ltrim(result);
  
  // -- magic to form the resulting string
  result.pop_back(); // remove an end
  result.erase(0, 1); // remove front char
  replaceAll(result, "\\n", "\n"); // remove all escaped newlines
  replaceAll(result, "\\", ""); // remove all escaped double quotes
  result.pop_back();
  result.pop_back();
  return result;
}


// ----------------------------------------------------------------------
vector<string> jsonGetValueVector(string& jstring, string key) {
  vector<string> result;
  string::size_type s0(0);
  while (string::npos != s0) {
    s0 = jstring.find(key, s0);
    if (string::npos == s0) break;
    s0 = jstring.find(":", s0);
    if (string::npos == s0) break;
    s0 = jstring.find("\"", s0);
    if (string::npos == s0) break;
    string::size_type s1 = jstring.find(",", s0 + key.length() + 2);
    string::size_type s2 = jstring.find("}", s0 + key.length() + 2);
    string sresult;
    if (string::npos != s1) {
      sresult = jstring.substr(s0, s1-s0);
      s0 = s1;
    } else {
      sresult = jstring.substr(s0, s2-s0);
      s0 = s2;
    }
    ltrim(sresult);
    rtrim(sresult);
    replaceAll(sresult, "\"", "");
    result.push_back(sresult);
  }
  return result;
}


// ----------------------------------------------------------------------
string jsonGetVector(string& jstring, string key) {
  string::size_type s0 = jstring.find(key);
  s0 = jstring.find(":", s0);
  string::size_type s1 = jstring.find("[", s0);
  string::size_type s2 = jstring.find("]", s0);
  string result("");
  if (string::npos != s1 && string::npos != s2) {
    result = jstring.substr(s1+1, s2-s1-1);
  } else {
    cout << "jsonGetVector> parse error" << endl;
  }
  ltrim(result);
  rtrim(result);
  replaceAll(result, "\"", "");
  replaceAll(result, " ", "");
  return result;
}


// ----------------------------------------------------------------------
vector<string> jsonGetVectorVector(string& jstring, string key) {
  vector<string> result;
  string::size_type s0(0);
  while (string::npos != s0) {
    s0 = jstring.find(key);
    s0 = jstring.find(":", s0);
    string::size_type s1 = jstring.find("[", s0);
    string::size_type s2 = jstring.find("]", s0);
    string sresult("");
    if (string::npos != s1 && string::npos != s2) {
      sresult = jstring.substr(s1+1, s2-s1-1);
      s0 = s2;
    } else {
      cout << "jsonGetVector> parse error" << endl;
      s0 = string::npos;
    }
    ltrim(sresult);
    rtrim(sresult);
    replaceAll(sresult, "\"", "");
    replaceAll(sresult, " ", "");
    result.push_back(sresult);
  }
  return result;
}


// ----------------------------------------------------------------------
string timeStamp(int format) {
  time_t t;
  time(&t);
  
  // strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));
  struct timeval tv;
  gettimeofday(&tv, 0);
  
  tm *ltm = localtime(&t);
  int year  = 1900 + ltm->tm_year;
  int month = 1 + ltm->tm_mon;
  int day   = ltm->tm_mday;
  int hour  = ltm->tm_hour;
  int min   = ltm->tm_min;
  int sec   = ltm->tm_sec;
  std::stringstream result;
  if (0 == format) {
    result << year << "-"
           << std::setfill('0') << std::setw(2) << month << "-"
           << std::setfill('0') << std::setw(2) << day << " "
           << std::setfill('0') << std::setw(2) << hour << ":"
           << std::setfill('0') << std::setw(2) << min << ":"
           << std::setfill('0') << std::setw(2) << sec;
  }
  return result.str();
}
