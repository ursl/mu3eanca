#include <unistd.h>
#include <array>

// ----------------------------------------------------------------------
//Int_t blob2Int(char data[8]) {
//Int_t a(0);
//memcpy(&a, data, sizeof a);
//return a;
//}


// ----------------------------------------------------------------------
//UInt_t blob2UInt(char data[8]) {
//UInt_t a(0);
//memcpy(&a, data, sizeof a);
//return a;
//}


// ----------------------------------------------------------------------
//double blob2Double(char data[8]) {
// double a(0.0);
//  memcpy(&a, data, sizeof a);
//  return a;
//}


// ----------------------------------------------------------------------
Int_t blob2Int(std::array<char,8> v) {
  char data[8] = {0}; 
  for (int i = 0; i < 8; ++i) data[i] = v[i];
  Int_t a(0);
  memcpy(&a, data, sizeof a);
  return a;
}


// ----------------------------------------------------------------------
UInt_t blob2UInt(std::array<char,8> v) {
  char data[8] = {0}; 
  for (int i = 0; i < 8; ++i) data[i] = v[i];
  UInt_t a(0);
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
std::array<char,8> int2Blob(Int_t a) {
  char data[8] = {0}; 
  memcpy(data, &a, sizeof a);
  array<char,8> v; 
  for (int i = 0; i < 8; ++i) v[i] = data[i]; 
  return v;
}

// ----------------------------------------------------------------------
std::array<char,8> uint2Blob(UInt_t a) {
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
void writeBlob(string filename = "sensors.bin") {
  TTree *t = (TTree*)gFile->Get("alignment/sensors");
  double vx, vy, vz;
  double rowx, rowy, rowz;
  double colx, coly, colz;
  UInt_t sensor;
  Int_t  nrow, ncol;
  double width, length, thickness, pixelSize;
  
  t->SetBranchAddress("sensor", &sensor);

  t->SetBranchAddress("vx", &vx);
  t->SetBranchAddress("vy", &vy);
  t->SetBranchAddress("vz", &vz);

  t->SetBranchAddress("rowx", &rowx);
  t->SetBranchAddress("rowy", &rowy);
  t->SetBranchAddress("rowz", &rowz);

  t->SetBranchAddress("colx", &colx);
  t->SetBranchAddress("coly", &coly);
  t->SetBranchAddress("colz", &colz);

  t->SetBranchAddress("nrow", &nrow);
  t->SetBranchAddress("ncol", &ncol);
  t->SetBranchAddress("width", &width);
  t->SetBranchAddress("length", &length);
  t->SetBranchAddress("thickness", &thickness);
  t->SetBranchAddress("pixelSize", &pixelSize);

  ofstream ONS;
  ONS.open(filename);

  char data[8], data1[8], data2[8]; 
  for (unsigned int i = 0; i < t->GetEntries(); ++i) {
    t->GetEntry(i);
    if (1) cout << "sensor = " << sensor
                << " v x/y/z = " << vx << "/" << vy << "/" << vz
                << " row x/y/z = " << rowx << "/" << rowy << "/" << rowz
                << " col x/y/z = " << colx << "/" << coly << "/" << colz
                << " rest: " << nrow << "/" << ncol << "/" << width
                << "/" << length << "/" << thickness << "/" << pixelSize
                << endl;
    printArray(ONS, uint2Blob(sensor));
    printArray(ONS, double2Blob(vx));
    printArray(ONS, double2Blob(vy));
    printArray(ONS, double2Blob(vz));
    printArray(ONS, double2Blob(rowx));
    printArray(ONS, double2Blob(rowy));
    printArray(ONS, double2Blob(rowz));
    printArray(ONS, double2Blob(colx));
    printArray(ONS, double2Blob(coly));
    printArray(ONS, double2Blob(colz));
    printArray(ONS, int2Blob(nrow));
    printArray(ONS, int2Blob(ncol));
    printArray(ONS, double2Blob(width));
    printArray(ONS, double2Blob(length));
    printArray(ONS, double2Blob(thickness));
    printArray(ONS, double2Blob(pixelSize));

    if (0)    ONS << dumpArray(uint2Blob(sensor)) 
                  << dumpArray(double2Blob(vx)) 
                  << dumpArray(double2Blob(vy)) 
                  << dumpArray(double2Blob(vz)) 
                  << dumpArray(double2Blob(rowx))
                  << dumpArray(double2Blob(rowy))
                  << dumpArray(double2Blob(rowz))
                  << dumpArray(double2Blob(colx))
                  << dumpArray(double2Blob(coly))
                  << dumpArray(double2Blob(colz))
                  << dumpArray(int2Blob(nrow))
                  << dumpArray(int2Blob(ncol))
                  << dumpArray(double2Blob(width))
                  << dumpArray(double2Blob(length))
                  << dumpArray(double2Blob(thickness))
                  << dumpArray(double2Blob(pixelSize))
                  << endl;
  }
  ONS.close();
}


// ----------------------------------------------------------------------
std::array<char,8> getData(vector<char>::iterator it) {
  array<char,8> v;
  for (unsigned int i = 0; i < 8; ++i) {
    v[i] = *it;
    ++it;
  }
  return v;
}


// ----------------------------------------------------------------------
void readBlob(string filename = "sensors.bin") {
  double vx, vy, vz;
  double rowx, rowy, rowz;
  double colx, coly, colz;
  UInt_t sensor;
  Int_t  nrow, ncol;
  double width, length, thickness, pixelSize;
  
  ifstream INS;
  INS.open(filename);

  vector<char> buffer(std::istreambuf_iterator<char>(INS), {});
  
  std::array<char,8> v;
  std::vector<char>::iterator ibuffer = buffer.begin();
  while (ibuffer != buffer.end()) {
    sensor = blob2UInt(getData(ibuffer)); ibuffer += 8;
    vx = blob2Double(getData(ibuffer)); ibuffer += 8;
    vy = blob2Double(getData(ibuffer)); ibuffer += 8;
    vz = blob2Double(getData(ibuffer)); ibuffer += 8;
    rowx = blob2Double(getData(ibuffer)); ibuffer += 8;
    rowy = blob2Double(getData(ibuffer)); ibuffer += 8;
    rowz = blob2Double(getData(ibuffer)); ibuffer += 8;
    colx = blob2Double(getData(ibuffer)); ibuffer += 8;
    coly = blob2Double(getData(ibuffer)); ibuffer += 8;
    colz = blob2Double(getData(ibuffer)); ibuffer += 8;
    nrow = blob2Int(getData(ibuffer)); ibuffer += 8;
    ncol = blob2Int(getData(ibuffer)); ibuffer += 8;
    width = blob2Double(getData(ibuffer)); ibuffer += 8;
    length = blob2Double(getData(ibuffer)); ibuffer += 8;
    thickness = blob2Double(getData(ibuffer)); ibuffer += 8;
    pixelSize = blob2Double(getData(ibuffer)); ibuffer += 8;
    
    if (1) cout << "sensor = " << sensor
                << " v x/y/z = " << vx << "/" << vy << "/" << vz
                << " row x/y/z = " << rowx << "/" << rowy << "/" << rowz
                << " col x/y/z = " << colx << "/" << coly << "/" << colz
                << " rest: " << nrow << "/" << ncol << "/" << width
                << "/" << length << "/" << thickness << "/" << pixelSize
                << endl;
  }
  
}
