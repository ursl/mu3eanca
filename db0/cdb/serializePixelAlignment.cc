#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

#include "util/util.hh"

// include headers that implement a archive in simple text format
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/impl/basic_text_oarchive.ipp>
#include <boost/archive/impl/text_oarchive_impl.ipp>

using namespace std;

// ----------------------------------------------------------------------
// serializePixelAlignment
// -----------------------
//
// Examples:
// bin/serializePixelAlignment -f sensors.csv
// 
// ----------------------------------------------------------------------

struct pixelAlignment {
  pixelAlignment() {
    sensor = 0;
    vx = vy = vz = 0.;
    ax = ay = az = 0.;
  }
    
  pixelAlignment(vector<string> v) {
    if (v.size() != 7) {
      cout << "vector size not correct" << endl;
      return;
    }
    sensor = stoi(v[0]);
    vx = stof(v[1]);
    vy = stof(v[2]);
    vz = stof(v[3]);
    ax = stof(v[4]);
    ay = stof(v[5]);
    az = stof(v[6]);
  }
  void print() {
    cout << sensor << ": v = "
         << vx << "/" << vy << "/" << vz << " a = "
         << ax << "/" << ay << "/" << az
         << endl;
  }

  friend class boost::archive::save_access;
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive &ar, const unsigned int version = 0) {
    ar & sensor;
    ar & vx;
    ar & vy;
    ar & vz;
    ar & ax;
    ar & ay;
    ar & az;
  }
  int diff(pixelAlignment &a) {
    int res(0);
    if (sensor != a.sensor) res = 1; 
    if (fabs(vx - a.vx) > 1.e-8) res = 2; 
    if (fabs(vy - a.vy) > 1.e-8) res = 4; 
    if (fabs(vz - a.vz) > 1.e-8) res = 8; 
    return res; 
  }
                               
  int sensor;
  double vx, vy, vz;
  double ax, ay, az;
};


// ----------------------------------------------------------------------
vector<pixelAlignment> readCSV(string filename) {
  vector<pixelAlignment> res;
  ifstream INS;
  string sline;
  INS.open(filename); 
  while (getline(INS, sline)) {
    vector<string> tokens = split(sline, ',');
    pixelAlignment a(tokens);
    res.push_back(a);
  }    
  return res;
}


// ----------------------------------------------------------------------
void writePixelAlignment(string filename, vector<pixelAlignment> v) {
  ofstream ONS(filename);
  cout << "write boost archive to " << filename << endl;
  boost::archive::text_oarchive oa(ONS);
  for (auto it: v) {
    oa << it; 
  }
  ONS.close();
}

// ----------------------------------------------------------------------
vector<pixelAlignment> readPixelAlignment(string filename, int npa) {
  vector<pixelAlignment> res;
  ifstream INS(filename);
  cout << "read boost archive from " << filename << endl;
  boost::archive::text_iarchive ia(INS);
  pixelAlignment pa; 
  for (int i = 0; i < npa; ++i) {
    ia >> pa;
    res.push_back(pa);
  }
  INS.close();
  return res; 
}

// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
	ofstream ONS;
  
  // -- command line arguments
  string filename("sensors.csv");
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-f"))  {filename = string(argv[++i]);}
  }

  vector<pixelAlignment> sensors = readCSV(filename); 
  int npa = sensors.size();
  cout << "read " << npa << " sensor pixel alignment" << endl;
  for (auto it: sensors) {
    it.print();
  }
  string bfile = filename;
  replaceAll(bfile, ".csv", ".boost");
  writePixelAlignment(bfile, sensors);
  vector<pixelAlignment> rsensors = readPixelAlignment(bfile, npa);
  for (auto it: rsensors) {
    it.print();
  }
  cout << "======================================================================" << endl;
  cout << "Check for difference" << endl;
  for (unsigned int i = 0; i < sensors.size(); ++i) {
    if (sensors[i].diff(rsensors[i]) > 0) {
      cout << "Difference!" << endl;
      sensors[i].print(); 
      rsensors[i].print(); 
    }
    
  }
  cout << "======================================================================" << endl;
  return 0;
}
