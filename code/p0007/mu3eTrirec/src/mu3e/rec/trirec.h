//

#pragma once

#include "Root.h"

#include "cosmic/Frame.h"
#include "gpu/Frame.h"
#include "twolayer/Frame.h"

#include "Conf.h"
#include "Stat.h"

#include "util/sim.h"

#include <mu3e/util/midas_file.h>
#include <mu3e/util/RootFile.h>
#include <mu3e/util/verbose.hpp>

#include "Mu3eConditions.hh"

#include <boost/algorithm/string.hpp>

#if defined(__x86_64__)
#include <x86intrin.h>
#endif

#if defined(MU3E_DISPLAY)
#include "mu3e/display/EventDisplay.h"
#else
class EventDisplay {
public:
    void init() {};
    void RunOnFrame(const Frame& frame) {}
};
#endif

#include <iostream>

namespace mu3e::rec {

struct trirec_t {
    Root root;
    EventDisplay display;

    std::ofstream hitsFile;
    std::ofstream truthFile;

    trirec_t() {
        if(!Conf::inst.gpu.hitsFile.empty()) {
            hitsFile.open(Conf::inst.gpu.hitsFile, std::ios::out | std::ios::binary);
        }
        if(!Conf::inst.gpu.truthFile.empty()) {
            truthFile.open(Conf::inst.gpu.truthFile, std::ios::out | std::ios::binary);
        }

        if(mu3e::rec::conf.display.mode > 0) {
            display.init();
        }
    }

    virtual
    ~trirec_t() {
        if(hitsFile.is_open()) {
            hitsFile.flush();
            hitsFile.close();
            if(!hitsFile.good()) {
                fmt::print("E [{}] hitsFile is not in good state\n", __FUNCTION__);
            }
        }

        if(truthFile.is_open()) {
            truthFile.flush();
            truthFile.close();
            if(!truthFile.good()) {
                fmt::print("E [{}] truthFile is not in good state\n", __FUNCTION__);
            }
        }
    }

    float trec = 0; // [ms], reconstruction time
    float twrite = 0; // [ms], output write time

    using callback_t = std::function<void(const Frame& frame)>;

    void run(
        const mu3e::rec::util::header_t& header,
        const SiDet::ptr_t& siDet,
        const FbDet::ptr_t& fbDet,
        const TlDet::ptr_t& tlDet,
        MCTrack::map_t&& tracks,
        const callback_t& callback
    ) {
        std::unique_ptr<Frame> frame;
        if(mu3e::rec::conf.gpu.mode > 0) {
            frame = std::make_unique<mu3e::rec::gpu::Frame>(siDet, fbDet, tlDet, std::move(tracks));
        }
        else if(mu3e::rec::conf.cosmic.mode > 0) {
            frame = std::make_unique<mu3e::cosmic::Frame>(siDet, fbDet, tlDet, std::move(tracks));
        }
        else if(mu3e::rec::conf.twolayer.mode > 0) {
            frame = std::make_unique<mu3e::twolayer::Frame>(siDet, fbDet, tlDet, std::move(tracks));
        }
        else {
            frame = std::make_unique<Frame>(siDet, fbDet, tlDet, std::move(tracks));
        }

        frame->runId = header.run;
        frame->eventId = header.event;
        frame->weight = header.weight;

        auto trec_start = std::chrono::steady_clock::now();

        frame->rec();

        auto trec_end = std::chrono::steady_clock::now();
        float trec = std::chrono::duration<float, std::milli>(trec_end - trec_start).count();
        this->trec += trec;

        Stat::inst.htrec.fill(trec);
        Stat::inst.htrec_log10.fill(std::log10(trec));
        if(mu3e::rec::conf.verbose >= 1 && trec > 100) { // ms
            MU3E_WARN("trec = %.3f seconds\n", trec / 1000);
        }

        auto twrite_start = std::chrono::steady_clock::now();
        root.write(*frame);
        auto twrite_end = std::chrono::steady_clock::now();
        float twrite = std::chrono::duration<float, std::milli>(twrite_end - twrite_start).count();
        this->twrite += twrite;

        if(hitsFile.is_open()) {
            siDet->write_hits(hitsFile);
        }
        if(truthFile.is_open()) {
            frame->write_segs(truthFile);
        }

        if(mu3e::rec::conf.display.mode > 0) {
            display.RunOnFrame(*frame);
        }

        callback(*frame);
    }

