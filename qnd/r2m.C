// ----------------------------------------------------------------------
// r2m -- root2Midas
//
// Usage: root filename.root
// -----  .L r2m.C
//        v0()
// ----------------------------------------------------------------------

#include <fstream>
#include "/psi/home/langenegger/data/mu3e-analyzer/analyzer/utility/json.h"


using json = nlohmann::json;

// ----------------------------------------------------------------------
// -- holds sensorID for integration run detector from tree alignment/sensors
// ----------------------------------------------------------------------
map<std::string, std::vector<unsigned int> > simSensorId = {
  // -- ladder numbering adheres to Ben's table, not to the bits encoded in
  //      https://bitbucket.org/mu3e/mu3e/src/dev/mu3eTrirec/src/SiDet.cpp
  //    sensors go along increasing z, i.e. [0] is the most upstream sensor
  {"layer0:ladder0", {   1,   2,   3,   4,   5,   6}},
  {"layer0:ladder1", {  33,  34,  35,  36,  37,  38}},
  {"layer0:ladder2", {  65,  66,  67,  68,  69,  70}},
  {"layer0:ladder3", {  97,  98,  99, 100, 101, 102}},
  {"layer0:ladder4", { 257, 258, 259, 260, 261, 262}},
  {"layer0:ladder5", { 289, 290, 291, 292, 293, 294}},
  {"layer0:ladder6", { 321, 322, 323, 324, 325, 326}},
  {"layer0:ladder7", { 353, 354, 355, 356, 357, 358}},

  {"layer1:ladder0", { 1025, 1026, 1027, 1028, 1029, 1030}},
  {"layer1:ladder1", { 1057, 1058, 1059, 1060, 1061, 1062}},
  {"layer1:ladder2", { 1089, 1090, 1091, 1092, 1093, 1094}},
  {"layer1:ladder3", { 9999, 9999, 9999, 9999, 9999, 9999}},
  {"layer1:ladder4", { 1153, 1154, 1155, 1156, 1157, 1158}},
  {"layer1:ladder5", { 1185, 1186, 1187, 1188, 1189, 1190}},
  {"layer1:ladder6", { 1281, 1282, 1283, 1284, 1285, 1286}},
  {"layer1:ladder7", { 1313, 1314, 1315, 1316, 1317, 1318}},
  {"layer1:ladder8", { 1345, 1346, 1347, 1348, 1349, 1350}},
  {"layer1:ladder9", { 9999, 9999, 9999, 9999, 9999, 9999}},
  {"layer1:ladder10",{ 1409, 1410, 1411, 1412, 1413, 1414}},
  {"layer1:ladder11",{ 1441, 1442, 1443, 1444, 1445, 1446}}
};

// ----------------------------------------------------------------------
// -- holds simChipId for integration run detector from
//    https://bitbucket.org/mu3e/analyzer/src/midas_to_root/analyzer/config/sensors_intRun2021.json
// ----------------------------------------------------------------------
map<std::string, std::vector<unsigned int> > simChipId = {
  // -- ladder numbering adheres to Ben's table, not to the bits encoded in
  //      https://bitbucket.org/mu3e/mu3e/src/dev/mu3eTrirec/src/SiDet.cpp
  //    sensors go along increasing z, i.e. [0] is the most upstream sensor
  {"layer0:ladder0", {   5,   4,   3,   2,   1,   0}},
  {"layer0:ladder1", {  47,  46,  45,  44,  43,  42}},
  {"layer0:ladder2", {  41,  40,  39,  38,  37,  36}},
  {"layer0:ladder3", {  35,  34,  33,  32,  31,  30}},
  {"layer0:ladder4", {  29,  28,  27,  26,  25,  24}},
  {"layer0:ladder5", {  23,  22,  21,  20,  19,  18}},
  {"layer0:ladder6", {  17,  16,  15,  14,  13,  12}},
  {"layer0:ladder7", {  11,  10,   9,   8,   7,   6}},

  {"layer1:ladder0", {   53,   53,   51,   50,   49,   48}},
  {"layer1:ladder1", {  107,  106,  105,  104,  103,  102}},
  {"layer1:ladder2", {  101,  100,   99,   98,   97,   96}},
  {"layer1:ladder3", {  999,  999,  999,  999,  999,  999}},
  {"layer1:ladder4", {   95,   94,   93,   92,   91,   90}},
  {"layer1:ladder5", {   89,   88,   87,   86,   85,   84}},
  {"layer1:ladder6", {   83,   82,   81,   80,   79,   78}},
  {"layer1:ladder7", {   77,   76,   75,   74,   73,   72}},
  {"layer1:ladder8", {   71,   70,   69,   68,   67,   66}},
  {"layer1:ladder9", { 9999, 9999, 9999, 9999, 9999, 9999}},
  {"layer1:ladder10",{   65,   64,   63,   62,   61,   60}},
  {"layer1:ladder11",{   59,   58,   57,   56,   55,   54}}
};



