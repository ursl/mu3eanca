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
void writeBlob(string filename = "fibres.bin", bool modify = false) {
  TTree *t = (TTree*)gFile->Get("alignment/fibres");
  unsigned int fibre;
  double cx, cy, cz;
  double fx, fy, fz;
  bool round, square;
  double diameter;
  long unsigned int header(0xdeadface);
  
  t->SetBranchAddress("fibre", &fibre);

  t->SetBranchAddress("cx", &cx);
  t->SetBranchAddress("cy", &cy);
  t->SetBranchAddress("cz", &cz);

  t->SetBranchAddress("fx", &fx);
  t->SetBranchAddress("fy", &fy);
  t->SetBranchAddress("fz", &fz);

  t->SetBranchAddress("round", &round);
  t->SetBranchAddress("square", &square);
  t->SetBranchAddress("diameter", &diameter);

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
    if (1) cout << "fibre = " << fibre
                << " c x/y/z = " << cx << "/" << cy << "/" << cz
                << " f x/y/z = " << fx << "/" << fy << "/" << fz
                << " round = " << round << " square = " << square
                << " diameter = " << diameter
                << endl;
    double mx, my, mz;
    if (modify) {
      if (0 == i%2) {
        mx = cx + 0.0001;
        my = cy + 0.0001;
        mz = cz + 0.0001;
      } else {
        mx = cx - 0.0001;
        my = cy - 0.0001;
        mz = cz - 0.0001;
      }
    } else {
      mx = cx;
      my = cy;
      mz = cz;
    }

    if (0) {
      printArray(ONS, uint2Blob(fibre));
      printArray(ONS, double2Blob(mx));
      printArray(ONS, double2Blob(my));
      printArray(ONS, double2Blob(mz));
      printArray(ONS, double2Blob(fx));
      printArray(ONS, double2Blob(fy));
      printArray(ONS, double2Blob(fz));
      printArray(ONS, uint2Blob(static_cast<unsigned int>(round)));
      printArray(ONS, uint2Blob(static_cast<unsigned int>(square)));
      printArray(ONS, double2Blob(diameter));
    }
    
    if (1) {
      ONS << dumpArray(uint2Blob(fibre)) 
          << dumpArray(double2Blob(mx)) 
          << dumpArray(double2Blob(my)) 
          << dumpArray(double2Blob(mz)) 
          << dumpArray(double2Blob(fx))
          << dumpArray(double2Blob(fy))
          << dumpArray(double2Blob(fz))
          << dumpArray(uint2Blob(static_cast<unsigned int>(round)))
          << dumpArray(uint2Blob(static_cast<unsigned int>(square)))
          << dumpArray(double2Blob(diameter));
    }

  }
  ONS.close();
}


// ----------------------------------------------------------------------
void writeCsv(string filename = "fibres.csv") {
  TTree *t = (TTree*)gFile->Get("alignment/fibres");
  unsigned int fibre;
  double cx, cy, cz;
  double fx, fy, fz;
  bool round, square;
  double diameter;
  long unsigned int header(0xdeadface);
  
  t->SetBranchAddress("fibre", &fibre);

  t->SetBranchAddress("cx", &cx);
  t->SetBranchAddress("cy", &cy);
  t->SetBranchAddress("cz", &cz);

  t->SetBranchAddress("fx", &fx);
  t->SetBranchAddress("fy", &fy);
  t->SetBranchAddress("fz", &fz);

  t->SetBranchAddress("round", &round);
  t->SetBranchAddress("square", &square);
  t->SetBranchAddress("diameter", &diameter);

  ofstream ONS;
  ONS.open(filename);
  
  char data[8], data1[8], data2[8]; 
  for (unsigned int i = 0; i < t->GetEntries(); ++i) {
    t->GetEntry(i);
    if (1) cout << "fibre = " << fibre
                << " c x/y/z = " << cx << "/" << cy << "/" << cz
                << " f x/y/z = " << fx << "/" << fy << "/" << fz
                << " round = " << round << " square = " << square
                << " diameter = " << diameter
                << endl;
    
    if (1) {
      ONS << fibre << ","
          << cx << "," << cy << "," << cz << ","
          << fx << "," << fy << "," << fz << ","
          << round << "," << square << ","
          << diameter
          << endl;
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
void readBlob(string filename = "fibres.bin") {
  unsigned int fibre;
  double cx, cy, cz;
  double fx, fy, fz;
  bool round, square;
  double diameter;
  long unsigned int header;
  
  ifstream INS;
  INS.open(filename);

  vector<char> buffer(std::istreambuf_iterator<char>(INS), {});
  std::vector<char>::iterator ibuffer = buffer.begin();

  header = blob2UInt(getData(ibuffer)); 
  cout << "header: " << hex << header << dec << endl;

  while (ibuffer != buffer.end()) {
    fibre = blob2UInt(getData(ibuffer));
    cx = blob2Double(getData(ibuffer));
    cy = blob2Double(getData(ibuffer));
    cz = blob2Double(getData(ibuffer));
    fx = blob2Double(getData(ibuffer));
    fy = blob2Double(getData(ibuffer));
    fz = blob2Double(getData(ibuffer));
    round = static_cast<bool>(blob2UInt(getData(ibuffer)));
    square = static_cast<bool>(blob2Double(getData(ibuffer)));
    diameter = blob2Double(getData(ibuffer));
    
    if (1) cout << "fibre = " << fibre
                << " c x/y/z = " << cx << "/" << cy << "/" << cz
                << " f x/y/z = " << fx << "/" << fy << "/" << fz
                << " round = " << round << " square = " << square
                << " diameter = " << diameter
                << endl;
  }
}
