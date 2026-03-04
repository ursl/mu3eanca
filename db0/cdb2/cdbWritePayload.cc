#include "cdbPayloadWriter.hh"

#include <iostream>
#include <string.h>

using namespace std;

// ----------------------------------------------------------------------
// cdbWritePayload
// ---------------
//
//  -c pixelalignment    produce the pixelalignment payloads
//  -c tilealignment     produce the tilealignment payloads
//  -c fibrealignment   produce the fibrealignment payloads
//  -c mppcalignment    produce the mppcalignment payloads
//  -d inputfiledir
//  -f filename         file to read in
//  -g GT               the global tag for the payload
//  -i RUN              the interval of validity
//  -p payloaddir       the CDB directory (payloaddir/payloads/)
//  -a annotation       comment for the payload
//
// ----------------------------------------------------------------------

int main(int argc, const char* argv[]) {
  cdbPayloadWriter writer;
  writer.run(argc, argv);
  return 0;
}
