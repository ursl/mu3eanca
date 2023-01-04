//

#include "G4RunManager.hh"
#include "G4UImanager.hh"

#include <mu3e/sim/Mu3eDetectorConstruction.h>

#include <G4VisExecutive.hh>
#include <G4UIExecutive.hh>
#include <G4GenericPhysicsList.hh>

int main(int argc, char* argv[]) {
    std::vector<std::string> confFiles;

    if(confFiles.empty()) confFiles.emplace_back("simvis.json");
    //    if(confFiles.empty()) confFiles.emplace_back("sim.json");

    auto& detcfg = Mu3eDetectorCfg::Instance();
    auto& digicfg = Mu3eDigiCfg::Instance();
    if(!confFiles.empty()) {
        boost::property_tree::ptree pt;
        for(auto& conf_file : confFiles) {
            mu3e::util::read_conf(conf_file, pt);
        }
        detcfg.ptree = pt;
        digicfg.ptree = pt;
    }

    auto runManager = new G4RunManager;

    auto detector = new Mu3eDetectorConstruction(detcfg,digicfg);
    runManager->SetUserInitialization(detector);

    auto physicsList = new G4GenericPhysicsList();
    runManager->SetUserInitialization(physicsList);

    runManager->Initialize();

    auto visManager = new G4VisExecutive;
    visManager->Initialize();

    auto UImanager = G4UImanager::GetUIpointer();

    auto ui = new G4UIExecutive(argc,argv);
    // G4UIsession* session = new G4UIQt(argc, argv);

    UImanager->ApplyCommand("/control/execute vis.mac");
    //session->SessionStart();
    ui->SessionStart();
    delete ui;

    delete visManager;
    delete runManager;

    return 0;
}
