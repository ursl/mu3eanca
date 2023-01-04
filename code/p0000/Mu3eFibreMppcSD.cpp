//

#include "Mu3eFibreMppcSD.h"

#include "Mu3eEvent.h"

using namespace CLHEP;

Mu3eFibreMppcSD::Mu3eFibreMppcSD(const G4String& name)
    : G4VSensitiveDetector(name)
{
    edepHisto = new TH1F("hFibreMppcEdepTotal", "Edep Fibre Mppc", 1000, 0, 1);
    edepHisto->GetXaxis()->SetTitle("edep [MeV]");
    edepHisto->GetYaxis()->SetTitle("counts");
    edepSiO2Histo = new TH1F("hFibreMppcEdepSiO2Total", "Edep Fibre Mppc SiO2", 1000, 0, 1);
    edepSiO2Histo->GetXaxis()->SetTitle("edep [MeV]");
    edepSiO2Histo->GetYaxis()->SetTitle("counts");
    eHisto = new TH1F("hKineticEnergy", "Kinetic Energy Fibre Mppc", 1000, 0, 70);
    eHisto->GetXaxis()->SetTitle("kinetic energy [MeV]");
    eHisto->GetYaxis()->SetTitle("counts");

    hAngleElUp = new TH2F("hAngleElUp", "Angle Electrons Upstream", 100, 0, M_PI, 200, 0, 2 * M_PI);
    hAngleElDown = new TH2F("hAngleElDown", "Angle Electrons Downstream", 100, 0, M_PI, 200, 0, 2 * M_PI);
    hAnglePosUp = new TH2F("hAnglePosUp", "Angle Positrons Upstream", 100, 0, M_PI, 200, 0, 2 * M_PI);
    hAnglePosDown = new TH2F("hAnglePosDown", "Angle Positrons Downstream", 100, 0, M_PI, 200, 0, 2 * M_PI);
    hAngleElUp->GetXaxis()->SetTitle("theta");
    hAngleElUp->GetYaxis()->SetTitle("phi");
    hAngleElDown->GetXaxis()->SetTitle("theta");
    hAngleElDown->GetYaxis()->SetTitle("phi");
    hAnglePosUp->GetXaxis()->SetTitle("theta");
    hAnglePosUp->GetYaxis()->SetTitle("phi");
    hAnglePosDown->GetXaxis()->SetTitle("theta");
    hAnglePosDown->GetYaxis()->SetTitle("phi");

    //collectionName.insert("fibreMppc");
}

Mu3eFibreMppcSD::~Mu3eFibreMppcSD() {
    //for(auto it : edepHistos) {
    //G4cout << it.second->GetName() << G4endl;
    //delete it.second;
    //}
}

void Mu3eFibreMppcSD::Initialize(G4HCofThisEvent*) {
}