    void run(
        TDirectory* file, int nEntry, int64_t beginEntry,
        const callback_t& callback = [] (const Frame&) {}
    ) {
        // -- Force reading/calculating (initial) payloads
        Mu3eConditions *pDC = Mu3eConditions::instance();
        pDC->setRunNumber(1);
  
        SiDet::Sensor::map_t sensors;
        SiDet::readSensors(sensors, file->Get<TTree>(mu3e::rec::conf.alignment.sensors.c_str()));
        SiDet::init(sensors);
        auto si = std::make_shared<SiDet>(sensors);

        FbDet::Fibre::map_t fibres;
        FbDet::readFibres(fibres, file->Get<TTree>(mu3e::rec::conf.alignment.fibres.c_str()));
        FbDet::Mppc::map_t mppcs;
        FbDet::readMppcs(mppcs, file->Get<TTree>(mu3e::rec::conf.alignment.mppcs.c_str()));
        FbDet::init(mppcs);
        auto fb = std::make_shared<FbDet>(fibres, mppcs);

        if(fibres.empty()) mu3e::rec::conf.fbDet = 0;
        else mu3e::rec::conf.fbDet = 1;

        TlDet::Tile::map_t tiles;
        TlDet::readTiles(tiles, file->Get<TTree>(mu3e::rec::conf.alignment.tiles.c_str()));
        auto tl = std::make_shared<TlDet>(tiles);

        auto tree = file->Get<TTree>("mu3e");
        if(tree == nullptr) {
            mu3e::log::fatal("[main] 'mu3e' tree not found\n");
            exit(EXIT_FAILURE);
        }
        auto tree_mc = file->Get<TTree>("mu3e_mchits");

        mu3e::rec::util::header_t header;
        tree->SetBranchAddress("Header", &header);
        tree->SetBranchAddress("Weight", &header.weight);

        mu3e::rec::util::sihit_t sihit(tree);
        mu3e::rec::util::mppchit_t mppchit(tree);
        mu3e::rec::util::fbhit_t fbhit(tree);
        mu3e::rec::util::tlhit_t tlhit(tree);
        mu3e::rec::util::mchit_t mchit(tree_mc);
        mu3e::rec::util::traj_t traj(tree);

        int64_t endEntry = tree->GetEntries();
        if(nEntry >= 0 && beginEntry + nEntry < endEntry) endEntry = beginEntry + nEntry;
        for(int64_t entry = beginEntry; entry < endEntry; entry++) {
            mu3e::rec::conf.runtime.frame = entry;
            if(mu3e::rec::conf.verbose >= 3) printf("%s", "");
         
            tree->GetEntry(entry);

            pDC->setRunNumber(header.run);

            si->reset();
            for(size_t i = 0, n = sihit.id.size(); i < n; i++) {
                auto hit = si->add(sihit.id[i], sihit.timestamp[i]);
                if(!hit) continue;
                if(i < sihit.mc_i.size()) mchit.load(&hit->mcs, sihit.mc_i[i], sihit.mc_n[i]);
                hit->tid = hit->mcs.front().tid;
                hit->hid = hit->mcs.front().hid;
            }
            si->generateNoise(mu3e::rec::conf.expert.noise * mu3e::units::Hz, mu3e::rec::conf.frameLength);
            si->init();

            // use local (frame) time for fb and tl hits
            for(auto& time : mppchit.time.data) {
                time -= double(mu3e::rec::conf.frameLength) * header.event;
            }
            for(auto& time : tlhit.time.data) {
                time -= double(mu3e::rec::conf.frameLength) * header.event;
            }

            fb->reset();
            switch(mu3e::rec::conf.rec_fb) {
            case 2:
                for(size_t i = 0, n = fbhit.id.size(); i < n; i++) {
                    //auto hit =
                    fb->add(fbhit.id[i], fbhit.time[i]);
                    //if(i < fbhit.mc_i.size()) mchit.load(&hit->mcs, fbhit.mc_i[i], fbhit.mc_n[i]);
                }
                break;
            default:
                for(size_t i = 0, n = mppchit.id.size(); i < n; i++) {
                    mu3e::id::mppc mppcId(mppchit.id[i]);
                    uint32_t id = mu3e::id::mppc::hit(mppcId, mppchit.col[i]).value();
                    auto hit = fb->add(id, mppchit.timestamp[i]);
                    if(i < mppchit.time.size()) hit->time = mppchit.time[i];
                    if(i < mppchit.mc_i.size()) mchit.load(&hit->mcs, mppchit.mc_i[i], mppchit.mc_n[i]);
                    for(auto& mc : hit->mcs) mc.time -= double(mu3e::rec::conf.frameLength) * header.event;
                }
            }
            fb->init();

            tl->reset();
            for(size_t i = 0, n = tlhit.id.size(); i < n; i++) {
                auto hit = tl->add(tlhit.id[i], tlhit.timestamp[i], tlhit.edep[i]);
                if(i < tlhit.primary.size()) hit->primary = tlhit.primary[i];
                if(i < tlhit.time.size()) hit->time = tlhit.time[i];
                if(i < tlhit.mc_i.size()) mchit.load(&hit->mcs, tlhit.mc_i[i], tlhit.mc_n[i]);
                for(auto& mc : hit->mcs) mc.time -= double(mu3e::rec::conf.frameLength) * header.event;
            }

            MCTrack::map_t tracks;
            for(size_t i = 0, n = traj.pid.size(); i < n; i++) {
                int32_t pid = traj.pid[i];
                if(std::abs(pid) != 11 && std::abs(pid) != 13) continue; // e^+/-, mu^+/-

                int32_t id = traj.id[i];
                traj.fillMCTrack(tracks[id], i);
            }

            // TODO: init si, fb and tl once; clean/add hits each frame
            run(header, si, fb, tl, std::move(tracks), callback);

            // stop after ^C
            if(mu3e::rec::conf.runtime.sigint_cnt > 0) break;
        }

        MU3E_INFO("trec = %.1f seconds / %ld frames\n", trec/1000, endEntry - beginEntry);
        MU3E_INFO("twrite = %.1f seconds\n", twrite/1000);

        file->Close();
    }