// ----------------------------------------------------------------------
vector<unsigned int>mupixPayload(vector<unsigned int> *vPixelID, vector<unsigned int> *vTimeStamp) {
  vector<unsigned int> payload;

  unsigned int preamble(0xE80000BC), trailer(0xFC00009C);
  assert(vPixelID->size() == vTimeStamp->size());

  // -- sort the two vectors such that timestamp is strictly increasing
  vector<unsigned int> vid, vts;
  vid.push_back(vPixelID->at(0)); 
  vts.push_back(vTimeStamp->at(0)); 
  for (unsigned int ihit = 1; ihit < vTimeStamp->size(); ++ihit) {
    for (unsigned int i = 0; i < vts.size(); ++i) {
      if ((vTimeStamp->at(ihit) < vts[i]) && (i < vts.size())) {
	auto it = vts.insert(vts.begin()+i, vTimeStamp->at(ihit));
	auto iu = vid.insert(vid.begin()+i, vPixelID->at(ihit));
	break;	
      }
      if (i+1 == vts.size()) {
	vts.push_back(vTimeStamp->at(ihit));
	vid.push_back(vPixelID->at(ihit));
	break;	
      }
    }
  }
  
  // -- preamble
  payload.push_back(preamble);
  
  unsigned int irow(0), icol(0), isen(0), itot(31);
  unsigned int ts47_16(0), ts15_00(0), ts10_04(0), ts03_00(0); 
  unsigned int oldts47_16(1), oldts15_00(1), oldts10_04(1), oldts03_00(1); 
  unsigned int overflow(0xDEAD);
  unsigned int word(0);

  ULong64_t timestamp = static_cast<ULong64_t>(vTimeStamp->at(0));
  // -- multiply timestamp from simulation by a factor 2
  timestamp = (timestamp << 1);
  unsigned int pixelid   = vPixelID->at(0);
  ts47_16 = (timestamp >> 16) & 0xffffffff;
  ts15_00 = (timestamp & 0xffff) << 16;
  ts10_04 = (timestamp >> 4) & 0x7f;
  ts03_00 = timestamp & 0xfULL;

  // -- MuPix Data Header
  payload.push_back(ts47_16);
  payload.push_back(ts15_00);

  for (unsigned int ihit = 0; ihit < vid.size(); ++ihit) {
    timestamp = static_cast<ULong64_t>(vts[ihit]);
    pixelid   = vid[ihit];

    irow = pixelid & 0xff;
    icol = (pixelid >> 8) & 0xff;
    isen = (pixelid >> 16) & 0xffff;
    // FIXME HERE! translate isen to irChip!
    
    ts47_16 = (timestamp >> 16) & 0xffffffff;
    ts15_00 = (timestamp & 0xffff) << 16;
    ts10_04 = (timestamp >> 4) & 0x7f;
    ts03_00 = timestamp & 0xf;

    // -- check whether TS overflowed (or whether the first sub-Header should be put out)
    if ((ts47_16 > oldts47_16)
	|| (ts15_00 > oldts15_00)
	|| (ts10_04 > oldts10_04)
	|| (ts03_00 < oldts03_00) // smaller!
	) {

      // -- sub-Header
      word  = (0xfc << 24);
      word |= (ts10_04 << 16);
      word |= overflow;
      payload.push_back(word);
    }

    // -- correct place?
    oldts47_16 = ts47_16;
    oldts15_00 = ts15_00;
    oldts10_04 = ts10_04;
    oldts03_00 = ts03_00;

    // -- hit
    word  = 0;
    word  = (ts03_00 << 28);
    word |= (isen << 21);
    word |= (irow << 13);
    word |= (icol << 5);
    word |= (itot << 0);
    payload.push_back(word);
    
  }
  payload.push_back(trailer);
  

  
  return payload;
}


