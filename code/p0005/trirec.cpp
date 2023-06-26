//

#include "Root.h"

#include "cosmic/Frame.h"
#include "gpu/Frame.h"
#include "twolayer/Frame.h"

#include "Conf.h"
#include "Stat.h"

#include <mu3e/util/RootFile.h>

#include "Mu3eConditions.hh"
#include "cdbAbs.hh"
#include "cdbJSON.hh"
#include "cdbRest.hh"
#include "calPixelAlignment.hh"

#if defined(MU3E_TRIREC_DISPLAY)
#include "display/EventDisplay.h"
#else
class EventDisplay {
public:
    void run(const Frame&) {}
    void init() {}
};
#endif

#include <boost/algorithm/string.hpp>

#include <x86intrin.h>

#include <csignal>
static volatile std::sig_atomic_t sigint_cnt = 0;

static int64_t printf_frame = -1;

/**
 * Print frame id before any other output.
 */
int printf(const char* format, ...) {
    static int64_t frame = -1;

    if(frame != printf_frame) {
        frame = printf_frame;
        mu3e::log::sgr({1,32});
        fprintf(stdout, "## FRAME %ld ##", frame);
        mu3e::log::sgr();
        fprintf(stdout, "\n");
    }

    va_list args;
    va_start(args, format);
    int n = vfprintf(stdout, format, args);
    va_end(args);

    return n;
}

#include "util/sim.h"
using namespace mu3e::rec::util;

struct trirec_t {
    EventDisplay display;
    Root root;

    std::ofstream hitsFile;
    std::ofstream truthFile;

