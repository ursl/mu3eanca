#include <unistd.h>
#include <array>

// -- DEPRECATED: Use mu3eanca/db0/cdb1/dumpAlignmentTreesToCsv.C

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
void writeBlob(string filename = "sensors.bin", bool modify = false) {
  TTree *t = (TTree*)gFile->Get("alignment/sensors");
  double vx, vy, vz;
  double rowx, rowy, rowz;
  double colx, coly, colz;
  UInt_t sensor;
  Int_t  nrow, ncol;
  long unsigned int header(0xdeadface);
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
  if (0) {
    printArray(ONS, uint2Blob(header));
  }
  if (1) {
    ONS << dumpArray(uint2Blob(header));
  }
  
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
    double mx, my, mz;
    if (modify) {
      if (0 == i%2) {
        mx = vx + 0.0001;
        my = vy + 0.0001;
        mz = vz + 0.0001;
      } else {
        mx = vx - 0.0001;
        my = vy - 0.0001;
        mz = vz - 0.0001;
      }
    } else {
      mx = vx;
      my = vy;
      mz = vz;
    }

    if (0) {
      printArray(ONS, uint2Blob(sensor));
      printArray(ONS, double2Blob(mx));
      printArray(ONS, double2Blob(my));
      printArray(ONS, double2Blob(mz));
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
    }
    
    if (1) {
      ONS << dumpArray(uint2Blob(sensor)) 
          << dumpArray(double2Blob(mx)) 
          << dumpArray(double2Blob(my)) 
          << dumpArray(double2Blob(mz)) 
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
          << dumpArray(double2Blob(pixelSize));
    }

  }
  ONS.close();
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
void readBlob(string filename = "sensors.bin") {
  double vx, vy, vz;
  double rowx, rowy, rowz;
  double colx, coly, colz;
  UInt_t sensor;
  Int_t  nrow, ncol;
  double width, length, thickness, pixelSize;
  long unsigned int header;
  
  ifstream INS;
  INS.open(filename);

  vector<char> buffer(std::istreambuf_iterator<char>(INS), {});
  std::vector<char>::iterator ibuffer = buffer.begin();

  header = blob2UInt(getData(ibuffer)); 
  cout << "header: " << hex << header << dec << endl;

  while (ibuffer != buffer.end()) {
    sensor = blob2UInt(getData(ibuffer));
    vx = blob2Double(getData(ibuffer));
    vy = blob2Double(getData(ibuffer));
    vz = blob2Double(getData(ibuffer));
    rowx = blob2Double(getData(ibuffer));
    rowy = blob2Double(getData(ibuffer));
    rowz = blob2Double(getData(ibuffer));
    colx = blob2Double(getData(ibuffer));
    coly = blob2Double(getData(ibuffer));
    colz = blob2Double(getData(ibuffer));
    nrow = blob2Int(getData(ibuffer));
    ncol = blob2Int(getData(ibuffer));
    width = blob2Double(getData(ibuffer));
    length = blob2Double(getData(ibuffer));
    thickness = blob2Double(getData(ibuffer));
    pixelSize = blob2Double(getData(ibuffer));
    
    if (0) cout << "sensor = " << sensor
                << " v x/y/z = " << vx << "/" << vy << "/" << vz
                << " row x/y/z = " << rowx << "/" << rowy << "/" << rowz
                << " col x/y/z = " << colx << "/" << coly << "/" << colz
                << " rest: " << nrow << "/" << ncol << "/" << width
                << "/" << length << "/" << thickness << "/" << pixelSize
                << endl;
    if (1) cout << sensor
                << "," << vx << "," << vy << "," << vz
                << "," << rowx << "," << rowy << "," << rowz
                << "," << colx << "," << coly << "," << colz
                << "," << nrow << "," << ncol << "," << width
                << "," << length << "," << thickness << "," << pixelSize
                << endl;
  }
}
