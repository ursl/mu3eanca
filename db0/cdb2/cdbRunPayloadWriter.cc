#include "cdbPayloadWriter.hh"

#include <iostream>
#include <string.h>

#include <dirent.h>

#include <string>

using namespace std;

// ----------------------------------------------------------------------
// cdbRunPayloadWriter
// -------------------
// 
// This is basically a "run" executable for cdbPayloadWriter and 
// special calls there (in particular the creation of code segments
// for the creation of lists of sensor, tile, and fibre IDs)
// 
// Furthermore
// - pixelmask payloads
//
// Usage examples
// ./bin/cdbRunPayloadWriter -m 2025 -c fibrealignment -f ascii/mu3e_alignment_mcidealv6.5.root -a "as installed in 2025" -p ~/data/mu3e/cdb/payloads -g datav6.5=2025V1
// ./bin/cdbRunPayloadWriter -t datav6.9=2025V0 -a "all frames with perfect data for all subsystems" -p payloads -d /Users/ursl/data/mu3e/run2025/rawv2 -r 1 -c eventstuffv2
//
// ./bin/cdbRunPayloadWriter -m eventstuffv2-ideal -f eventstuff-ideal-test.json
// ./bin/cdbRunPayloadWriter -c eventstuffv2 -a "ideal tag, for inclusion of skipped header bug for pixels" -t mcidealv6.9 -r 1 
// ./bin/cdbRunPayloadWriter -c eventstuffv2 -d ~/data/mu3e/run2025/rawv2 -a "includes skipped header bug for pixels, but nothing on fibres of tiles" -t datav6.9=2025V0 
// 
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
int main(int argc, const char* argv[]) {
  // -- command line arguments 
  string mode("nada");
  string filename("unset");

  string uri("/Users/ursl/data/mu3e/test-cdb");
  string tagname("unset");
  string inputfiledir("unset");
  string annotation("unset");
  int run(1);
  
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-a")) {  annotation = argv[++i];  }
    if (!strcmp(argv[i], "-d")) {  inputfiledir = argv[++i];  }
    if (!strcmp(argv[i], "-f")) {  filename = argv[++i];  }
    if (!strcmp(argv[i], "-m")) {  mode = argv[++i];  }
    if (!strcmp(argv[i], "-r")) {  run = atoi(argv[++i]);  }
    if (!strcmp(argv[i], "-t")) {  tagname = argv[++i];  }
    if (!strcmp(argv[i], "-u")) {  uri = argv[++i];  }
  }

  cdbPayloadWriter writer;

  // -- first filter out the ID writing (not payloads!)
  if (mode == "createsensorids") {
    if (filename == "unset") {
      cout << "Error: filename is unset" << endl;
      cout << "Usage: cdbRunPayloadWriter -m createsensorids -f filename" << endl;
      return 0;
    }
    writer.createSensorIDs(filename);
    return 0;
  }

  if (mode == "createtileids") {
    if (filename == "unset") {
      cout << "Error: filename is unset" << endl;
      cout << "Usage: cdbRunPayloadWriter -m createtileids -f filename" << endl;
      return 0;
    }
    writer.createTileIDs(filename);
    return 0;
  }

  if (mode == "createfibreids") {
    if (filename == "unset") {
      cout << "Error: filename is unset" << endl;
      cout << "Usage: cdbRunPayloadWriter -m createfibreids -f filename" << endl;
      return 0;
    }
    writer.createFibreIDs(filename);
    return 0;
  }

  // -- pixeltimecalibration
  if (mode == "pixeltimecalibration") {
    if (filename == "unset") {
      cout << "Error: filename is unset" << endl;
      cout << "Usage: cdbRunPayloadWriter -m pixeltimecalibration -f filename" << endl;
      return 0;
    }
    writer.writePixelTimeCalibrationIdealInput(filename, mode);
    return 0;
  }
  
  // -- pixelmask
  if (mode.find("pixelmask") != string::npos) {
    if (run > 1 && inputfiledir == "unset") {
      cout << "Error: inputfiledir is unset" << endl;
      cout << "Usage: cdbRunPayloadWriter -u <CDBPATH> -m pixelmask -d <inputfiledir>" << endl;
      return 0;
    }
    if (uri == "unset") {
      cout << "Error: uri is unset" << endl;
      cout << "Usage: cdbRunPayloadWriter -u <CDBPATH> -m pixelmask -d <inputfiledir>" << endl;
      return 0;
    }
    if (run == 1) {
      writer.writePixelMaskIdealPayload(uri+"/payloads/", tagname, annotation, mode);
     return 0;
    } else {
      writer.writePixelMaskPayloads(uri+"/payloads/", tagname, inputfiledir, annotation, run);
      return 0;
    }
  }
  
  // -- IDEAL eventstuffv2 
  if (string::npos != mode.find("eventstuffv2") && string::npos != mode.find("ideal")) {
    if (filename == "unset") {
      cout << "Error: filename is unset" << endl;
      cout << "Usage: cdbRunPayloadWriter -m eventstuffv2-ideal -f filename" << endl;
      return 0;
    }
    writer.writeEventStuffV2IdealInput(filename, mode);
    return 0;
  }

  // -- IDEAL tiletimecalibration 
  if (string::npos != mode.find("tiletimecalibration") && string::npos != mode.find("ideal")) {
    if (filename == "unset") {
      cout << "Error: filename is unset" << endl;
      cout << "Usage: cdbRunPayloadWriter -m tiletimecalibration-ideal -f filename" << endl;
      return 0;
    }
    writer.writeTileTimeCalibrationIdealInput(filename, mode);
    return 0;
  }

  // -- default case: run the main function of cdbPayloadWriter
  writer.run(argc, argv);
  return 0;

}