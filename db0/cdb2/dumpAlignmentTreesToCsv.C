//

#include <array>
#include <string>
#include <vector>

#include <unistd.h>

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
  std::array<char,8> v;
  for (int i = 0; i < 8; ++i) v[i] = data[i];
  return v;
}


// ----------------------------------------------------------------------
std::array<char,8> double2Blob(double a) {
  char data[8] = {0};
  memcpy(data, &a, sizeof a);
  std::array<char,8> v;
  for (int i = 0; i < 8; ++i) v[i] = data[i];
  return v;
}


// ----------------------------------------------------------------------
std::string dumpArray(std::array<char,8> v) {
  std::stringstream sstr;
  for (auto it: v) sstr << it;
  return sstr.str();
}


// ----------------------------------------------------------------------
void printArray(std::ofstream &OS, std::array<char,8> v) {
  for (auto it: v) OS << it;
}


// ----------------------------------------------------------------------
std::array<char,8> getData(std::vector<char>::iterator &it) {
  std::array<char,8> v;
  for (unsigned int i = 0; i < 8; ++i) {
    v[i] = *it;
    ++it;
  }
  return v;
}


// ----------------------------------------------------------------------
void writeSensors(std::string filename = "sensors.csv", bool modify = false) {
  TTree *t = gFile->Get<TTree>("alignment/sensors");
  if (0 == t) {
    cout << "dumpAlignmentToCsv/writeSensors> Error in retrieving tree alignment/sensors" << endl;
    return;
  }
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

  // -- default is CSV
  int mode(0);
  // -- BLOB
  if (string::npos != filename.find(".bin")) mode = 1;

  ofstream ONS;
  ONS.open(filename);
  if (1 == mode) {
    printArray(ONS, uint2Blob(header));
    if (0) ONS << dumpArray(uint2Blob(header));
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

    if (0 == mode) {
      ONS << sensor << ","
          << std::setprecision(15)
          << mx << ","
          << my << ","
          << mz << ","
          << rowx << ","
          << rowy << ","
          << rowz << ","
          << colx << ","
          << coly << ","
          << colz << ","
          << nrow << ","
          << ncol << ","
          << std::setprecision(15)
          << width << ","
          << length << ","
          << thickness << ","
          << pixelSize
          << endl;
    }

    if (1 == mode) {
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
          << std::setprecision(9)
          << dumpArray(double2Blob(width))
          << dumpArray(double2Blob(length))
          << dumpArray(double2Blob(thickness))
          << dumpArray(double2Blob(pixelSize));
    }

  }
  ONS.close();
}