    trirec_t() {
        if(!Conf::inst.gpu.hitsFile.empty()) {
            hitsFile.open(Conf::inst.gpu.hitsFile, std::ios::out | std::ios::binary);
        }
        if(!Conf::inst.gpu.truthFile.empty()) {
            truthFile.open(Conf::inst.gpu.truthFile, std::ios::out | std::ios::binary);
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

    void run(
        const header_t& header,
        const SiDet::ptr_t& siDet,
        const FbDet::ptr_t& fbDet,
        const TlDet::ptr_t& tlDet,
        MCTrack::map_t&& tracks
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
            mu3e::log::warn("[%s] trec = %.3f seconds\n", __FUNCTION__, 1e-3 * trec);
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

        display.run(*frame);
    }

    void run(const char* input, int nEntry, int64_t beginEntry) {
        run(new TFile(input, "READ"), nEntry, beginEntry);
    }

    void run(TDirectory* file, int nEntry, int64_t beginEntry) {
        std::cout << "HALLO in run(" << file << ", " << nEntry << ", " << beginEntry << ")" << std::endl;
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

        header_t header;
        tree->SetBranchAddress("Header", &header);
        tree->SetBranchAddress("Weight", &header.weight);

        sihit_t sihit(tree);
        mppchit_t mppchit(tree);
        fbhit_t fbhit(tree);
        tlhit_t tlhit(tree);
        mchit_t mchit(tree_mc);
        traj_t traj(tree);

        int64_t endEntry = tree->GetEntries();
        if(nEntry >= 0 && beginEntry + nEntry < endEntry) endEntry = beginEntry + nEntry;
        for(int64_t entry = beginEntry; entry < endEntry; entry++) {
            printf_frame = entry;
            if(mu3e::rec::conf.verbose >= 3) printf("%s", "");

            tree->GetEntry(entry);

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
                time -= mu3e::rec::conf.frameLength * header.event;
            }
            for(auto& time : tlhit.time.data) {
                time -= mu3e::rec::conf.frameLength * header.event;
            }

            fb->reset();
            switch(mu3e::rec::conf.rec_fb) {
            case 2:
                for(size_t i = 0, n = fbhit.id.size(); i < n; i++) {
//                    auto hit =
                    fb->add(fbhit.id[i], fbhit.time[i]);
//                    if(i < fbhit.mc_i.size()) mchit.load(&hit->mcs, fbhit.mc_i[i], fbhit.mc_n[i]);
                }
                break;
            default:
                for(size_t i = 0, n = mppchit.id.size(); i < n; i++) {
                    mu3e::id::mppc mppcId(mppchit.id[i]);
                    uint32_t id = mu3e::id::mppc::hit(mppcId, mppchit.col[i]).value();
                    auto hit = fb->add(id, mppchit.timestamp[i]);
                    if(i < mppchit.time.size()) hit->time = mppchit.time[i];
                    if(i < mppchit.mc_i.size()) mchit.load(&hit->mcs, mppchit.mc_i[i], mppchit.mc_n[i]);
                    for(auto& mc : hit->mcs) mc.time -= mu3e::rec::conf.frameLength * header.event;
                }
            }
            fb->init();

            tl->reset();
            for(size_t i = 0, n = tlhit.id.size(); i < n; i++) {
                auto hit = tl->add(tlhit.id[i], tlhit.timestamp[i], tlhit.edep[i]);
                if(i < tlhit.primary.size()) hit->primary = tlhit.primary[i];
                if(i < tlhit.time.size()) hit->time = tlhit.time[i];
                if(i < tlhit.mc_i.size()) mchit.load(&hit->mcs, tlhit.mc_i[i], tlhit.mc_n[i]);
                for(auto& mc : hit->mcs) mc.time -= mu3e::rec::conf.frameLength * header.event;
            }

            MCTrack::map_t tracks;
            for(size_t i = 0, n = traj.pid.size(); i < n; i++) {
                int32_t pid = traj.pid[i];
                if(std::abs(pid) != 11 && std::abs(pid) != 13) continue; // e^+/-, mu^+/-

                int32_t id = traj.id[i];
                traj.fillMCTrack(tracks[id], i);
            }

            // TODO: init si, fb and tl once; clean/add hits each frame
            run(header, si, fb, tl, std::move(tracks));

            // stop after ^C
            if(sigint_cnt > 0) break;
        }

        mu3e::log::info("[%s] trec = %.1f seconds / %ld frames\n", __func__, 1e-3 * trec, endEntry - beginEntry);
        mu3e::log::info("[%s] twrite = %.1f seconds\n", __func__, 1e-3 * twrite);

        file->Close();
    }
};

#include <mu3e/util/verbose.hpp>

int main(int argc, const char* argv[]) {
    namespace po = boost::program_options;

    /**
     * Handle ^C.
     *
     * Stop reconstruction after current event.
     */
    std::signal(SIGINT, [] (int) {
        sigint_cnt += 1;

        // FIXME: handle ^C inside printf
        write(STDERR_FILENO, "SIGINT\n", 7);

        if(sigint_cnt > 0) {
            // set default signal handler
            std::signal(SIGINT, SIG_DFL);
            // (^C will terminate execution)
//            std::raise(SIGINT);
        }
    });

    /**
     * Flush stdout on abort.
     */
/*    signal(SIGABRT, [] (int) {
        fflush(stdout);
        signal(SIGABRT, SIG_DFL);
        raise(SIGABRT);
    });*/

    mu3e::util::verbose_t verbose;
    std::string input, output;
    std::vector<std::string> confFiles;
    int n = -1, skip = 0;

    std::string dbconn("fixme");
    int run(779);
    po::options_description opts;
    opts.add_options()
        ("input", po::value(&input), "input root file (mu3eSim)")
        ("output", po::value(&output), "output root file")
        ("dbconn", po::value(&dbconn), "DB connection method (json,rest)")
        ("run", po::value(&run), "run number")
        (",n", po::value(&n), "number of frames to process")
        ("skip,s", po::value(&skip), "number of frames to skip")
        ("conf", po::value(&confFiles), ".conf or .json file")
        ("verbose,v", po::value(&verbose)->zero_tokens(), "increase verbosity")
        ("display,d", po::value(&mu3e::rec::conf.display.mode)->implicit_value(1), "run event display")
        ("version", "version")
        ("help", "help");

    po::options_description opts_cosmic("Cosmic options");
    opts_cosmic.add_options()
        ("cosmic", po::value(&mu3e::rec::conf.cosmic.mode)->implicit_value(1), "cosmic reconstruction mode");
    opts.add(opts_cosmic);

    po::options_description opts_gpu("GPU options");
    opts_gpu.add_options()
        ("gpu", po::value(&mu3e::rec::conf.gpu.mode)->implicit_value(1), "4-hit segments selected as on GPU")
        ("gpu-hits-file", po::value(&mu3e::rec::conf.gpu.hitsFile), "Write hits in central station into binary file to be used as input for GPU selection")
        ("gpu-truth-file", po::value(&mu3e::rec::conf.gpu.truthFile), "Write true combinations into binary file to compare with GPU selection");
    opts.add(opts_gpu);

    po::options_description opts_twolayer("Two layer detector options");
    opts_twolayer.add_options()
        ("twolayer", po::value(&mu3e::rec::conf.twolayer.mode)->implicit_value(1), "two layer detector mode");
    opts.add(opts_twolayer);

    po::options_description opts_expert("Expert options");
    opts_expert.add_options()
        ("noise", po::value(&mu3e::rec::conf.expert.noise), "pixel noise rate [Hz]")
        ("no-denormals", "disable denormals");
    opts.add(opts_expert);

#if defined(MU3E_TRIREC_DISPLAY)
    po::options_description opts_display("Display options");
    opts_display.add_options()
        ("display-scale", po::value(&mu3e::rec::conf.display.scale));
    opts.add(opts_display);
#endif

    po::positional_options_description opts_p;
    opts_p.add("input", 1);
    opts_p.add("output", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(opts).positional(opts_p).run(), vm);
    po::notify(vm);

    mu3e::rec::conf.verbose = verbose.n;

 
    int dbverbose(10); 
    cdbAbs *pDB(0);
    std::string gt = "dt23prompt";
    if (dbconn == "rest") {
      pDB = new cdbRest(gt, "https://eu-central-1.aws.data.mongodb-api.com/app/data-pauzo/endpoint/data/v1/action/",
                        dbverbose);
    } else if (dbconn == "json") {
      pDB = new cdbJSON(gt, "/psi/home/langenegger/mu3e/mu3eanca/db0/cdb1/json", dbverbose);
    } else {
      std::cout << "NO DB connection defined. Exit!" << std::endl;
      exit(1);
    }
    
    Mu3eConditions *pDC = Mu3eConditions::instance(gt, pDB);

    calAbs *cal = pDC->createClass("pixelalignment_");
    std::cout << "setting run number " << run << std::endl;
    pDC->setRunNumber(run);

    
    if(vm.count("version")) {
        fmt::print("mu3e/rec {}\n", BOOST_PP_STRINGIZE(MU3E_VERSION));
        return 0;
    }
    if(vm.count("help") || input.empty()) {
        printf("Usage: trirec [options] <input> <output>\n");
        opts.print(std::cout);
        return 0;
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

    if(mu3e::rec::conf.display.mode > 0) {
#ifndef MU3E_TRIREC_DISPLAY
        mu3e::log::fatal("Build trirec with -DMU3E_TRIREC_DISPLAY\n");
        exit(EXIT_FAILURE);
#endif
        output = "";
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

    // debug info
    if(mu3e::rec::conf.verbose >= 3) {
        mu3e::log::debug("[main] sizeof(HelixBase) = %zu\n", sizeof(HelixBase));
    }

    // set configs for different modes
    if(confFiles.empty()) {
        if(mu3e::rec::conf.gpu.mode > 0) confFiles.emplace_back("trirec_gpu.conf");
        if(mu3e::rec::conf.twolayer.mode > 0) confFiles.push_back("trirec_twolayer.conf");
    }

    // default config file
    if(confFiles.empty()) confFiles.emplace_back("trirec.conf");

    trirec_t trirec;

    if(boost::ends_with(input, ".root")) {
        auto inputFile = new mu3e::RootFile(input, "READ");
        if(!inputFile->IsOpen()) {
            mu3e::log::fatal("[main] failed to open file: %s\n", input.c_str());
            exit(EXIT_FAILURE);
        }

        // read config files given on command line
        for(auto& confFile : confFiles) {
            inputFile->config.read_file(confFile);
        }
        // read config stored in input (root) file
        mu3e::rec::conf.read_ptree(inputFile->config.ptree);
        // put version into config
        inputFile->config.ptree.put("rec.version", BOOST_PP_STRINGIZE(MU3E_VERSION));

        if(mu3e::rec::conf.display.mode > 0) {
            trirec.display.init();
        }

        if(!output.empty()) {
            if(boost::filesystem::equivalent(input, output)) {
                mu3e::log::fatal("[main] input == output\n");
                exit(EXIT_FAILURE);
            }

            auto outputFile = new mu3e::RootFile(output, inputFile);

            trirec.root.init(outputFile);
        }

        std::cout << "HALLO" << std::endl;
        trirec.run(inputFile, n, skip);
    }
    else {
        mu3e::log::fatal("[main] unknown input file format\n");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
