//

#include "Mu3eRunAction.h"

#include <mu3e/util/log.hpp>

#include "Mu3eEvent.h"
#include "Mu3eTargetSD.h"
#include "Mu3eTrackerSD.h"
#include "Mu3eTileSD.h"
#include "Mu3eTileSipmSD.h"
#include "Mu3eFibreSD.h"
#include "Mu3eFibreMppcSD.h"
#include "Mu3eFibreSmbSD.h"
#include "Mu3eKaptonFlapSD.h"

#include "G4Run.hh"
#include <G4RunManager.hh>
#include "G4SDManager.hh"

#include <CLHEP/Random/Random.h>

Mu3eRunAction::Mu3eRunAction(Mu3eDetectorCfg & _detcfg)
    : detcfg(_detcfg)
    , messenger("/mu3e/run", "Run control.")
{
    initRandom = true;

    messenger.createCommand("setRunNumber")
        .guidance("Set run number.")
        .states(G4State_PreInit, G4State_Idle)
        .parameter("runnumber", 'i')
        .callback([&] (const std::string& value) {
            G4RunManager::GetRunManager()->SetRunIDCounter(atoi(value.c_str()));
        });

    messenger.createCommand("setEventNumber")
        .guidance("Set event number.")
        .states(G4State_PreInit, G4State_Idle)
        .parameter("eventnumber", 'i')
        .callback([&] (const std::string& /*value*/) {
//            SetNextEventNumber(atoi(value.c_str()));
        });

    messenger.createCommand("setRandomState")
        .guidance("Restore random number generator state.")
        .states(G4State_PreInit, G4State_Idle)
        .parameter("GeneratorState")
        .callback([&] (const std::string& value) {
            RestoreGenerator(value);
        });

}

void Mu3eRunAction::BeginOfRunAction(const G4Run* run) {
    int runId = run->GetRunID();
    printf("INFO: Start of run %d\n", runId);

    if(initRandom) {
        CLHEP::HepRandom::setTheSeed(runId);
    }

    if(!file) {
        std::string fileName = fmt::format("data/mu3e_run_{:06}.root", runId);
        file = std::make_unique<mu3e::RootFile>(fileName, "RECREATE");
        mu3e::log::info("[RunAction] output = '%s'\n", fileName.c_str());
    }
    if(!file->IsOpen()) {
        printf("FATAL: Failed to create file: %s\n", file->GetName());
        exit(1);
    }
    file->config.ptree.put("sim.version", BOOST_PP_STRINGIZE(MU3E_VERSION));
    file->config.read(Mu3eDetectorCfg::Instance().ptree);
    file->config.read(Mu3eDigiCfg::Instance().ptree);

    auto alignmentDir = file->mkdir("alignment");

    auto smallSensorSD = mu3e::g4::SDManager::findSD<Mu3eTrackerSD>("mu3e/SmallSilicon");
    auto largeSensorSD = mu3e::g4::SDManager::findSD<Mu3eTrackerSD>("mu3e/LargeSilicon");

    Mu3eTrackerSD::writeAlignment(smallSensorSD, alignmentDir, "sensors");
    Mu3eTrackerSD::writeAlignment(largeSensorSD, alignmentDir, "sensors");

    auto tileSD = mu3e::g4::SDManager::findSD<Mu3eTileSD>("mu3e/TileSensorSD");
    if(tileSD) {
        tileSD->WriteAlignment(tileSD, alignmentDir, "tiles");
    } else {
        G4cout << "Mu3eRunAction: WARNING : Tiles alignment not written!" << G4endl;
    }

    auto fibreSD = mu3e::g4::SDManager::findSD<Mu3eFibreSD>("mu3e/FibreSD");
    if(fibreSD) {
        Mu3eFibreSD::WriteFibreAlignment(fibreSD, alignmentDir, "fibres");
        Mu3eFibreSD::WriteMPPCAlignment(fibreSD, alignmentDir, "mppcs");
    }
    else {
        G4cout << "Mu3eRunAction: WARNING : Fibre/Mppc alignment not written!" << G4endl;
    }

    auto targetSD = mu3e::g4::SDManager::findSD<Mu3eTargetSD>("mu3e/TargetSD");
    if(targetSD) {
        targetSD->writeAlignment(alignmentDir, detcfg);
    }

    file->cd();
    tree = new TTree("mu3e","mu3e");

    ev = Mu3eEvent::GetInstance();
    ev->ResetCounts();
    ev->runId = runId;
    ev->SetBranches(tree);

    ev->startTime = 0;

    initRandom = true;
}

