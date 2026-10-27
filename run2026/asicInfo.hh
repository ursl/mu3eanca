#ifndef ASICINFO_h
#define ASICINFO_h

#include <string>

// ----------------------------------------------------------------------
// Sensor columns: A = 1-88, B = 89-172, C = 173-256.
//
// linkMask, linkMatrix, lvdsErrRate0/1/2 and the abcLink* arrays use
// right-hand indices. Index 0 is the rightmost character or number in
// the midas JSON (lvdsErrRates included; that array is printed one
// value per line, and the last value is index 0).
//
// A letter maps to an index only when that letter occurs once. 'E' does
// not map. Two or more 'E' leave at most one index mapped to a letter,
// and a letter may have no index.
// ----------------------------------------------------------------------
struct AsicInfo {
  int confId{};
  int globalId{};
  int fedID{};
  int idxInSection{}; // position within the section (0..)
  std::string FEBName;
  std::string FEBLinkName;
  std::string linkMask;
  std::string linkMatrix;
  // Right-hand order: 0 = rightmost JSON entry.
  long long lvdsErrRate0{};
  long long lvdsErrRate1{};
  long long lvdsErrRate2{};
  // abcLink*[i] is right-hand index i, not sensor A/B/C.
  // abcLinkMatrix: 0=A, 1=B, 2=C, 4=E.
  int         abcLinkMask[3]{0,0,0};
  long long   abcLinkErrs[3]{0LL,0LL,0LL};
  int         abcLinkMatrix[3]{4,4,4};
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

  // True when letter (A, B, or C) occurs once. index is its right-hand index.
  bool letterToIndex(char letter, int& index) const {
    if (letter < 'A' || letter > 'C') return false;
    const int n = static_cast<int>(linkMatrix.size());
    int found = -1;
    int hits = 0;
    for (int rhs = 0; rhs < n && rhs < 3; ++rhs) {
      if (linkMatrix[n - 1 - rhs] == letter) {
        ++hits;
        found = rhs;
      }
    }
    if (hits != 1) return false;
    index = found;
    return true;
  }

  // True when right-hand index holds a unique A, B, or C.
  bool indexToLetter(int index, char& letter) const {
    if (index < 0 || index > 2) return false;
    const int n = static_cast<int>(linkMatrix.size());
    const int src = n - 1 - index;
    if (src < 0 || src >= n) return false;
    const char c = linkMatrix[src];
    int idx = -1;
    if (!letterToIndex(c, idx) || idx != index) return false;
    letter = c;
    return true;
  }
};
#endif
