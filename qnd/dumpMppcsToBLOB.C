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
void writeBlob(string filename = "mppcs.bin", bool modify = false) {
  TTree *t = (TTree*)gFile->Get("alignment/mppcs");
  unsigned int mppc;
  double vx, vy, vz;
  double colx, coly, colz;
  int ncol;
  long unsigned int header(0xdeadface);
  
  t->SetBranchAddress("mppc", &mppc);

  t->SetBranchAddress("vx", &vx);
  t->SetBranchAddress("vy", &vy);
  t->SetBranchAddress("vz", &vz);

  t->SetBranchAddress("colx", &colx);
  t->SetBranchAddress("coly", &coly);
  t->SetBranchAddress("colz", &colz);

  t->SetBranchAddress("ncol", &ncol);

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
    if (1) cout << "mppc = " << mppc
                << " v x/y/z = " << vx << "/" << vy << "/" << vz
                << " col x/y/z = " << colx << "/" << coly << "/" << colz
                << " ncol = " << ncol
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
      printArray(ONS, uint2Blob(mppc));
      printArray(ONS, double2Blob(mx));
      printArray(ONS, double2Blob(my));
      printArray(ONS, double2Blob(mz));
      printArray(ONS, double2Blob(colx));
      printArray(ONS, double2Blob(coly));
      printArray(ONS, double2Blob(colz));
      printArray(ONS, int2Blob(ncol));
    }
    
    if (1) {
      ONS << dumpArray(uint2Blob(mppc)) 
          << dumpArray(double2Blob(mx)) 
          << dumpArray(double2Blob(my)) 
          << dumpArray(double2Blob(mz)) 
          << dumpArray(double2Blob(colx))
          << dumpArray(double2Blob(coly))
          << dumpArray(double2Blob(colz))
          << dumpArray(int2Blob(ncol));
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
void readBlob(string filename = "mppcs.bin") {
  unsigned int mppc;
  double vx, vy, vz;
  double colx, coly, colz;
  int ncol;
  
  ifstream INS;
  INS.open(filename);

  vector<char> buffer(std::istreambuf_iterator<char>(INS), {});
  std::vector<char>::iterator ibuffer = buffer.begin();
  
  long unsigned int header = blob2UInt(getData(ibuffer)); 
  cout << "header: " << hex << header << dec << endl;

  while (ibuffer != buffer.end()) {
    mppc = blob2UInt(getData(ibuffer));
    vx = blob2Double(getData(ibuffer));
    vy = blob2Double(getData(ibuffer));
    vz = blob2Double(getData(ibuffer));
    colx = blob2Double(getData(ibuffer));
    coly = blob2Double(getData(ibuffer));
    colz = blob2Double(getData(ibuffer));
    ncol = blob2Int(getData(ibuffer));
    
    if (1) cout << "mppc = " << mppc
                << " v x/y/z = " << vx << "/" << vy << "/" << vz
                << " col x/y/z = " << colx << "/" << coly << "/" << colz
                << " ncol = " << ncol 
                << endl;
  }
}
