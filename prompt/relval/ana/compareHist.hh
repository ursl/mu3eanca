#ifndef COMPAREHIST_HH
#define COMPAREHIST_HH

#include <map>
#include <string>
#include <vector>

#include <TH1.h>
#include <TH1D.h>
#include <TFile.h>

class TCanvas;

class compareHist {
public:
  compareHist(const std::string &infile1, const std::string &infile2);
  ~compareHist();

  void setupHists(std::string mode = "relval");
  void run(std::string dirname = ".");
  void makeSummaryPDF(std::string dirname = ".");

private:
  struct plotInfo {
    std::string name;
    TH1 *h1;
    TH1 *h2;
    bool valid{false};
    bool logY{false};
    bool statsBox{false};
    double ks{0.};
    double ad{0.};
  };

  struct histDeco {
    int lineStyle;
    Color_t lineColor;
    int lineWidth;
    int markerStyle;
    int markerSize;
  };

  // Skeleton plotting helpers to extend.
  void plotOverlay(plotInfo &plot);
  void plotDifference(plotInfo &plot);
  void plotRatio(plotInfo &plot);
  void decorateHist(TH1 *h, const histDeco &deco);
  void addDoubleStatsBox(const plotInfo &plot);
  void replaceAll(std::string &str, const std::string &from, const std::string &to);

  TH1 *getHist(TFile *f, const std::string &name);

  TFile *fInFile1 = nullptr;
  TFile *fInFile2 = nullptr;

  TCanvas *fCanvas = nullptr;

  std::string fInFileName1, fInFileName2;
  std::string fDirName, fMode;

  double fEvents1, fEvents2;
  double fHiTracks1, fHiTracks2;
  std::string fSetup1, fSetup2;
  std::string fSimVersion1, fSimVersion2;
  std::string fSimConf1, fSimConf2;
  std::string fTriRecVersion1, fTriRecVersion2;
  std::string fTriRecConf1, fTriRecConf2;
  std::string fCdbDbconn1, fCdbDbconn2;
  std::string fCdbGlobalTag1, fCdbGlobalTag2;

  int fKSPassed, fKSFailed;
  double fKSThreshold;

  int fADPassed, fADFailed;
  double fADThreshold;

  std::map<std::string, plotInfo> fPlots;
  std::vector<histDeco> fHistDecos;
};

#endif
