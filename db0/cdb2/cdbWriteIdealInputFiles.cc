#include "cdbPayloadWriter.hh"

#include <iostream>
#include <string.h>

using namespace std;

// ----------------------------------------------------------------------
// cdbWriteIdealInputFiles
// -----------------------
//
// ----------------------------------------------------------------------

void callX() {
  cout << "Calling X" << endl;
  double eff(0.);
  for (int i = 0; i < 800; i++) {
    for (int j = 1; j < 10000; j++) {
      eff = double(i)/double(j);
      // 39.258688
      if (eff > 0.392586 && eff < 0.392587) {
        cout << "i: " << i << ", j: " << j << ", eff: " << eff << endl;
      }
    }
  }
}

int main(int argc, const char* argv[]) {
  // -- command line arguments 
  string mode("pixeltimecalibration");
  string filename("unset");

  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-m")) {  mode = argv[++i];  }
    if (!strcmp(argv[i], "-f")) {  filename = argv[++i];  }
    if (!strcmp(argv[i], "-x")) { callX(); return 0;}
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