    void run_mid(TDirectory* file, int nEntry, int64_t beginEntry) {
        SiDet::Sensor::map_t sensors;
        SiDet::readSensors(sensors, file->Get<TTree>(mu3e::rec::conf.alignment.sensors.c_str()));
        SiDet::init(sensors);
        auto si = std::make_shared<SiDet>(sensors);

        FbDet::Fibre::map_t fibres;
        FbDet::readFibres(fibres, file->Get<TTree>(mu3e::rec::conf.alignment.fibres.c_str()));
        FbDet::Mppc::map_t mppcs;
        FbDet::readMppcs(mppcs, file->Get<TTree>(mu3e::rec::conf.alignment.mppcs.c_str()));
        FbDet::init(mppcs);
        auto fb = std::make_shared<FbDet>(fibres, mppcs);

        if(fibres.empty()) mu3e::rec::conf.fbDet = 0;
        else mu3e::rec::conf.fbDet = 1;

        TlDet::Tile::map_t tiles;
        TlDet::readTiles(tiles, file->Get<TTree>(mu3e::rec::conf.alignment.tiles.c_str()));
        auto tl = std::make_shared<TlDet>(tiles);

        midas_file_t mf(mu3e::rec::conf.runtime.input_mid.c_str());

        int64_t endEntry = INT64_MAX;
        if(nEntry >= 0 && beginEntry + nEntry < endEntry) endEntry = beginEntry + nEntry;
        for(int64_t entry = 0; entry < endEntry; entry++) {
            if(mf.next_event() < 0) break;
            if(entry < beginEntry) continue;

            mu3e::rec::conf.runtime.frame = mf.event.serial_number;
            if(mu3e::rec::conf.verbose >= 3) printf("%s", "");

            si->reset();
            while(1) {
                if(mf.next_bank() < 0) break;
                if(strcmp(mf.bank.name, "PHIT") == 0) for(uint32_t i = 0; i < mf.bank.size; i += 8) {
                    uint64_t hit;
                    memcpy(reinterpret_cast<char*>(&hit), mf.bank.data.get() + i, 8);
                    //int sensor = (hit >> 48) & 0xFFFF;
                    //int col = (hit >> 40) & 0xFF;
                    //int row = (hit >> 32) & 0xFF;
                    si->add(hit >> 32, hit & 0x3FFFFFF);
                }
            }
            si->init();

            MCTrack::map_t tracks;

            int frameId = mf.event.serial_number;
            run({ frameId }, si, fb, tl, std::move(tracks), {});
        }
    }
};

/**
 * Handle ^C.
 *
 * Stop reconstruction after current event.
 */
static
void set_SIGINT_handler() {
    std::signal(SIGINT, [] (int) {
        mu3e::rec::conf.runtime.sigint_cnt += 1;

        // FIXME: handle ^C inside printf
        write(STDERR_FILENO, "SIGINT\n", 7);

        if(mu3e::rec::conf.runtime.sigint_cnt > 0) {
            // set default signal handler
            std::signal(SIGINT, SIG_DFL);
            // (^C will terminate execution)
            //std::raise(SIGINT);
        }
    });
}

static
void parse_argc(int argc, const char* argv[]) {
    namespace po = boost::program_options;

    mu3e::util::verbose_t verbose;

    auto& input = mu3e::rec::conf.runtime.input;
    auto& output = mu3e::rec::conf.runtime.output;
    auto& confFiles = mu3e::rec::conf.runtime.confFiles;
    auto& n = mu3e::rec::conf.runtime.n;
    auto& skip = mu3e::rec::conf.runtime.skip;

    po::options_description opts("Global Options");
    opts.add_options()
        (",n", po::value(&n), "number of frames to process")
        ("skip,s", po::value(&skip), "number of frames to skip")
        ("conf", po::value(&confFiles), ".conf or .json file")
        ("verbose,v", po::value(&verbose)->zero_tokens(), "increase verbosity")
        ("version", "version")
        ("help", "help");

    po::options_description opts_cosmic("Cosmic Options");
    opts_cosmic.add_options()
        ("cosmic", po::value(&mu3e::rec::conf.cosmic.mode)->implicit_value(1), "cosmic reconstruction mode");
    opts.add(opts_cosmic);

    po::options_description opts_gpu("GPU Options");
    opts_gpu.add_options()
        ("gpu", po::value(&mu3e::rec::conf.gpu.mode)->implicit_value(1), "4-hit segments selected as on GPU")
        ("gpu-hits-file", po::value(&mu3e::rec::conf.gpu.hitsFile), "Write hits in central station into binary file to be used as input for GPU selection")
        ("gpu-truth-file", po::value(&mu3e::rec::conf.gpu.truthFile), "Write true combinations into binary file to compare with GPU selection");
    opts.add(opts_gpu);

    po::options_description opts_twolayer("Two-layer Options");
    opts_twolayer.add_options()
        ("twolayer", po::value(&mu3e::rec::conf.twolayer.mode)->implicit_value(1), "two layer detector mode");
    opts.add(opts_twolayer);

    po::options_description opts_expert("Expert Options");
    opts_expert.add_options()
        ("noise", po::value(&mu3e::rec::conf.expert.noise), "pixel noise rate [Hz]")
        ("no-denormals", "disable denormals");
    opts.add(opts_expert);

    // define hidden args
    po::options_description opts_p;
    opts_p.add_options()
        ("input", po::value(&input))
        ("input-mid", po::value(&mu3e::rec::conf.runtime.input_mid))
        ("output", po::value(&output))
        ("display", po::value(&mu3e::rec::conf.display.mode)->implicit_value(1))
        ("test", po::value(&mu3e::rec::conf.test));

    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv)
        .options(po::options_description().add(opts).add(opts_p))
        .positional(po::positional_options_description()
            .add("input", 1)
            .add("output", 1)
        )
        .run(), vm
    );
    po::notify(vm);

    mu3e::rec::conf.verbose = verbose.n;

    if(vm.count("version")) {
        fmt::print("mu3e/rec {}\n", BOOST_PP_STRINGIZE(MU3E_VERSION));
        exit(EXIT_SUCCESS);
    }
    if(vm.count("help") || input.empty()) {
        printf("Usage: trirec [OPTION]... <INPUT> [OUTPUT]\n\n");
        opts.print(std::cout);
        exit(EXIT_SUCCESS);
    }

    if(vm.count("no-denormals")) {
#if defined(_MM_SET_DENORMALS_ZERO_MODE)
        mu3e::log::warn("[main] denormals zero mode\n");
        _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
#else
        mu3e::log::fatal("[main] undefined _MM_SET_DENORMALS_ZERO_MODE\n");
        exit(EXIT_FAILURE);
#endif
    }

    if(skip < 0) skip = 0;
    if(n < 0) n = -1;

    if(mu3e::rec::conf.gpu.mode == 0) {
        if(vm.count("gpu-hits-file")) {
            mu3e::log::fatal("[main] --gpu-hits-file requires --gpu\n");
            exit(EXIT_FAILURE);
        }
        if(vm.count("gpu-truth-file")) {
            mu3e::log::fatal("[main] --gpu-truth-file requires --gpu\n");
            exit(EXIT_FAILURE);
        }
    }

    // set configs for different modes
    if(confFiles.empty()) {
        if(mu3e::rec::conf.gpu.mode > 0) confFiles.emplace_back("trirec_gpu.conf");
        if(mu3e::rec::conf.twolayer.mode > 0) confFiles.push_back("trirec_twolayer.conf");
    }

    // default config file
    if(confFiles.empty()) confFiles.emplace_back("trirec.conf");

    if(!output.empty() && boost::filesystem::equivalent(input, output)) {
        mu3e::log::fatal("[main] input == output\n");
        exit(EXIT_FAILURE);
    }
}

} // namespace mu3e::rec
