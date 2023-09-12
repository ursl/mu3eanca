#include "manalyzer.h"

#include "AnaFillHits.h"
#include "AnaPixelHistos.h"

static TARegister tar1(new TAFactoryTemplate<AnaFillHits>);
static TARegister tar2(new AnaPixelHistoFactory);
