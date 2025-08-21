#ifndef anaFrameTree_h
#define anaFrameTree_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <string>
#include <TH1D.h>
#include <TH2D.h>
#include <TProfile.h>
#include <TProfile2D.h>

// Header file for the classes stored in the TTree if any.

class anaFrameTree {
public:
    anaFrameTree(TChain *chain=0);
    virtual ~anaFrameTree();
    virtual void     loop(int nevents=-1, int start=0); 
    virtual void     startAnalysis();
    virtual void     endAnalysis();
    virtual void     bookHistograms();
    virtual void     bookVtx2D(std::string batch="vtx");
    virtual void     bookVtx1D(std::string batch="vtx", int nbins = 32, int lo = 0, int hi = 32);
    virtual void     bookVtx2DProfile(std::string batch="vtx");

    virtual void     setVerbose(int verbose) { fVerbose = verbose; }
    virtual void     setOutputDir(std::string outputDir) { fOutputDir = outputDir; }
    virtual void     openHistFile(std::string histfile);
    virtual void     closeHistFile();

private:
    int fVerbose;
    std::string fOutputDir = ".";
    std::string fHistFileName;
    TFile *fpHistFile;

    std::vector<int> fLayer1, fLayer2, fAllChips;

    std::map<std::string, TH1D*> fHistograms, fVtx1D;
    std::map<std::string, TH2D*> fHistograms2D, fVtx2D;
    std::map<std::string, TProfile*> fHistogramsProfile;
    std::map<std::string, TProfile2D*> fVtx2DProfile;


    // -- boilerplate functions from MakeClass()
    virtual Int_t    GetEntry(Long64_t entry);
    virtual Long64_t LoadTree(Long64_t entry);
    virtual void     Init();
    virtual bool     Notify();

    TChain          *fpChain;   //!pointer to the analyzed TTree or TChain
    Int_t           fCurrent; //!current Tree number in a TChain
    std::string     fChainName;
    Long64_t        fNentries;  

    Int_t           run;
    UInt_t          frameID;
    Int_t           hitN;
    Int_t           hitPixelID[10000];   //[hitN]
    Int_t           hitToT[10000];   //[hitN]
    ULong64_t       hitDebugSiData[10000];   //[hitN]
    Int_t           hitChipID[10000];   //[hitN]
    Int_t           hitCol[10000];   //[hitN]
    Int_t           hitRow[10000];   //[hitN]
    Int_t           hitTime[10000];   //[hitN]
    Int_t           hitTimeNs[10000];   //[hitN]
    Float_t         hitX[10000];   //[hitN]
    Float_t         hitY[10000];   //[hitN]
    Float_t         hitZ[10000];   //[hitN]
    Int_t           hitRawToT[10000];   //[hitN]
    Int_t           hitBitToT[10000];   //[hitN]
    Int_t           hitStatus[10000];   //[hitN]
    Int_t           hitStatusBits[10000];   //[hitN]
    Bool_t          hitValidHit[10000];   //[hitN]

    // List of branches
    TBranch        *b_run;   //!
    TBranch        *b_frameID;   //!
    TBranch        *b_hitN;   //!
    TBranch        *b_hitPixelID;   //!
    TBranch        *b_hitToT;   //!
    TBranch        *b_hitDebugSiData;   //!
    TBranch        *b_hitChipID;   //!
    TBranch        *b_hitCol;   //!
    TBranch        *b_hitRow;   //!
    TBranch        *b_hitTime;   //!
    TBranch        *b_hitTimeNs;   //!
    TBranch        *b_hitX;   //!
    TBranch        *b_hitY;   //!
    TBranch        *b_hitZ;   //!
    TBranch        *b_hitRawToT;   //!
    TBranch        *b_hitBitToT;   //!
    TBranch        *b_hitStatus;   //!
    TBranch        *b_hitStatusBits;   //!
    TBranch        *b_hitValidHit;   //!

};

#endif

