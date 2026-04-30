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
  };

  struct histDeco {
    int lineStyle;
    Color_t lineColor;
    int lineWidth;
    int markerStyle;
    int markerSize;
  };

  // Skeleton plotting helpers to extend.
  void plotOverlay(const plotInfo &plot);
  void plotDifference(const plotInfo &plot);
  void plotRatio(const plotInfo &plot);
  void decorateHist(TH1 *h, const histDeco &deco);
  void addDoubleStatsBox(const plotInfo &plot);

  TH1 *getHist(TFile *f, const std::string &name);

  TFile *fInFile1 = nullptr;
  TFile *fInFile2 = nullptr;

  TCanvas *fCanvas = nullptr;

  std::string fInFileName1;
  std::string fInFileName2;
  std::string fDirName;

  double fEvents1, fEvents2;
  double fHiTracks1, fHiTracks2;
  std::string fSetup1, fSetup2;

  std::map<std::string, plotInfo> fPlots;
  std::vector<histDeco> fHistDecos;
};

#endif
