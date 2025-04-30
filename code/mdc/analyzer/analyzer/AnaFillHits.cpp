#include "AnaFillHits.h"
#include "HitVectorFlowEvent.h"

AnaFillHits::AnaFillHits(TARunInfo* runinfo) : TARunObject(runinfo)
{
    fModuleName = "AnaFillHits";
};

AnaFillHits::~AnaFillHits(){};

void AnaFillHits::BeginRun(TARunInfo* runinfo){
    printf("AnaFillHits::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
};

void AnaFillHits::EndRun(TARunInfo* runinfo) {
        printf("AnaFillHits::EndRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
};

TAFlowEvent* AnaFillHits::Analyze(TARunInfo* runinfo, TMEvent* event, 
    TAFlags* flags, TAFlowEvent* flow)
    {

        uint64_t * pdata = 0;
        uint64_t * fdata = 0;
        uint64_t * tdata = 0;
        
        uint32_t psize =0;
        uint32_t fsize =0;
        uint32_t tsize =0;

        TMBank *pbank = event->FindBank("PHIT");
        if (pbank){
            pdata = (uint64_t *) event->GetBankData(pbank);
            psize = pbank->data_size / 8;
        }
        
        TMBank *fbank = event->FindBank("FHIT");
        if (fbank){
            fdata = (uint64_t *) event->GetBankData(fbank);
            fsize = fbank->data_size / 8;
        }

        TMBank *tbank = event->FindBank("THIT");
        if (tbank){
            tdata = (uint64_t *) event->GetBankData(tbank);
            tsize = tbank->data_size / 8;
        }

        flow = new HitVectorFlowEvent(flow, pdata, psize, fdata, fsize, tdata, tsize);
        return flow;
    }