#include <iostream>
#include <fstream>
#include <cstdio>

#include <TH2D.h>
#include <TStyle.h>
#include <TCanvas.h>

#include "sensor.hh"
#include "util/util.hh"
#include "../common/json.h"

using namespace::std;

map<int, struct sensor> gDetectorChips;
map<int, vector<pair<int, int> > > gChipNoisyPixels; 

int VERBOSE(0); 
int NCHIP(120);

// ----------------------------------------------------------------------
int getValInt(string line) {
  replaceAll(line, ",", "");
  replaceAll(line, " ", "");
  size_t icol = line.rfind(":");
  string snum = line.substr(icol+1);
  cout << "int  snum ->" << snum << "<-" << endl;
  return atoi(snum.c_str());
}

// ----------------------------------------------------------------------
float getValFloat(string line) {
  replaceAll(line, ",", "");
  replaceAll(line, " ", "");
  size_t icol = line.rfind(":");
  string snum = line.substr(icol+1);
  cout << "float snum ->" << snum << "<-" << endl;
  return atof(snum.c_str());
}

// ----------------------------------------------------------------------
vector<string> readEntry(vector<string> lines, int &iLine) {
  cout << "reading from line " << iLine << endl;
  vector<string> result;
  // -- counters for opening and closing braces
  int ibrace(0); 
  // -- start from indicated iLine
  for (unsigned int i = iLine; i < lines.size(); ++i) {
    if (string::npos != lines[i].find("{")) ++ibrace;
    if (string::npos != lines[i].find("}")) --ibrace;
    result.push_back(lines[i]); 
    if (0 == ibrace) {
      iLine = i + 1;
      break;
    }
  }

  return result;
  
}


// ----------------------------------------------------------------------
struct sensor fillEntry(vector<string> lines) {
  struct sensor chip; 
  for (unsigned int i = 0; i < lines.size(); ++i) {
    if (string::npos != lines[i].find("runChip")) {
      chip.runChip = getValInt(lines[i]);
    }
    if (string::npos != lines[i].find("layer")) {
      chip.layer = getValInt(lines[i]);
    }
    if (string::npos != lines[i].find("v")) {
      chip.v.SetX(getValFloat(lines[i+1]));
      chip.v.SetY(getValFloat(lines[i+2]));
      chip.v.SetZ(getValFloat(lines[i+3]));
    }
  }
  return chip;
}


// ----------------------------------------------------------------------
void readJSON(string filename, string dir = ".") {
  TCanvas c0("c0","--c0--",2303,0,656,700);

  vector<string> allLines; 
  ifstream INS;
  string sline;
  INS.open(filename);
  while (getline(INS, sline)) {
    allLines.push_back(sline);
  }   
  cout << "read " << allLines.size() << " lines" << endl;
  
  int iLine(1);
  vector<string> sentry = readEntry(allLines, iLine); 
  struct sensor chip; 
  while (sentry.size() > 0) {
    chip = fillEntry(sentry); 
    gDetectorChips.insert(make_pair(chip.runChip, chip));
    sentry = readEntry(allLines, iLine);
    if (iLine == allLines.size() - 1) break;
  }
  chip = fillEntry(sentry);
  gDetectorChips.insert(make_pair(chip.runChip, chip));
  
  TH2D *hl0 = new TH2D("hl0", "inner layer", 6, -42., 63., 8, -3.15, 3.15);
  TH2D *hl1 = new TH2D("hl1", "outer layer", 6, -42., 63., 10, -3.15, 3.15);

  for (map<int, struct sensor>::iterator it = gDetectorChips.begin(); it != gDetectorChips.end(); ++it) {
    int layer = (it->second.v.Perp() > 40.? 1:0);
    cout << it->second.runChip << ": v = (" << it->second.v.X() << ", " << it->second.v.Y() << ", " << it->second.v.Z() << ")"
         << " phi = " << it->second.v.Phi()
         << " r = " << it->second.v.Perp()
         << endl;
    if (0 == layer) {
      if (1 == it->second.layer) {
        cout << "XXXXXXXXXXXXXXX layer mismatch" << endl;
      }
      hl0->Fill(it->second.v.Z(), it->second.v.Phi(), it->second.runChip);
    } else {
      if (0 == it->second.layer) {
        cout << "XXXXXXXXXXXXXXX layer mismatch" << endl;
      }
      hl1->Fill(it->second.v.Z(), it->second.v.Phi(), it->second.runChip);
    }
  }

  gStyle->SetOptStat(0);
  hl0->SetMinimum(-1.);
  hl0->Draw("textcol");
  c0.SaveAs(Form("%s/l0.pdf", dir.c_str()));
  hl1->Draw("textcol");
  c0.SaveAs(Form("%s/l1.pdf", dir.c_str()));
  
}


// ----------------------------------------------------------------------
bool validNoise(const vector<uint8_t> &v) {
  for (unsigned int i = 0; i < v.size(); ++i) {
    if (0 != v[i]) {
      return true;
    }
  }
  return false;
}