void Mu3eRunAction::RestoreGenerator(const std::string& saved) {
    std::stringstream s;
    s << saved;
    CLHEP::HepRandom::restoreFullState(s);

    initRandom = false;
}

void Mu3eRunAction::EndOfRunAction(const G4Run* run) {
    int runId = run->GetRunID();
    printf("INFO: End of run %d\n", runId);

    std::string output =
        fmt::format("Muons generated: primaries = {}\n", ev->GetNPrimaries()) +
        fmt::format("Muons decayed:\n") +
        fmt::format("    afterWindow = {}\n", ev->GetNAfterWindow()) +
        fmt::format("    beforeTarget = {} (fraction = {:.3f})\n", ev->GetNBeforeTarget(), 1.0 * ev->GetNBeforeTarget() / ev->GetNPrimaries()) +
        fmt::format("    inTarget = {}\n", ev->GetNIntarget()) +
        fmt::format("    afterTarget = {}\n", ev->GetNAfterTarget()) +
        fmt::format("Target stopping fraction:\n") +
        fmt::format("    inTarget/(inTarget+afterTarget) = {:.3f}\n", 1.0 * ev->GetNIntarget() / (ev->GetNIntarget() + ev->GetNAfterTarget())) +
        fmt::format("    inTarget/afterWindow = {:.3f}\n", 1.0 * ev->GetNIntarget() / ev->GetNAfterWindow()) +
        fmt::format("    inTarget/primaries = {:.3f}\n", 1.0 * ev->GetNIntarget() / ev->GetNPrimaries()) +
        "";

    std::cout << output << std::endl;

    // write stats
    TDirectory* stat = file->mkdir("stat");
    stat->cd();

    TObjString obj_simoutput(output.c_str());
    gDirectory->WriteObject(&obj_simoutput, "muonstat");

    auto pixelSD = mu3e::g4::SDManager::findSD<Mu3eTrackerSD>("mu3e/SmallSilicon");
    if(pixelSD) pixelSD->writeStat();
    auto fibreSD = mu3e::g4::SDManager::findSD<Mu3eFibreSD>("mu3e/FibreSD");
    if(fibreSD) fibreSD->writeStat();
    auto fibreMppcSD = mu3e::g4::SDManager::findSD<Mu3eFibreMppcSD>("mu3e/FibreMppcSD");
    if(fibreMppcSD) fibreMppcSD->writeStat();
    auto fibreSmbSD = mu3e::g4::SDManager::findSD<Mu3eFibreSmbSD>("mu3e/FibreSmbSD");
    if(fibreSmbSD) fibreSmbSD->writeStat();
    auto tileSD = mu3e::g4::SDManager::findSD<Mu3eTileSD>("mu3e/TileSensorSD");
    if(tileSD) tileSD->writeStat();
    auto tilesipmSD = mu3e::g4::SDManager::findSD<Mu3eTileSipmSD>("mu3e/TileSipmSD");
    if(tilesipmSD) tilesipmSD->writeStat();

    int Nmuons_target = ev->GetNIntarget(); //number of stopped muons for normalization of kapton dose calculation

    auto kaptonFlapSD = mu3e::g4::SDManager::findSD<Mu3eKaptonFlapSD>("mu3e/KaptonFlapSD");
    if(kaptonFlapSD) kaptonFlapSD->writeStat(Nmuons_target);

    file->cd();

    if(ev->tree_mchits) ev->tree_mchits->SetEntries();

    file->Write();
    file->Close();
    file.reset();
}
