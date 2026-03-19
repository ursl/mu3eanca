#include "cdbPayloadWriter.hh"

#include <iostream>
#include <string.h>

using namespace std;

// ----------------------------------------------------------------------
// cdbRunPayloadWriter
// -------------------
// 
// This is basically a "run" executable for cdbPayloadWriter and 
// special calls there (in particular the creation of code segments
// for the creation of lists of sensor, tile, and fibre IDs)
//
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
int main(int argc, const char* argv[]) {
  // -- command line arguments 
  string mode("pixeltimecalibration");
  string filename("unset");

  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-m")) {  mode = argv[++i];  }
    if (!strcmp(argv[i], "-f")) {  filename = argv[++i];  }
  }

  cdbPayloadWriter writer;

  if (mode == "createsensorids") {
    if (filename == "unset") {
      cout << "Error: filename is unset" << endl;
      cout << "Usage: cdbWriteIdealInputFiles -m createsensorids -f filename" << endl;
      return 0;
    }
    writer.createSensorIDs(filename);
    return 0;
  }

  if (mode == "createtileids") {
    if (filename == "unset") {
      cout << "Error: filename is unset" << endl;
      cout << "Usage: cdbWriteIdealInputFiles -m createtileids -f filename" << endl;
      return 0;
    }
    writer.createTileIDs(filename);
    return 0;
  }

  if (mode == "createfibreids") {
    if (filename == "unset") {
      cout << "Error: filename is unset" << endl;
      cout << "Usage: cdbWriteIdealInputFiles -m createfibreids -f filename" << endl;
      return 0;
    }
    writer.createFibreIDs(filename);
    return 0;
  }

  if (mode == "pixeltimecalibration") {
    if (filename == "unset") {
      cout << "Error: filename is unset" << endl;
      cout << "Usage: cdbWriteIdealInputFiles -m pixeltimecalibration -f filename" << endl;
      return 0;
    }
    writer.writePixelTimeCalibrationIdealInput(filename, mode);
    return 0;
  }
  
  
  return 0;

}