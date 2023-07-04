#include <unistd.h>
#include <array>

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
void writeBlob(string filename = "tiles.bin", bool modify = false) {
  TTree *t = (TTree*)gFile->Get("alignment/tiles");
  int sensor;
  unsigned int id;
  double posx, posy, posz;
  double dirx, diry, dirz;
  long unsigned int header(0xdeadface);
  
  t->SetBranchAddress("sensor", &sensor);
  t->SetBranchAddress("id", &id);

  t->SetBranchAddress("posx", &posx);
  t->SetBranchAddress("posy", &posy);
  t->SetBranchAddress("posz", &posz);

  t->SetBranchAddress("dirx", &dirx);
  t->SetBranchAddress("diry", &diry);
  t->SetBranchAddress("dirz", &dirz);

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
    if (1) cout << "sensor = " << sensor << " id = " << id 
                << " pos x/y/z = " << posx << "/" << posy << "/" << posz
                << " dir x/y/z = " << dirx << "/" << diry << "/" << dirz
                << endl;
    double mx, my, mz;
    if (modify) {
      if (0 == i%2) {
        mx = posx + 0.0001;
        my = posy + 0.0001;
        mz = posz + 0.0001;
      } else {
        mx = posx - 0.0001;
        my = posy - 0.0001;
        mz = posz - 0.0001;
      }
    } else {
      mx = posx;
      my = posy;
      mz = posz;
    }

    if (0) {
      printArray(ONS, int2Blob(sensor));
      printArray(ONS, uint2Blob(id));
      printArray(ONS, double2Blob(posx));
      printArray(ONS, double2Blob(posy));
      printArray(ONS, double2Blob(posz));
      printArray(ONS, double2Blob(dirx));
      printArray(ONS, double2Blob(diry));
      printArray(ONS, double2Blob(dirz));
    }
    
    if (1) {
      ONS << dumpArray(int2Blob(sensor)) 
          << dumpArray(uint2Blob(id)) 
          << dumpArray(double2Blob(posx)) 
          << dumpArray(double2Blob(posy)) 
          << dumpArray(double2Blob(posz)) 
          << dumpArray(double2Blob(dirx))
          << dumpArray(double2Blob(diry))
          << dumpArray(double2Blob(dirz));
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
  unsigned int sensor;
  double posx, posy, posz;
  double dirx, diry, dirz;
  long unsigned int header;
  
  ifstream INS;
  INS.open(filename);

  vector<char> buffer(std::istreambuf_iterator<char>(INS), {});
  std::vector<char>::iterator ibuffer = buffer.begin();

  header = blob2UInt(getData(ibuffer)); 
  cout << "header: " << hex << header << dec << endl;

  while (ibuffer != buffer.end()) {
    sensor = blob2UInt(getData(ibuffer));
    posx = blob2Double(getData(ibuffer));
    posy = blob2Double(getData(ibuffer));
    posz = blob2Double(getData(ibuffer));
    dirx = blob2Double(getData(ibuffer));
    diry = blob2Double(getData(ibuffer));
    dirz = blob2Double(getData(ibuffer));
    
    if (1) cout << "sensor = " << sensor
                << " pos x/y/z = " << posx << "/" << posy << "/" << posz
                << " dir x/y/z = " << dirx << "/" << diry << "/" << dirz
                << endl;
  }
}
