#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

using namespace std;

vector<int> readRunnumbersFromFile(const string& filename) {
  vector<int> runnumbers;
  
  ifstream file(filename);
  if (!file.is_open()) {
    cerr << "Error: Could not open file " << filename << endl;
    return runnumbers;
  }
  
  // Read the entire file content
  string content;
  string line;
  while (getline(file, line)) {
    content += line;
  }
  file.close();
  
  // Filter out braces and square brackets
  string filtered;
  for (char c : content) {
    if (c != '{' && c != '}' && c != '[' && c != ']') {
      filtered += c;
    }
  }
  
  // Split by commas and convert to integers
  stringstream ss(filtered);
  string token;
  while (getline(ss, token, ',')) {
    // Trim whitespace
    token.erase(0, token.find_first_not_of(" \t\n\r"));
    token.erase(token.find_last_not_of(" \t\n\r") + 1);
    
    if (!token.empty()) {
      try {
        int runnum = stoi(token);
        runnumbers.push_back(runnum);
      } catch (const exception& e) {
        cerr << "Warning: Could not parse runnumber: " << token << endl;
      }
    }
  }
  
  return runnumbers;
}

// ----------------------------------------------------------------------
int main(int argc, char *argv[]) {

  string firstFile("unset"), secondFile("unset"), thirdFile("unset");
  // -- command line parsing 
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-a")) {
      firstFile = string(argv[++i]);
    }
    if (!strcmp(argv[i], "-b")) {
      secondFile = string(argv[++i]);
    }
    if (!strcmp(argv[i], "-c")) {
      thirdFile = string(argv[++i]);
    }
  }

  // -- read in the first file
  vector<int> firstRunnumbers = readRunnumbersFromFile(firstFile);
  vector<int> secondRunnumbers = readRunnumbersFromFile(secondFile);
  vector<int> thirdRunnumbers = readRunnumbersFromFile(thirdFile);

  // -- compare the vectors
  vector<int> firstOnlyRunnumbers;
  vector<int> secondOnlyRunnumbers;
  vector<int> secondAndFirstRunnumbers;
  vector<int> thirdOnlyRunnumbers;
  vector<int> allRunnumbers;
  vector<int> thirdAndFirstRunnumbers;
  vector<int> thirdAndSecondRunnumbers;
  for (int i = 0; i < firstRunnumbers.size(); i++) {
    if (find(secondRunnumbers.begin(), secondRunnumbers.end(), firstRunnumbers[i]) == secondRunnumbers.end()) {
      firstOnlyRunnumbers.push_back(firstRunnumbers[i]);
    } 
  }
  for (int i = 0; i < secondRunnumbers.size(); i++) {
    if (find(firstRunnumbers.begin(), firstRunnumbers.end(), secondRunnumbers[i]) == firstRunnumbers.end()) {
      secondOnlyRunnumbers.push_back(secondRunnumbers[i]);
    }
  }
  for (int i = 0; i < firstRunnumbers.size(); i++) {
    if (find(secondRunnumbers.begin(), secondRunnumbers.end(), firstRunnumbers[i]) != secondRunnumbers.end()) {
      secondAndFirstRunnumbers.push_back(firstRunnumbers[i]);
    }
  }

  if (thirdFile != "unset") {
    for (int i = 0; i < thirdRunnumbers.size(); i++) {
      if (find(firstRunnumbers.begin(), firstRunnumbers.end(), thirdRunnumbers[i]) != firstRunnumbers.end()) {
        thirdAndFirstRunnumbers.push_back(thirdRunnumbers[i]);
      }
      if (find(secondRunnumbers.begin(), secondRunnumbers.end(), thirdRunnumbers[i]) != secondRunnumbers.end()) {
        thirdAndSecondRunnumbers.push_back(thirdRunnumbers[i]);
      }
    }
    for (int i = 0; i < thirdRunnumbers.size(); i++) {
      if (find(firstRunnumbers.begin(), firstRunnumbers.end(), thirdRunnumbers[i]) == firstRunnumbers.end()
          && find(secondRunnumbers.begin(), secondRunnumbers.end(), thirdRunnumbers[i]) == secondRunnumbers.end()
         ) {
          thirdOnlyRunnumbers.push_back(thirdRunnumbers[i]);
      } 
      if (find(secondAndFirstRunnumbers.begin(), secondAndFirstRunnumbers.end(), thirdRunnumbers[i]) != secondAndFirstRunnumbers.end()) {
        allRunnumbers.push_back(thirdRunnumbers[i]);
      }
    }
  }

  cout << "firstOnlyRunnumbers: " << firstOnlyRunnumbers.size() << " total (" << firstRunnumbers.size() << ")" << endl;
  cout << endl;
  cout << "secondOnlyRunnumbers: " << secondOnlyRunnumbers.size() << " total (" << secondRunnumbers.size() << ")" << endl;
  cout << endl;

  if (secondAndFirstRunnumbers.size() > 0) {
    cout << "secondAndFirstRunnumbers: " << secondAndFirstRunnumbers.size() << " runs: " << endl;
    for (int i = 0; i < secondAndFirstRunnumbers.size(); i++) {
      cout << secondAndFirstRunnumbers[i];
      if (i < secondAndFirstRunnumbers.size() - 1) {
        cout << ",";
      }
    }
    cout << endl;
  } else {
    cout << "secondAndFirstRunnumbers: no runs" << endl;
  }

  cout << endl;
  if (thirdFile != "unset") {
    cout << "thirdOnlyRunnumbers: " << thirdOnlyRunnumbers.size() << " total (" << thirdRunnumbers.size() << ")" << endl;

    cout << endl;
    if (thirdAndFirstRunnumbers.size() > 0) {
      cout << "thirdAndFirstRunnumbers: " << thirdAndFirstRunnumbers.size() << " runs: " << endl;
      for (int i = 0; i < thirdAndFirstRunnumbers.size(); i++) {
        cout << thirdAndFirstRunnumbers[i];
        if (i < thirdAndFirstRunnumbers.size() - 1) {
          cout << ",";
        }
      }
      cout << endl;
    } else {
      cout << "thirdAndFirstRunnumbers: no runs" << endl;
    }

    cout << endl << endl;
    if (thirdAndSecondRunnumbers.size() > 0) {
      cout << "thirdAndSecondRunnumbers: " << thirdAndSecondRunnumbers.size() << " runs: " << endl;
      for (int i = 0; i < thirdAndSecondRunnumbers.size(); i++) {
        cout << thirdAndSecondRunnumbers[i];
        if (i < thirdAndSecondRunnumbers.size() - 1) {
          cout << ",";
        }
      }
      cout << endl;
    } else {
      cout << "thirdAndSecondRunnumbers: no runs" << endl;
    }

    cout << endl;
    if (allRunnumbers.size() > 0) {
      cout << "allRunnumbers: " << allRunnumbers.size() << " runs: " << endl;
      for (int i = 0; i < allRunnumbers.size(); i++) {
        cout << allRunnumbers[i];
        if (i < allRunnumbers.size() - 1) {
          cout << ",";
        }
      }
      cout << endl;
    } else {
      cout << "allRunnumbers: no runs" << endl;
    }
  }

  return 0;
}