G4bool Mu3eFibreMppcSD::ProcessHits(G4Step* aStep, G4TouchableHistory*) {
    G4double edep = aStep->GetTotalEnergyDeposit();
    if(edep <= 0) return false;
    auto prePoint = aStep->GetPreStepPoint();
    bool isSio2 = prePoint->GetPhysicalVolume()->GetName() == "fibreMppcSiO2";

    int depth = 0 + isSio2;

    int copyNo_ribbon = prePoint->GetTouchable()->GetCopyNumber(depth);
    int copyNo_side = prePoint->GetTouchable()->GetCopyNumber(depth + 1);
    uint32_t mppcId = mu3e::id::mppc(copyNo_ribbon, copyNo_side).value();

    //G4cout << "DEBUG " << prePoint->GetTouchable()->GetCopyNumber(2) << " " << prePoint->GetTouchable()->GetCopyNumber(1) << " " << prePoint->GetTouchable()->GetCopyNumber(0) << G4endl;
    G4double dose_c = 0;
    int pid = aStep->GetTrack()->GetParticleDefinition()->GetPDGEncoding();
    if(isSio2) {
        G4double cubicVolume = prePoint->GetPhysicalVolume()->GetLogicalVolume()->GetSolid()->GetCubicVolume();
        G4double density = prePoint->GetMaterial()->GetDensity();
        //G4cout << "DEBUG density " << density << " " << density/kg << " " << (density/kg * cubicVolume) << G4endl;
        dose_c = edep / joule / (density / kg * cubicVolume);
        dose_c *= prePoint->GetWeight();
        dose[mppcId] += dose_c;
        dose_p[pid][mppcId] += dose_c;
        //G4cout << "DEBUG mppcId" << mppcId << ", dose " << dose_c << " dose sum " << dose[mppcId] << "pid" << pid << "dose sum pid" << dose_p[pid][mppcId] << G4endl;

        if (0) G4cout << "Mppc   DEBUG density " << density
               << " rho/kg = " << density/kg
               << " cubicVol = " << cubicVolume
               << " w8 = " << (density/kg * cubicVolume)
               << " edep = " << edep << " edep/joule = " << edep/joule
               << " dose_c = " << dose_c
               << " w8' = " << prePoint->GetWeight()
               << G4endl;

        if(edepSiO2Histos[mppcId] == nullptr) {
            edepSiO2Histos[mppcId] = new TH1F(
                fmt::format("hFibreMppcEdepSiO2_{}", mppcId).c_str(),
                fmt::format("Edep Fibre Mppc SiO2 {}", mppcId).c_str(),
                1000, 0, 1
            );
            edepSiO2Histos[mppcId]->SetDirectory(0);
            edepSiO2Histos[mppcId]->GetXaxis()->SetTitle("edep [MeV]");
            edepSiO2Histos[mppcId]->GetYaxis()->SetTitle("counts");
        }
        edepSiO2Histos[mppcId]->Fill(edep);
        edepSiO2Histo->Fill(edep);

        if(edep_sio2_p[pid][mppcId] == nullptr) {
            edep_sio2_p[pid][mppcId] = new TH1F(
                fmt::format("mppcEdepSiO2_pid{}_mppc{}", pid, mppcId).c_str(),
                fmt::format("Edep Mppc SiO2 {} (pid {})", mppcId, pid).c_str(),
                1000, 0, 1
            );
            edep_sio2_p[pid][mppcId]->SetDirectory(0);
            edep_sio2_p[pid][mppcId]->GetXaxis()->SetTitle("edep [MeV]");
            edep_sio2_p[pid][mppcId]->GetYaxis()->SetTitle("counts");
        }
        edep_sio2_p[pid][mppcId]->Fill(edep);
    }
    else {
        if(aStep->IsFirstStepInVolume()) rate[mppcId] += 1;
        double df = 0;
        if(aStep->IsFirstStepInVolume()) { //  && abs(pid)==11
            df = damageFunction(aStep->GetTrack()->GetKineticEnergy());
        }
        rate_neq[mppcId] += df;
        if(edepHistos[mppcId] == nullptr) {
            edepHistos[mppcId] = new TH1F(
                fmt::format("hFibreMppcEdep_{}", mppcId).c_str(),
                fmt::format("Edep Fibre Mppc {}", mppcId).c_str(),
                1000, 0, 1
            );
            edepHistos[mppcId]->SetDirectory(0);
            edepHistos[mppcId]->GetXaxis()->SetTitle("edep [MeV]");
            edepHistos[mppcId]->GetYaxis()->SetTitle("counts");
        }
        edepHistos[mppcId]->Fill(edep);
        if(eHistos[mppcId] == nullptr) {
            eHistos[mppcId] = new TH1F(
                fmt::format("hKineticEnergy{}", mppcId).c_str(),
                fmt::format("Kinetic Energy Fibre Mppc {}", mppcId).c_str(),
                1000, 0, 70
            );
            eHistos[mppcId]->SetDirectory(0);
            eHistos[mppcId]->GetXaxis()->SetTitle("kinetic energy [MeV]");
            eHistos[mppcId]->GetYaxis()->SetTitle("counts");
        }
        edepHisto->Fill(edep);
        if(aStep->IsFirstStepInVolume()) {
            eHistos[mppcId]->Fill(aStep->GetTrack()->GetKineticEnergy());
            eHisto->Fill(aStep->GetTrack()->GetKineticEnergy());
        }

        if(aStep->IsFirstStepInVolume()) rate_p[pid][mppcId] += 1;
        rate_neq_p[pid][mppcId] += df;
        if(e_p[pid][mppcId] == nullptr) {
            e_p[pid][mppcId] = new TH1F(
                fmt::format("hKineticEnergy_pid{}_mppc{}", pid, mppcId).c_str(),
                fmt::format("Kinetic Energy Fibre Mppc {} (pid {})", mppcId, pid).c_str(),
                1000, 0, 70
            );
            e_p[pid][mppcId]->SetDirectory(0);
            e_p[pid][mppcId]->GetXaxis()->SetTitle("kinetic energy [MeV]");
            e_p[pid][mppcId]->GetYaxis()->SetTitle("counts");
        }
        e_p[pid][mppcId]->Fill(aStep->GetTrack()->GetKineticEnergy());

        if(edep_p[pid][mppcId] == nullptr) {
            edep_p[pid][mppcId] = new TH1F(
                fmt::format("mppcEdep_pid{}_mppc{}", pid, mppcId).c_str(),
                fmt::format("Edep Mppc {} (pid {})", mppcId, pid).c_str(),
                1000, 0, 1
            );
            edep_p[pid][mppcId]->SetDirectory(0);
            edep_p[pid][mppcId]->GetXaxis()->SetTitle("edep [MeV]");
            edep_p[pid][mppcId]->GetYaxis()->SetTitle("counts");
        }
        edep_p[pid][mppcId]->Fill(edep);
    }

    double theta = aStep->GetTrack()->GetMomentum().getTheta();
    double phi = 0;
    if(mppcId & 0x1)
        phi = fmod((aStep->GetTrack()->GetMomentum().getPhi() - (-M_PI / 2 - (mppcId >> 1) * M_PI / 6) + 4 * M_PI), 2 * M_PI);
    else
        phi = fmod((aStep->GetTrack()->GetMomentum().getPhi() - (+M_PI / 2 + (mppcId >> 1) * M_PI / 6) + 4 * M_PI), 2 * M_PI);
    if((mppcId & 0x1)) {
        if(pid == -11) hAnglePosUp->Fill(theta, phi);
        if(pid == 11) hAngleElUp->Fill(theta, phi);
    }
    else {
        if(pid == -11) hAnglePosDown->Fill(theta, phi);
        if(pid == 11) hAngleElDown->Fill(theta, phi);
    }

    //Mu3eEvent::GetInstance()->FillFibreDistX(x)
    return true;
}

