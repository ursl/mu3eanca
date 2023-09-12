#include "AnaPixelHistos.h"

#include "HitVectorFlowEvent.h"

#include "TH1F.h"
#include "TH1D.h"
#include "TH2F.h"
#include "TH2D.h"

#include "root_helpers.h"


AnaPixelHistos::AnaPixelHistos(TARunInfo* runinfo):
    TARunObject(runinfo){
    fModuleName = "PixelHistos";
}

AnaPixelHistos::~AnaPixelHistos(){};

void AnaPixelHistos::BeginRun(TARunInfo* runinfo)
{
        printf("PixelHistos::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());

        char xfilename[256];
        sprintf(xfilename, "root_output_files/pixel_histos%05d.root", runinfo->fRunNo);
        dataFile = new TFile(xfilename, "RECREATE");

        // select correct ROOT directory
        /*TDirectory *d_root =*/ make_or_get_dir("", gDirectory);

        // histograms
        chipID          = new TH1F("chipID", "chipID", 1024, 0, 1<<16);
}


 void AnaPixelHistos::EndRun(TARunInfo* runinfo) 
 {
        printf("PixelHistos::EndRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());

        dataFile->Write();
        dataFile->Close();
}


TAFlowEvent* AnaPixelHistos::AnalyzeFlowEvent(TARunInfo*, TAFlags*, TAFlowEvent* flow)
{
    if (!flow)
        return flow;

    HitVectorFlowEvent * hitevent = flow-> Find<HitVectorFlowEvent>();

    if(!hitevent)
        return flow;

    for(auto & hit: hitevent->pixelhits){
        chipID->Fill(hit.chipid());
    }

      // // here we could send stuff to midas ()
        // TMEvent* event = new TMEvent();
        // event->Init(100, 0xFFFF, 1, 0, 0);
        // uint32_t test_data[] = { 0x11112222, 0x33334444, 0x55556666, 0x77778888 };
        // event->AddBank("QANA", TID_UINT32, (const char*)test_data, sizeof(test_data));

        // //event->PrintHeader();

        // if (fEq) {
        //     fEq->EqSendEvent(event->data);
        //     fEq->EqWriteStatistics();
        // }
    return flow;
}

void AnaPixelHistoFactory::Init(const std::vector<std::string> &args)
{
        //printf("Init!\n");
        //printf("Arguments:\n");
        //for (unsigned i=0; i<args.size(); i++)
            //printf("arg[%d]: [%s]\n", i, args[i].c_str());


    }
    
    
    void AnaPixelHistoFactory::Finish(){
        //printf("Finish!\n");
    }