// ----------------------------------------------------------------------
vector<uint8_t> readFile(string filename) {
  // -- open the file
  streampos fileSize;
  ifstream file;
  file.open(filename.c_str(), std::ios::binary);
  if (!file) {
    cout << "file ->" << filename << "<- not found, skipping" << endl;
    vector<uint8_t> fileData = {};
    return fileData;
  }
  
  // -- get its size
  file.seekg(0, std::ios::end);
  fileSize = file.tellg();
  file.seekg(0, std::ios::beg);
  
  // -- read the data
  vector<uint8_t> fileData(fileSize);
  file.read((char*) &fileData[0], fileSize);

  file.close();
  
  return fileData;
}

// ----------------------------------------------------------------------
pair<int, int> colrowFromIdx(int idx) {
  int col = idx/256;
  int row = idx%256;
  return make_pair(col, row);
}

// ----------------------------------------------------------------------
int idxFromColRow(int col, int row) {
  int idx = col*256 + row;
  return idx; 
}


// ----------------------------------------------------------------------
// -- produce summary of a mask file
// ----------------------------------------------------------------------
void summarize(vector<uint8_t> vnoise) {
  int cntNoisy(0);
  int errCode(0);
  for (unsigned int i = 0; i < vnoise.size(); ++i){
    if ((0xda == vnoise[i]) && (0xda == vnoise[i+1])) {
      errCode = vnoise[i+5];
      i += 5; //??
      continue;
    }
    pair<int, int> a = colrowFromIdx(i);
    if (0xff != vnoise[i]) {
      ++cntNoisy;
      if (VERBOSE > 0) cout << Form("pix: %d/%d ", a.first, a.second);
    }
  }
  if (VERBOSE > 0) cout  << endl;
  cout << " Number of noisy pixels = " << cntNoisy
       << (errCode>0? Form(", errCode = %d", errCode): "")
       << endl;
}


// ----------------------------------------------------------------------
// -- dump vector<uint8_t> into mask file
// ----------------------------------------------------------------------
void writeNoiseMaskFile(vector<uint8_t> noise, string ofilename) {
  ofstream o(ofilename); 
  for (unsigned int i = 0; i < noise.size(); ++i) {
    o << noise[i];
  }
  o.close();
}


// ----------------------------------------------------------------------
// -- produce summary of a mask file
// ----------------------------------------------------------------------
void summaryMaskFile(string filename) {
  vector<uint8_t> vnoise = readFile(filename);
  cout << filename << ": ";
  summarize(vnoise);
}


// ----------------------------------------------------------------------
// -- fill gChipNoisyPixels for a chipID
// ----------------------------------------------------------------------
void fillNoisyPixels(int chipID, vector<uint8_t> &vnoise,
                     map<int, vector<pair<int, int> > > &map1) {
  vector<pair<int, int> > vnp;

  for (unsigned int i = 0; i < vnoise.size(); ++i){
    if ((0xda == vnoise[i]) && (0xda == vnoise[i+1])) {
      i += 5; //??
      continue;
    }
    if (0xff != vnoise[i]) {
      pair<int, int> a = colrowFromIdx(i);
      vnp.push_back(a);
    }
  }
  map1.insert(make_pair(chipID, vnp));  
}


// ----------------------------------------------------------------------
// -- adds a run to the vector<uint8_t>
// ----------------------------------------------------------------------
void addNoiseMaskFile(vector<uint8_t> &vnoise, string name) {
  vector<uint8_t> vread = readFile(Form("%s", name.c_str()));
  // cout << name; 
  // summarize(vread);
  
  if (0 == vread.size()) {
    return;
  }
  
  for (unsigned int i = 0; i < vread.size(); ++i){
    if ((0xda == vnoise[i]) && (0xda == vnoise[i+1])) {
      // -- update error flag in case it is set
      if (0x00 != vread[i+5]) {
        vnoise[i+5] = vread[i+5];
      }
      i += 5; //??
      continue;
    }
 
    if ((0xff == vnoise[i]) && (0xff != vread[i])) {
      pair<int, int> a = colrowFromIdx(i);
      if (VERBOSE > 1) cout << Form("file %s change setting col/row = %3d/%3d from %x to %x",
                                    name.c_str(), a.first, a.second, vnoise[i], vread[i])
                            << endl;
      vnoise[i] = vread[i];
    }
  }
 
}
 
 
// ----------------------------------------------------------------------
// -- merge files into one mask file and summarizes the combination
// ----------------------------------------------------------------------
void mergeNoiseFiles(vector<string> filelist) {
  vector<uint8_t> vnoise;
  for (int i = 0; i < 256*256; ++i) vnoise.push_back(0xff);
  
  for (unsigned int ifile = 0; ifile < filelist.size(); ++ifile) {
    addNoiseMaskFile(vnoise, filelist[ifile]);
  }

  writeNoiseMaskFile(vnoise, "combination");
  summaryMaskFile("combination");
  remove("combination");
}