// ----------------------------------------------------------------------
void readBlobSensors(string filename = "sensors.bin") {
  double vx, vy, vz;
  double rowx, rowy, rowz;
  double colx, coly, colz;
  UInt_t sensor;
  Int_t  nrow, ncol;
  double width, length, thickness, pixelSize;
  long unsigned int header;

  ifstream INS;
  INS.open(filename);
  if (INS.fail()) {
    cout << "dumpAlignmentToCsv/readBlobSensors> Error in opening " << filename << endl;
    return;
  }

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


// ----------------------------------------------------------------------
void writeTiles(string filename = "tiles.csv", bool modify = false) {
  TTree *t = gFile->Get<TTree>("alignment/tiles");
  if (0 == t) {
    cout << "dumpAlignmentToCsv/writeTiles> Error in retrieving tree alignment/tiles" << endl;
    return;
  }
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

  // -- default is CSV
  int mode(0);
  // -- BLOB
  if (string::npos != filename.find(".bin")) mode = 1;

  ofstream ONS;
  ONS.open(filename);
  // -- dump header only for BLOB
  if (1 == mode) {
    ONS << dumpArray(uint2Blob(header));
    if (0) printArray(ONS, uint2Blob(header));
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

    // -- print it
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

    // -- dump BLOB
    if (1 == mode) {
      ONS << dumpArray(int2Blob(sensor))
          << dumpArray(uint2Blob(id))
          << dumpArray(double2Blob(posx))
          << dumpArray(double2Blob(posy))
          << dumpArray(double2Blob(posz))
          << dumpArray(double2Blob(dirx))
          << dumpArray(double2Blob(diry))
          << dumpArray(double2Blob(dirz));
    }

    // -- dump CSV
    if (0 == mode) {
      ONS << sensor << ","
          << id << ","
          << std::setprecision(15)
          << posx << ","
          << posy << ","
          << posz << ","
          << dirx << ","
          << diry << ","
          << dirz
          << endl;
    }

  }
  ONS.close();
}


// ----------------------------------------------------------------------
void readBlobTiles(string filename = "tiles.bin") {
  unsigned int id;
  int sensor;
  double posx, posy, posz;
  double dirx, diry, dirz;
  long unsigned int header;

  ifstream INS;
  INS.open(filename);
  if (INS.fail()) {
    cout << "dumpAlignmentToCsv/readBlobTiles> Error in opening " << filename << endl;
    return;
  }

  vector<char> buffer(std::istreambuf_iterator<char>(INS), {});
  std::vector<char>::iterator ibuffer = buffer.begin();

  header = blob2UInt(getData(ibuffer));
  cout << "header: " << hex << header << dec << endl;

  while (ibuffer != buffer.end()) {
    sensor = blob2UInt(getData(ibuffer));
    id     = blob2UInt(getData(ibuffer));
    posx   = blob2Double(getData(ibuffer));
    posy   = blob2Double(getData(ibuffer));
    posz   = blob2Double(getData(ibuffer));
    dirx   = blob2Double(getData(ibuffer));
    diry   = blob2Double(getData(ibuffer));
    dirz   = blob2Double(getData(ibuffer));

    if (1) cout << "sensor/id = " << sensor << "/" << id
                << " pos x/y/z = " << posx << "/" << posy << "/" << posz
                << " dir x/y/z = " << dirx << "/" << diry << "/" << dirz
                << endl;
  }
}


// ----------------------------------------------------------------------
void writeFibres(string filename = "fibres.csv", bool modify = false) {
  TTree *t = gFile->Get<TTree>("alignment/fibres");
  if (0 == t) {
    cout << "dumpAlignmentToCsv/writeFibres> Error in retrieving tree alignment/fibres" << endl;
    return;
  }
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

  // -- default is CSV
  int mode(0);
  // -- BLOB
  if (string::npos != filename.find(".bin")) mode = 1;

  ofstream ONS;
  ONS.open(filename);
  if (1 == mode) {
    printArray(ONS, uint2Blob(header));
    if (0) ONS << dumpArray(uint2Blob(header));
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

    // -- print it
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

    // -- dump BLOB
    if (1 == mode) {
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

    // -- dump CSV
    if (0 == mode) {
      ONS << fibre << ","
          << std::setprecision(15)
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
void readBlobFibres(string filename = "fibres.bin") {
  unsigned int fibre;
  double cx, cy, cz;
  double fx, fy, fz;
  bool round, square;
  double diameter;
  long unsigned int header;

  ifstream INS;
  INS.open(filename);
  if (INS.fail()) {
    cout << "dumpAlignmentToCsv/readBlobFibres> Error in opening " << filename << endl;
    return;
  }

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


// ----------------------------------------------------------------------
void writeMppcs(string filename = "mppcs.csv", bool modify = false) {
  TTree *t = gFile->Get<TTree>("alignment/mppcs");
  if (0 == t) {
    cout << "dumpAlignmentToCsv/writeMppcs> Error in retrieving tree alignment/mppcs" << endl;
    return;
  }

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

  // -- default is CSV
  int mode(0);
  // -- BLOB
  if (string::npos != filename.find(".bin")) mode = 1;

  ofstream ONS;
  ONS.open(filename);
  if (1 == mode) {
    printArray(ONS, uint2Blob(header));
    if (0) ONS << dumpArray(uint2Blob(header));
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

    if (0 == mode) {
      ONS << mppc << ","
          << std::setprecision(15)
          << mx << ","
          << my << ","
          << mz << ","
          << colx << ","
          << coly << ","
          << colz << ","
          << ncol
          << endl;
    }

    if (1 == mode) {
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
void readBlobMppcs(string filename = "mppcs.bin") {
  unsigned int mppc;
  double vx, vy, vz;
  double colx, coly, colz;
  int ncol;

  ifstream INS;
  INS.open(filename);
  if (INS.fail()) {
    cout << "dumpAlignmentToCsv/readBlobMppcs> Error in opening " << filename << endl;
    return;
  }

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


// ----------------------------------------------------------------------
// intrun:  ~/data/mu3e/mc/run000042-sort_100k.root
// mcideal full det: ~/data/mu3e/mc/mu3e_sorted_000779.root
void writeAll(string mode = "mcideal_2024CosmicRun", string filename = "nada") {
  if (string::npos != mode.find("mcideal_v5.0") && (string::npos != filename.find("nada"))) {
    filename = "/data/experiment/mu3e/mc/build_5.0/michel/mu3e_run_000011.root";
  } else {
    cout << "we trust that you properly specified tag and filename" << endl;
  }

  TFile *f = TFile::Open(filename.c_str());
  string ofile("");

  ofile  = "sensors-" + mode + ".csv";
  writeSensors(ofile);

  ofile = "fibres-" + mode + ".csv";
  writeFibres(ofile);

  ofile = "tiles-" + mode + ".csv";
  writeTiles(ofile);

  ofile = "mppcs-" + mode + ".csv";
  writeMppcs(ofile);
}


// ----------------------------------------------------------------------
// mcideal cosmic run: ~/data/mu3e/mc/mu3e_1E4_beam_2L2T-v5.4.root
void writeCosmicRun(string mode = "mcideal_2025CosmicRun", string filename = "nada") {
  if ("mcideal_2024CosmicRun" == mode && ("nada" == filename)) {
    filename = "/Users/ursl/data/mu3e/mc/mu3e_1E4_beam_2L2T-v5.4.root";
  } else {
    cout << "we trust that you properly specified tag and filename" << endl;
  }

  TFile *f = TFile::Open(filename.c_str());
  string ofile("");

  ofile  = "sensors-" + mode + ".csv";
  writeSensors(ofile);

  if (0) {
    ofile = "fibres-" + mode + ".csv";
    writeFibres(ofile);
    
    ofile = "tiles-" + mode + ".csv";
    writeTiles(ofile);
    
    ofile = "mppcs-" + mode + ".csv";
    writeMppcs(ofile);
  }
}
