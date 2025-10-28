#ifndef ASICINFO_h
#define ASICINFO_h  
// ----------------------------------------------------------------------
struct AsicInfo {
  int confId{};
  int globalId{};
  int fedID{};
  int idxInSection{}; // position within the section (0..)
  std::string FEBLinkName;
  std::string linkMask;
  std::string linkMatrix;
  long long lvdsErrRate0{};
  long long lvdsErrRate1{};
  long long lvdsErrRate2{};
  // -- ABC link information (the above resorted with linkMask)
  int         abcLinkMask[3]{0,0,0};
  long long   abcLinkErrs[3]{0LL,0LL,0LL};
  std::string abcLinkNames[3]{"unset", "unset", "unset"};
  int         abcLinkOffsets[3]{9,9,9};
  int ckdivend{};
  int ckdivend2{};
  int vdacBLPix{};
  int vdacThHigh{};
  int vdacThLow{};
  int biasVNOutPix{};
  int biasVPDAC{};
  int biasVNDcl{};
  int biasVNLVDS{};
  int biasVNLVDSDel{};
  int biasVPDcl{};
  int biasVPTimerDel{};
  int vdacBaseline{};
};
#endif