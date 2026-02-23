//

#include "mu3e/rec/trirec.h"


#include "Mu3eConditions.hh"
#include "cdbAbs.hh"
#include "cdbJSON.hh"
#include "cdbRest.hh"
#include "calAbs.hh"

/**
 * Print frame id before any other output.
 */
int printf(const char* format, ...) {
    static int64_t frame = -1;

    if(frame != mu3e::rec::conf.runtime.frame) {
        frame = mu3e::rec::conf.runtime.frame;
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

int main(int argc, const char* argv[]) {
    mu3e::rec::set_SIGINT_handler();
    mu3e::rec::parse_argc(argc, argv);

    auto& input = mu3e::rec::conf.runtime.input;
    auto& output = mu3e::rec::conf.runtime.output;
    auto& confFiles = mu3e::rec::conf.runtime.confFiles;
    auto& n = mu3e::rec::conf.runtime.n;
    auto& skip = mu3e::rec::conf.runtime.skip;

    if(!boost::ends_with(input, ".root")) {
        mu3e::log::fatal("[main] unknown input file format\n");
        exit(EXIT_FAILURE);
    }
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

    if (mu3e::rec::conf.conddb.useCDB) {
      std::cout << "initialize conddb" << std::endl;
      if (std::string::npos != mu3e::rec::conf.conddb.dbconn.find("unset")) {
        std::cout << "Error: You cannot set useCDB = true, but have dbconn = unset" << std::endl;
        return EXIT_FAILURE;
      } else {
        std::string dir = mu3e::rec::conf.conddb.dbconn.c_str();
        if (boost::filesystem::is_directory(dir)) {
          std::string gt = mu3e::rec::conf.conddb.globalTag;
          int dbverbose(0);
          cdbAbs *pDB = new cdbJSON(mu3e::rec::conf.conddb.dbconn, dbverbose);
          Mu3eConditions *pDC = Mu3eConditions::instance(gt, pDB);
        } else {
          std::cout << "Error: dbconn ->" << dir << "<- is not an existing directory" << std::endl;
          return EXIT_FAILURE;
        }
      }
    }

    
    mu3e::rec::trirec_t trirec;
    if(!output.empty()) {
        auto outputFile = new mu3e::RootFile(output, inputFile);
        trirec.root.init(outputFile);
    }
    if(!mu3e::rec::conf.runtime.input_mid.empty()) trirec.run_mid(inputFile, n, skip);
    else trirec.run(inputFile, n, skip);

    return EXIT_SUCCESS;
}
