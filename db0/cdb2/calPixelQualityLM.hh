#ifndef CALPIXELQUALITYLM_h
#define CALPIXELQUALITYLM_h

#include "calAbs.hh"

#include <string>
#include <map>


// ----------------------------------------------------------------------
// pixel quality class using 3+1 link words and maps for dead cols and pixels 
// ----------------------------------------------------------------------
class calPixelQualityLM : public calAbs {
public:

  calPixelQualityLM() = default;
  calPixelQualityLM(cdbAbs *db);
  calPixelQualityLM(cdbAbs *db, std::string tag);
  ~calPixelQualityLM();

  // -- direct accessors
  uint32_t id(uint32_t id) {return fMapConstants[id].id;}

  std::string getName() override {return fPixelQualityTag;}
  void        calculate(std::string hash) override;

  std::string makeBLOB() override;
  // -- these are identical to the code in calPixelQuality (but this does not derive from that)
  std::string makeBLOB(const std::map<unsigned int, std::vector<double>>&) override;
  std::map<unsigned int, std::vector<double> > decodeBLOB(std::string) override;
  void printBLOB(std::string s, int verbosity = 1) override;

  void readCsv(std::string filename); 
  void writeCsv(std::string filename);


  int         getStatus(unsigned int chipid, int icol, int irow) override;
  int         getColStatus(unsigned int chipid, int icol);
  int         getLinkStatus(unsigned int chipid, int ilink);

  int         getNpixWithStatus(unsigned int chipid, int status);

  bool        getNextID(uint32_t &ID);
  void        resetIterator() {fMapConstantsIt = fMapConstants.begin();}

  void        printPixelQuality(unsigned int chipid, int minimumStatus = 0);

  std::string getSchema() override {return fSchema;}

private:
  std::string fPixelQualityTag{"pixelqualitylm_"};
  std::string fSchema{"ui_id,ui_ckdivend,ui_ckdivend2,ui_linkA,ui_linkB,ui_linkC,ui_linkM,i_ncol[,i_col,ui_qual],i_npix[,i_col,i_row,ui_qual]"};

  // -- local and private
  struct constants {
    uint32_t id;
    uint32_t ckdivend, ckdivend2;
    uint32_t linkA, linkB, linkC, linkM;
    std::map<int, char> mcol;
    std::map<int, char> mpixel;
  };

  std::map<uint32_t, constants> fMapConstants;
  std::map<uint32_t, constants>::iterator fMapConstantsIt{fMapConstants.end()};
};

#endif
