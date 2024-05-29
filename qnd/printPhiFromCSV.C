#include <fstream>
#include <string>

#include "../util/util.hh"

using namespace::std;

void printPhiFromCSV(string filename, double minRadius = 0., double maxRadius = 40.) {

  vector<string> vfiles;
  // fill it

  string sline;
  ifstream INS(filename);
  while (getline(INS, sline)) {
    vfiles.push_back(sline);
  }
  INS.close();

  for (auto it: vfiles) {
    vector<string> numbers = split(it, ',');
    double x = atof(numbers[1].c_str());
    double y = atof(numbers[2].c_str());
    double z = atof(numbers[3].c_str());
    double r = TMath::Sqrt(x*x + y*y);
    //    cout << numbers[2] << ", " << numbers[1] << endl;

    if ((r > minRadius) && (r < maxRadius)) {
      cout << numbers[0] << ": " << x << ", " << y << ", " << z << ": "
           << " phi = " << TMath::RadToDeg()*TMath::ATan2(y,x)
           << " r = " << r
           << endl;
    }

  }

}
