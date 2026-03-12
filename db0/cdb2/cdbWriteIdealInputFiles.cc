#include "cdbPayloadWriter.hh"

#include <iostream>
#include <string.h>

using namespace std;

// ----------------------------------------------------------------------
// cdbWriteIdealInputFiles
// -----------------------
//
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

  if (mode == "createchipidsperlayer") {
    if (filename == "unset") {
      cout << "Error: filename is unset" << endl;
      cout << "Usage: cdbWriteIdealInputFiles -m createchipidsperlayer -f filename" << endl;
      return 0;
    }
    writer.createChipIDsPerLayer(filename);
  }

  if (mode == "pixeltimecalibration") {
    if (filename == "unset") {
      cout << "Error: filename is unset" << endl;
      cout << "Usage: cdbWriteIdealInputFiles -m pixeltimecalibration -f filename" << endl;
      return 0;
    }
    writer.writePixelTimeCalibrationIdealInput(filename);
  }
  
  
  return 0;

}