void Mu3eFibreMppcSD::writeStat() {
    //for(auto it : dose) {
    //  G4cout << "Dose in Fibre Mppc " << it.first << " is " << it.second/evendtime << " Gy/s" << G4endl;
    //}

    TH1F* hFibreMppcDose = new TH1F("hFibreMppcDose", "Fibre Mppc Dose", 32, 0, 32);
    hFibreMppcDose->GetXaxis()->SetTitle("fibre mppc");
    hFibreMppcDose->GetYaxis()->SetTitle("dose/time [Gy/s]");
    double evendtime = (Mu3eEvent::GetInstance()->startTime + Mu3eEvent::GetInstance()->frameLength) / s;
    for(int i = 0; i < 32; ++i) {
        hFibreMppcDose->SetBinContent(i + 1, dose[i] / evendtime);
    }
    hFibreMppcDose->Write();

    TH1F* hFibreMppcRate = new TH1F("hFibreMppcRate", "Fibre Mppc Rate", 32, 0, 32);
    hFibreMppcRate->GetXaxis()->SetTitle("fibre mppc");
    hFibreMppcRate->GetYaxis()->SetTitle("rate [Hz]");
    //double evendtime = Mu3eEvent::GetInstance()->startTime + Mu3eEvent::GetInstance()->frameLength;
    for(int i = 0; i < 32; ++i) {
        hFibreMppcRate->SetBinContent(i + 1, rate[i] / evendtime);
        hFibreMppcRate->SetBinError(i + 1, sqrt(rate[i]) / evendtime);
    }
    hFibreMppcRate->Write();

    TH1F* hFibreMppcRateNeq = new TH1F("hFibreMppcRateNeq", "Fibre Mppc Rate (neq)", 32, 0, 32);
    hFibreMppcRateNeq->GetXaxis()->SetTitle("fibre mppc");
    hFibreMppcRateNeq->GetYaxis()->SetTitle("rate (neq) [Hz/cm^2]");
    //double evendtime = Mu3eEvent::GetInstance()->startTime + Mu3eEvent::GetInstance()->frameLength;
    for(int i = 0; i < 32; ++i) {
        hFibreMppcRateNeq->SetBinContent(i + 1, rate_neq[i] / evendtime / (3.2 * 0.16));
        //hFibreMppcRateNeq->SetBinError(i+1,   sqrt(rate[i])/evendtime);
    }
    hFibreMppcRateNeq->Write();
    //G4cout << "ok" << G4endl;

    edepHisto->Write();
    edepSiO2Histo->Write();
    eHisto->Write();
    hAngleElUp->Write();
    hAngleElDown->Write();
    hAnglePosUp->Write();
    hAnglePosDown->Write();

    gDirectory->mkdir("FibreMppcSensors");
    gDirectory->cd("FibreMppcSensors");

    for(auto itl : edepHistos) {
        itl.second->Write();
    }
    for(auto itl : edepSiO2Histos) {
        itl.second->Write();
    }
    for(auto itl : eHistos) {
        itl.second->Write();
    }

    gDirectory->cd("..");
    gDirectory->mkdir("FibreMppcPid");
    gDirectory->cd("FibreMppcPid");

    // pid
    std::map<int, TH1F*> h_dose_p;
    std::map<int, TH1F*> h_rate_p;
    std::map<int, TH1F*> h_rate_neq_p;
    for(auto it : edep_sio2_p) {
        int pid = it.first;

        h_dose_p[pid] = new TH1F(
            fmt::format("hFibreMppcDose_{}", pid).c_str(),
            fmt::format("Mppc Dose (pid {})", pid).c_str(),
            32, 0, 32
        );
        h_dose_p[pid]->GetXaxis()->SetTitle("fibre mppc");
        h_dose_p[pid]->GetYaxis()->SetTitle("dose/time [Gy/s]");
        //double evendtime = Mu3eEvent::GetInstance()->startTime + Mu3eEvent::GetInstance()->frameLength;
        for(int i = 0; i < 32; ++i) {
            h_dose_p[pid]->SetBinContent(i + 1, dose_p[pid][i] / evendtime);
        }
        h_dose_p[pid]->Write();
    }

    for(auto it : edep_p) {
        int pid = it.first;
        //h_rate_p[pid]  = new TH1F("hFibreMppcRate", "Fibre Mppc Rate", 64, 0, 64);
        h_rate_p[pid] = new TH1F(
            fmt::format("hFibreMppcRate_{}", pid).c_str(),
            fmt::format("Mppc Rate (pid {})", pid).c_str(),
            32, 0, 32
        );
        h_rate_p[pid]->GetXaxis()->SetTitle("fibre mppc");
        h_rate_p[pid]->GetYaxis()->SetTitle("rate [Hz]");
        //double evendtime = Mu3eEvent::GetInstance()->startTime + Mu3eEvent::GetInstance()->frameLength;
        for(int i = 0; i < 32; ++i) {
            h_rate_p[pid]->SetBinContent(i + 1, rate_p[pid][i] / evendtime);
        }
        h_rate_p[pid]->Write();

        h_rate_neq_p[pid] = new TH1F(
            fmt::format("hFibreMppcRateNeq_{}", pid).c_str(),
            fmt::format("Mppc Rate neq (pid {})", pid).c_str(),
            32, 0, 32
        );
        h_rate_neq_p[pid]->GetXaxis()->SetTitle("fibre mppc");
        h_rate_neq_p[pid]->GetYaxis()->SetTitle("rate (neq) [Hz/cm^2]");
        //double evendtime = Mu3eEvent::GetInstance()->startTime + Mu3eEvent::GetInstance()->frameLength;
        for(int i = 0; i < 32; ++i) {
            h_rate_neq_p[pid]->SetBinContent(i + 1, rate_neq_p[pid][i] / evendtime / (3.2 * 0.16));
        }
        h_rate_neq_p[pid]->Write();
    }
    for(auto itl : edep_p) {
        for(auto itt : itl.second) {
            itt.second->Write();
        }
    }
    for(auto itl : e_p) {
        for(auto itt : itl.second) {
            itt.second->Write();
        }
    }
    for(auto itl : edep_sio2_p) {
        for(auto itt : itl.second) {
            itt.second->Write();
        }
    }

    gDirectory->cd("..");
}

void Mu3eFibreMppcSD::EndOfEvent(G4HCofThisEvent*) {}

#include <mu3e/util/phys.hpp>

// http://ieeexplore.ieee.org/document/273529
double Mu3eFibreMppcSD::damageFunction(double energy) {
    return mu3e::phys::damageFunction_e_Si(energy);
}
