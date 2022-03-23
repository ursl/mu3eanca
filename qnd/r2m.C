// ----------------------------------------------------------------------
// r2m -- root2Midas
//
// Usage: .L r2m 
// -----
// ----------------------------------------------------------------------

#include <fstream>
#include "/psi/home/langenegger/data/mu3e-analyzer/analyzer/utility/json.h"


using json = nlohmann::json;

// namespace ns {
//   struct sensorInfo {
//     int id;
//     int intRunChip, simChip;
//     vector<double> v, drow, dcol;
//     int nrow, ncol;
//     double width, length, thickness, pixelSize;
//   };

//   void to_json(json &j, const sensorInfo &p) {
//     j = json{{"id", p.id},
// 	     {"intRunChip", p.intRunChip},
// 	     {"simChip", p.simChip}
//     }
//   }

//   void from_json(const json &j, sensorInfo &p) {
//     j.at("id").get_to(p.id);
//     j.at("intRunChip").get_to(p.intRunChip);
//     j.at("simChip").get_to(p.simChip);
//   }

// }

// ----------------------------------------------------------------------
void v0() {
  ifstream i("/psi/home/langenegger/data/mu3e-analyzer/scripts/python_scripts/chipID_remapping/sensors_intRun2021.json");
  json jMap;
  i >> jMap;
  
  // -- check parsed JSON file
  int intRunChip(-1), simChip(-1);
  for (json::iterator it = jMap.begin(); it != jMap.end(); ++it) {
    it->at("intRunChip").get_to(intRunChip);
    it->at("simChip").get_to(simChip);
    cout << "intRunChip = " << intRunChip << " simChip = " << simChip << endl;
  }

  // -- get trees
  TTree *tMu3e    = (TTree*)gFile->Get("mu3e");
  cout << "read Mu3e tree with " << tMu3e->GetEntries() << " entries" << endl;
  TTree *tSensors = (TTree*)gFile->Get("alignment/sensors");
  cout << "read alignment/sensors tree with " << tSensors->GetEntries() << " entries" << endl;

  int Nhit; 
  TBranch *b_Nhit(0);
  std::vector<unsigned int>  *v_hit_pixelid(0), *v_hit_timestamp(0);
  TBranch *b_hit_pixelid(0), *b_hit_timestamp(0);
  
  tMu3e->SetBranchAddress("Nhit", &Nhit, &b_Nhit);
  tMu3e->SetBranchAddress("hit_pixelid", &v_hit_pixelid, &b_hit_pixelid);
  tMu3e->SetBranchAddress("hit_timestamp", &v_hit_timestamp, &b_hit_timestamp);


  int nmax(20);
  // nmax = tMu3e->GetEntries();
  for (int ievt = 0; ievt < nmax; ++ievt) {
    Long64_t tentry = tMu3e->LoadTree(ievt);
    b_hit_pixelid->GetEntry(tentry); 
    b_hit_timestamp->GetEntry(tentry); 
    b_Nhit->GetEntry(tentry); 
    if (Nhit < 1) continue;
    cout << Form("%5d", ievt)
	 << " Nhit = " << Nhit << ", pixelid = "; 
    for (int ihit = 0; ihit < Nhit; ++ihit) {
      //      int sensorID = v_hit_pixelid->at(ihit) & 0xff
      cout << hex << v_hit_pixelid->at(ihit) << " " << dec;
    }
    cout << " ts = ";
    for (int ihit = 0; ihit < Nhit; ++ihit) {
      cout << v_hit_timestamp->at(ihit) << " ";
    }
    cout << endl;
  }

  
}