// ----------------------------------------------------------------------
// -- compare two files summarizes the difference
// ----------------------------------------------------------------------
void compareNoiseFiles(vector<string> filelist) {
  assert(2 == filelist.size());

  vector<uint8_t> vnoise1;
  for (int i = 0; i < 256*256; ++i) vnoise1.push_back(0xff);
  vector<uint8_t> vnoise2;
  for (int i = 0; i < 256*256; ++i) vnoise2.push_back(0xff);

  addNoiseMaskFile(vnoise1, filelist[0]);
  addNoiseMaskFile(vnoise2, filelist[1]);
  assert(vnoise1.size() == vnoise2.size());

  bool errCodeDiff(false);
  int  cntNoiseDiff(0);
  for (unsigned int i = 0; i < vnoise1.size(); ++i) {
    if ((0xda == vnoise1[i]) && (0xda == vnoise1[i+1])) {
      // -- update error flag in case it is set
      if (vnoise1[i+5] != vnoise2[i+5]) {
        errCodeDiff = true;
      }
      i += 5; //??
      continue;
    }
 
    if (vnoise1[i] != vnoise2[i]) {
      pair<int, int> a = colrowFromIdx(i);
      ++cntNoiseDiff;
      if (VERBOSE > 1) cout << Form("different setting col/row = %3d/%3d from %x to %x in %s vs. %s",
                                    a.first, a.second, vnoise1[i], vnoise1[i],
                                    filelist[0].c_str(), filelist[1].c_str()
                                    )
                            << endl;
    }
  }
  cout << "Difference of number of noisy pixels = " << cntNoiseDiff
       << ", difference in errCode = " <<  errCodeDiff
       << endl;
}

// ----------------------------------------------------------------------
// -- combines files into one mask file and summarizes the combination
// ----------------------------------------------------------------------
void mergeRunNoiseFiles(vector<int> runlist, string dir = ".") {
  vector<uint8_t> vnoise;
  
  for (unsigned int ichip = 0; ichip < NCHIP; ++ichip) {
    // -- initialize with no noisy pixels
    vnoise.clear();
    for (int i = 0; i < 256*256; ++i) vnoise.push_back(0xff);
    // -- add runs
    for (unsigned int irun = 0; irun < runlist.size(); ++irun) {
      string filename = Form("%s/noiseMaskFile-run%d-chipID%d", dir.c_str(), runlist[irun], ichip);
      addNoiseMaskFile(vnoise, filename);
    }
    writeNoiseMaskFile(vnoise, Form("%s/noiseMaskFile-chipID%d", dir.c_str(), ichip));
  }
}



// ----------------------------------------------------------------------
// -- main
// ----------------------------------------------------------------------
int main(int argc, char *argv[]) {
  string outputdir(".");
  // -- command line arguments
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i],"-v"))  {
      VERBOSE = atoi(argv[++i]);
    }      

    if (!strcmp(argv[i],"-o"))  {
      outputdir = argv[++i];
    }      

    if (!strcmp(argv[i],"-h")) {
      cout << "List of arguments: (provide -v as first argument!)" << endl;
      cout << "-h                prints this message and exits" << endl;
      cout << "-c file1 file2    compare two noisemaskfiles" << endl;
      cout << "-m file1 file2    merge various noisemaskfiles for a single chip" << endl;
      cout << "-o outputdir      set output directory" << endl;
      cout << "-r run1,run2,run3 combine noisemaskfiles for all chips for given runs" << endl;
      cout << "-s filename       summarize noisemask with filename" << endl;
      cout << "-v level          set verbosity level " << endl;
      return 0;
    }
  }

  for (int i = 0; i < argc; i++){
    // --  previously parsed
    if (!strcmp(argv[i],"-v"))  {++i; continue;}
    if (!strcmp(argv[i],"-o"))  {++i; continue;}

    // -- compare two files and print difference
    if (!strcmp(argv[i],"-c"))  {
      vector<string> fnames; 
      for (int j = i+1; j < argc; ++j) {
        fnames.push_back(argv[j]);
      }
      compareNoiseFiles(fnames);
      return 0; 
    }

    
    // -- merge specific files and summarize
    if (!strcmp(argv[i],"-m"))  {
      vector<string> fnames; 
      for (int j = i+1; j < argc; ++j) {
        fnames.push_back(argv[j]);
      }
      mergeNoiseFiles(fnames);
      return 0; 
    }

    // -- combine all files for specific runs
    if (!strcmp(argv[i],"-r"))  {
      vector<string> srunlist = split(argv[++i], ',');
      vector<int> runlist;
      for (int j = 0; j < srunlist.size(); ++j) {
        runlist.push_back(atoi(srunlist[j].c_str()));
        cout << "j = " << j << " run = " << runlist[j] << endl;
      }
      mergeRunNoiseFiles(runlist, outputdir);
      return 0; 
    }

    // -- summarize a single noise mask file
    if (!strcmp(argv[i],"-s"))  {
      string filename = argv[++i];
      summaryMaskFile(filename);
      return 0; 
    }

  }
  return 0;
}