// ----------------------------------------------------------------------
void v0() {
  ifstream i("/psi/home/langenegger/data/mu3e-analyzer/scripts/python_scripts/chipID_remapping/sensors_intRun2021.json");
  json jMap;
  i >> jMap;

  // -- check parsed JSON file
  int intRunChip(-1), simChip(-1);
  double vx, vy, vz;
  map<int, int> simChip2irChip;
  for (json::iterator it = jMap.begin(); it != jMap.end(); ++it) {
    // cout << *it << endl;
    it->at("intRunChip").get_to(intRunChip);
    it->at("simChip").get_to(simChip);
    it->at("v").at("x").get_to(vx);
    it->at("v").at("y").get_to(vy);
    it->at("v").at("z").get_to(vz);
    TVector3 r(vx, vy, vz);
    // -- Note: MK's JSON file has dimensions of meter, not milimeter!
    cout << Form("intRunChip = %03d simChip = %03d r = %5.2f phi = %6.1f v=(%+7.3f,%+7.3f,%+7.3f)",
		 intRunChip, simChip,
		 1.e3*TMath::Sqrt(vx*vx + vy*vy), r.Phi()*TMath::RadToDeg(),
		 1.e3*vx, 1.e3*vy, 1.e3*vz)
	 << endl;
    simChip2irChip[simChip] = intRunChip;
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


  int nmax(100);
  //  nmax = tMu3e->GetEntries();
  unsigned int irow(0), icol(0), isen(0);
  unsigned int chip(0), ladder(0), layer(0), rest(0); 
  unsigned int ts47_16(0), ts15_00(0), ts10_04(0), ts03_00(0); 
  for (int ievt = 0; ievt < nmax; ++ievt) {
    Long64_t tentry = tMu3e->LoadTree(ievt);
    b_hit_pixelid->GetEntry(tentry); 
    b_hit_timestamp->GetEntry(tentry); 
    b_Nhit->GetEntry(tentry); 
    if (Nhit < 1) continue;
    cout << Form("%5d", ievt)
	 << " Nhit = " << Nhit << ", pixelid = "; 
    for (int ihit = 0; ihit < Nhit; ++ihit) {
      unsigned int pixelid   = v_hit_pixelid->at(ihit);
      ULong64_t timestamp = static_cast<ULong64_t>(v_hit_timestamp->at(ihit));
      // -- multiply timestamp from simulation by a factor 2 (also for printout here)
      timestamp = (timestamp << 1);
      irow = pixelid & 0xff;
      icol = (pixelid >> 8) & 0xff;
      isen = (pixelid >> 16) & 0xffff;

      chip   = (isen & 0x1f);
      ladder = (isen >> 5) & 0x1f;
      layer  = (isen >> 10) & 0x1f;
      if (0 == layer) {
	if (ladder > 4) ladder -= 4; 
      } else if (1 == layer) {
	if (ladder > 5) ladder -= 2; 
      } else {
	cout << "Error: should not reach this point, skipping hit" << endl;
	continue;
      }
      // cout << "getting " << Form("layer%d:ladder%d", layer, ladder)  << endl;
      simChip = simChipId[Form("layer%d:ladder%d", layer, ladder)][chip-1];
      
      rest   = (isen >> 15);
      
      ts47_16 = (timestamp >> 16) & 0xffffffff;
      ts15_00 = timestamp & 0xffffULL;
      ts10_04 = (timestamp >> 4) & 0x7fULL;
      ts03_00 = timestamp & 0xfULL;
      
      cout << hex << pixelid << " " << dec;
      cout << " (" << isen
	   << " simChip: " << simChip
	   << " irChip: " << simChip2irChip[simChip]
	   << " c/r: " << icol << "/" << irow << ")";
      cout << Form(" chip/ladder/layer = %2d/%2d/%2d ", chip, ladder, layer);
      cout << " rest = " << rest << " "; 
      if (0) cout << "TS=" << hex << timestamp << dec << "(" << timestamp << ")" << hex 
		  << " ts47_16=" << ts47_16
		  << " ts15_00=" << ts15_00
		  << " ts10_04=" << ts10_04
		  << " ts03_00=" << ts03_00
		  << " " 
	       ;
      // specbook:
      // First the preamble is sent. After this the MuPix Data Header is sent. This contains the 48-bit timestamp.
      // After this the sub-Header is sent, which contains an indicator (111111), the (10:4) bits of the timestamp
      // and 16 bits for the overflow indicators of the hit sorter. Note that the overflow bits refer to the preceeding
      // block of 16 timestamps, i.e. for block 0, the overflow bits are meaningless and the overflow bits for the
      // last block are sent with the trailer. After this, hits are sent until the lowest 4 bits of the 48-bit timestamp
      // overflow. Then the sub-Header is sent again until all 10 bits overflow. After this a package is finished and
      // the trailer (K28.4 plus remaining overflow bits) will be sent. 
    }

    cout << endl;

    // -- get the payload
    vector<unsigned int>payload = mupixPayload(v_hit_pixelid, v_hit_timestamp);
    // -- print it
    for (unsigned ip = 0; ip < payload.size(); ++ip) {
      cout << Form("%3d: %08x", ip, payload[ip]) << endl;
    }

  }

  
}
