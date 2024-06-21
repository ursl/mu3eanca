#include "nlohmann/json.hpp"
#include <iostream>
#include <fstream>
#include <string>

#include <TROOT.h>
#include <TH2F.h>
#include <TBranch.h>
#include <TVector3.h>
#include <TChain.h>
#include <TFile.h>
#include <TTree.h>
#include <TTimeStamp.h>


#include "util/util.hh"

using namespace std;
using json = nlohmann::json;


// ----------------------------------------------------------------------
string time2SqlFormat(string time) {
  string result = time;
  replaceAll(result, "T", " ");
  result = result.substr(0, result.rfind("."));
  return result;
}

// ----------------------------------------------------------------------
void readJSON(string filename) {

  // -- read the JSON file
  ifstream INS;
  string sline, jline;
  INS.open(filename);
  while (getline(INS, sline)) {
    jline += sline;
  }

  // -- parse and serialize JSON
  json jComp = json::parse(jline);
  //  cout << std::setw(2) << jComp << endl;

  // -- prepare list of tags
  map<int, string> keywords;
  vector pars = {"sci", "pix", "daq"}; // LOWER CASE!
  for (auto [key, val] : jComp.items()) {
    string bla = val["doc"]["type"].dump();
    if (bla == "\"product\"") {
      bla = val["doc"]["pn"].dump();
      int ipn = stoi(bla.substr(1, bla.find("\"") - 1));
      for (int ipar = 0; ipar < pars.size(); ++ipar) {
        bla = val["doc"]["title"].dump();
        std::transform(bla.begin(), bla.end(), bla.begin(), [](unsigned char c){ return std::tolower(c); });
        if (string::npos != bla.find(pars[ipar])) {
          if (keywords.find(ipn) == keywords.end()) {
            keywords.insert(make_pair(ipn, pars[ipar]));
          } else {
            keywords[ipn] += (string(",") + pars[ipar]);
          }
        }
        bla = val["doc"]["tags"].dump();
        std::transform(bla.begin(), bla.end(), bla.begin(), [](unsigned char c){ return std::tolower(c); });
        if (string::npos != bla.find(pars[ipar])) {
          if (keywords.find(ipn) == keywords.end()) {
            keywords.insert(make_pair(ipn, pars[ipar]));
          } else {
            if (string::npos != keywords[ipn].find(pars[ipar])) {
              // do not add, already the same is in there (from title)
            } else {
              keywords[ipn] += (string(",") + pars[ipar]);
            }
          }
        }
      }
    }
  }

  if (0) {
    for (auto it: keywords) {
      cout << it.first << " " << it.second << endl;
    }
  }

  string rfilename = filename;
  replaceAll(rfilename, ".json", ".root");

  TFile *pdbFile = TFile::Open(rfilename.c_str(), "RECREATE");

  TTree *pdbTree = new TTree("pdb", "pdb");

  string id, type, tag;
  int revision;
  int pn, lot, item, lotsize, stocksize;

  int nstatus;
  string  stState[10];
  TDatime stTime[10];

  pdbTree->Branch("id",       &id);
  pdbTree->Branch("name",     &type);
  pdbTree->Branch("type",     &type);
  pdbTree->Branch("revision", &revision);
  pdbTree->Branch("tag",      &tag);

  pdbTree->Branch("pn",   &pn);
  pdbTree->Branch("item", &item);
  pdbTree->Branch("lot",  &lot);
  pdbTree->Branch("lotsize",  &lotsize);
  pdbTree->Branch("stocksize",  &stocksize);

  pdbTree->Branch("n", &nstatus);
  pdbTree->Branch("stState", &stState[0]);
  pdbTree->Branch("stTime", &stTime[0]);

  for (auto [key, val] : jComp.items()) {
    string bla = val["doc"]["_rev"].dump();
    try {
      revision = stoi(bla.substr(1, bla.find("-") - 1));
		}
    catch(const std::invalid_argument&) {
      revision = -2;
    }

    bla = val["doc"]["type"].dump();
    type = bla.substr(1, bla.rfind("\"") - 1);

    bla = val["doc"]["pn"].dump();
    pn = stoi(bla.substr(1, bla.find("\"") - 1));
    tag = keywords[pn];


    bla = val["doc"]["lot"].dump();
    lot = -1;
    try {
      lot = stoi(bla.substr(1, bla.find("\"") - 1));
		}
    catch(const std::invalid_argument&) {
      lot = -2;
    }

    bla = val["doc"]["item"].dump();
    item = -1;
    try {
      item = stoi(bla.substr(1, bla.find("\"") - 1));
		}
    catch(const std::invalid_argument&) {
      item = -2;
    }

    bla = val["doc"]["_id"].dump();
    id = bla.substr(1, bla.rfind("\"") - 1);

    lotsize = -1;
    if (val["doc"]["type"] == "lot") {
      bla = val["doc"]["size"].dump();
      try {
        lotsize = stoi(bla.substr(1, bla.find("\"") - 1));
      }
      catch(const std::invalid_argument&) {
        lotsize = -2;
      }
    }

    if (0) {
      cout << " " << type
           << " " << pn
           << " " << lot
           << " " << item
           << " " << id;
      if (val["doc"]["type"] == "lot")  cout << " lotsize =  " << lotsize;
      cout << endl;
    }

    nstatus = 0;
    if (type != "product") {
      nstatus = -1;
      for (auto istat: val["doc"]["status"]) {
        string bla0 = istat["status"].dump();
        bla0 = bla0.substr(1, bla0.rfind("\"") - 1);

        string bla1 = istat["datetime"].dump();
        bla1 = bla1.substr(1, bla1.rfind("\"") - 1);
        string time = time2SqlFormat(bla1);
        cout << "    " << bla0 << " -> " << bla1 << " = " << time << endl;

        nstatus = 0;
        stState[nstatus] = bla0;
        stTime[nstatus]  = TDatime(time.c_str());
      }

      for (auto istat: val["doc"]["sizestock"]) {
        string bla2 = istat["size"].dump();
        int stocksize(-1);
        try {
          stocksize = stoi(bla.substr(1, bla.find("\"") - 1));
        }
        catch(const std::invalid_argument&) {
          stocksize = -2;
        }

        string datetime = istat["datetime"].dump();
        datetime = datetime.substr(1, datetime.rfind("\"") - 1);

        cout << "    sizestock " << stocksize
             << "  -> " << datetime
             << endl;
      }
    } else {
      nstatus = -1;
      for (auto istat: val["doc"]["history"]) {
        string bla0 = istat["what"].dump();
        bla0 = bla0.substr(1, bla0.rfind("\"") - 1);

        string bla1 = istat["datetime"].dump();
        bla1 = bla1.substr(1, bla1.rfind("\"") - 1);
        string time = time2SqlFormat(bla1);
        cout << "    " << bla0 << " -> " << bla1 << " = " << time << endl;

        nstatus = 0;
        stState[nstatus] = bla0;
        stTime[nstatus]  = TDatime(time.c_str());
      }

    }

    pdbTree->Fill();
  }

  pdbFile->Write();
  pdbFile->Close();
}


// ----------------------------------------------------------------------
int main(int argc, char *argv[]) {
  string filename("pix.json");
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i],"-f"))  {filename = argv[++i];}
  }

  readJSON(filename);
  return 0;
}
