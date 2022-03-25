// ----------------------------------------------------------------------
// r2m -- root2Midas
//
// Usage: .L r2m 
// -----
// ----------------------------------------------------------------------

#include <fstream>
#include "/psi/home/langenegger/data/mu3e-analyzer/analyzer/utility/json.h"


using json = nlohmann::json;

// ----------------------------------------------------------------------
vector<unsigned int>mupixPayload(vector<unsigned int> *vPixelID, vector<unsigned int> *vTimeStamp) {
  vector<unsigned int> payload;

  unsigned int preamble(0xE80000BC), trailer(0xFC00009C);
  assert(vPixelID->size() == vTimeStamp->size());

  // -- preamble
  payload.push_back(preamble);
  
  unsigned int irow(0), icol(0), isen(0), itot(31);
  unsigned int ts47_16(0), ts15_00(0), ts10_04(0), ts03_00(0); 
  unsigned int oldts47_16(1), oldts15_00(1), oldts10_04(1), oldts03_00(1); 
  unsigned int overflow(0xDEAD);
  unsigned int word(0);

  ULong64_t timestamp = static_cast<ULong64_t>(vTimeStamp->at(0));
  unsigned int pixelid   = vPixelID->at(0);
  ts47_16 = (timestamp >> 16) & 0xffffffff;
  ts15_00 = (timestamp & 0xffff) << 16;
  ts10_04 = (timestamp >> 4) & 0x7f;
  ts03_00 = timestamp & 0xfULL;

  // -- MuPix Data Header
  payload.push_back(ts47_16);
  payload.push_back(ts15_00);

  for (unsigned int ihit = 0; ihit < vPixelID->size(); ++ihit) {
    timestamp = static_cast<ULong64_t>(vTimeStamp->at(ihit));
    pixelid   = vPixelID->at(ihit);

    irow = pixelid & 0xff;
    icol = (pixelid >> 8) & 0xff;
    isen = (pixelid >> 16) & 0xffff;
    
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
  map<int, int> simChip2irChip;
  for (json::iterator it = jMap.begin(); it != jMap.end(); ++it) {
    it->at("intRunChip").get_to(intRunChip);
    it->at("simChip").get_to(simChip);
    cout << "intRunChip = " << intRunChip << " simChip = " << simChip << endl;
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
      irow = pixelid & 0xff;
      icol = (pixelid >> 8) & 0xff;
      isen = (pixelid >> 16) & 0xffff;
      
      ts47_16 = (timestamp >> 16) & 0xffffffff;
      ts15_00 = timestamp & 0xffffULL;
      ts10_04 = (timestamp >> 4) & 0x7fULL;
      ts03_00 = timestamp & 0xfULL;

      cout << hex << pixelid << " " << dec;
      //      cout << " (" << isen << " aka " << simChip2irChip[isen] << ": " << icol << "/" << irow << ")";
      cout << "TS=" << hex << timestamp << dec << "(" << timestamp << ")" << hex 
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
    for (unsigned ip = 0; ip < payload.size(); ++ip) {
      cout << Form("%3d: %08x", ip, payload[ip]) << endl;
    }

  }

  
}
