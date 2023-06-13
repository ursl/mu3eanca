#include "payload.hh"

// ----------------------------------------------------------------------
payload::payload() : fLength(0),
                     fComment("now what?"), fHash("tag_X_iov_Y"), 
                     fBLOB(std::string("empty payload")) {

};
