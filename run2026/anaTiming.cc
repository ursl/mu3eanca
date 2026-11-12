#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <cstdlib>


#include "TCanvas.h"
#include "TFrame.h"
#include "TText.h"
#include "TLine.h"
#include "TH1D.h"
#include "TStyle.h"
#include "TLatex.h"

using namespace std;

// --------------------------------------------------------------------
// Run number sits between "mlzr/" and "/tmp".
// The mean time is the fourth number from the right (dashes are not numbers).
static bool parseRun(const string& line, int& run) {
  const string key = "mlzr/";
  size_t p = line.find(key);
  if (p == string::npos) return false;
  p += key.size();
  size_t q = line.find("/tmp", p);
  if (q == string::npos || q == p) return false;
  try {
    size_t used = 0;
    run = stoi(line.substr(p, q - p), &used);
    return used == (q - p);
  } catch (...) {
    return false;
  }
}

static bool fourthFromRight(const string& line, double& value) {
  stringstream ss(line);
  string token;
  vector<double> nums;
  while (ss >> token) {
    char* end = nullptr;
    double v = strtod(token.c_str(), &end);
    if (end != token.c_str() && *end == '\0') nums.push_back(v);
  }
  if (nums.size() < 4) return false;
  value = nums[nums.size() - 4];
  return true;
}

void readFile(const string& filename, map<int, double>& mRunTime) {
  ifstream file(filename);
  if (!file.is_open()) {
    cout << "Error opening file " << filename << endl;
    return;
  }

  map<int, double> sum;
  map<int, int> n;
  string line;
  while (getline(file, line)) {
    int run = 0;
    double t = 0;
    if (!parseRun(line, run)) continue;
    if (!fourthFromRight(line, t)) continue;
    sum[run] += t;
    n[run] += 1;
  }
  file.close();

  for (const auto& it : n) {
    if (sum[it.first] == 0) continue;
    if (it.second == 0) continue;
    mRunTime[it.first] = sum[it.first] / it.second;
  }
}

void anaTiming() {
  map<int, double> mRunTime;
  readFile("root/pixelhistos-v04.txt", mRunTime);
  readFile("root/pixelhistos-v05.txt", mRunTime);
  readFile("root/pixelhistos-v06.txt", mRunTime);

  double v04, v05, v06;
  TH1D hRunTime("hRunTime", "mean pixelhistos run time / \"event\" (ms)", mRunTime.size(), 0, mRunTime.size());
  int idx(1); 
  for (const auto& it : mRunTime) {
    hRunTime.Fill(idx, it.second);

    if (it.first == 11800) v04 = idx;
    if (it.first == 12543) v05 = idx;
    if (it.first == 12608) v06 = idx;
    if (idx%50 == 0)  hRunTime.GetXaxis()->SetBinLabel(idx, to_string(it.first).c_str());
    idx++;
    cout << it.first << " " << it.second << endl;
  }


  TCanvas c("c", "c", 800, 600);
  c.cd();
  gStyle->SetOptStat(0);
  hRunTime.Draw("hist");
  TLatex *tl = new TLatex();
  tl->SetTextSize(0.04);
  tl->DrawLatex(v04, 1000, "v04");
  tl->DrawLatex(v05, 1000, "v05");
  tl->DrawLatex(v06, 800, "v06");
  delete tl;
  c.SaveAs("runTime.pdf");
  c.SaveAs("runTime.png");
}

int main() {
  anaTiming();
  return 0;
}
