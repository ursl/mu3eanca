#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cstring>

#include "cdbJSON.hh"
#include "cdbRest.hh"
#include "calPixelAlignment.hh"
#include "calFibreAlignment.hh"
#include "calMppcAlignment.hh"
#include "calTileAlignment.hh"
#include "calPixelQualityLM.hh"
#include "calFibreQuality.hh"
#include "calTileQuality.hh"
#include "calPixelEfficiency.hh"
#include "calPixelTimeCalibration.hh"

using namespace std;

// ----------------------------------------------------------------------
// cdbSummaryGT
// ------------
// Summarize a global tag: name, number of tags, comment, and per-tag:
// tag name, IOV length, comment.
//
// Usage: ./cdbSummaryGT -u <URI> -g <GT>
//
//   -u, --uri   CDB URI: path for cdbJSON, or http(s) URL for cdbRest
//   -g, --gt    Global tag name (required)
//
// Examples:  ./bin/cdbSummaryGT -u http://mu3edb0/cdb
//            ./bin/cdbSummaryGT -u ~/data/mu3e/cdb/ -g datav6.5=2025V0
// ----------------------------------------------------------------------

void printUsage(const char* progname) {
  cerr << "Usage: " << progname << " -u <URI> -g <GT>" << endl;
  cerr << "  -u, --uri   CDB URI (path for JSON, http URL for REST)" << endl;
  cerr << "  -g, --gt    Global tag name" << endl;
}

// ----------------------------------------------------------------------
// Wrap text at ~width chars, breaking at space when possible.
// Returns string with newlines; continuation lines are prefixed with indent.
string wrapComment(const string& text, size_t width = 60, const string& indent = "             ") {
  if (text.empty()) return "";
  string result;
  size_t pos = 0;
  bool first = true;
  while (pos < text.size()) {
    size_t chunk = (pos + width < text.size()) ? width : text.size() - pos;
    size_t breakPos = pos + chunk;
    if (breakPos < text.size()) {
      size_t lastSpace = text.rfind(' ', breakPos);
      if (lastSpace != string::npos && lastSpace > pos) breakPos = lastSpace + 1;
    }
    if (!first) result += indent;
    result += text.substr(pos, breakPos - pos);
    if (breakPos < text.size()) result += "\n";
    pos = breakPos;
    while (pos < text.size() && text[pos] == ' ') pos++;
    first = false;
  }
  return result;
}

// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  string uri;
  string gt("unset");
  int verbose = 0;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-u") == 0 && i + 1 < argc) uri = argv[++i];
    else if (strcmp(argv[i], "--uri") == 0 && i + 1 < argc) uri = argv[++i];
    else if (strcmp(argv[i], "-g") == 0 && i + 1 < argc) gt = argv[++i];
    else if (strcmp(argv[i], "--gt") == 0 && i + 1 < argc) gt = argv[++i];
    else if (strcmp(argv[i], "-v") == 0 && i + 1 < argc) verbose = atoi(argv[++i]);
    else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      printUsage(argv[0]);
      return 0;
    }
  }

  if (uri.empty() || gt.empty()) {
    cerr << "Error: -u URI and -g GT are required" << endl;
    printUsage(argv[0]);
    return 1;
  }

  cdbAbs* pDB = nullptr;
  if (uri.find("http://") == 0 || uri.find("https://") == 0) {
    pDB = new cdbRest(uri, verbose);
  } else {
    pDB = new cdbJSON(uri, verbose);
  }

  vector<string> allGTs = pDB->readGlobalTags();
  bool found = false;
  for (const auto& igt : allGTs) {
    if (igt == gt) {
      found = true;
      break;
    }
  }
  
  if (gt != "unset" && !found) {
    cerr << "cdbSummaryGT: GT '" << gt << "' not found in CDB" << endl;
    cerr << "Available GTs: ";
    for (size_t i = 0; i < allGTs.size(); i++) {
      if (i > 0) cerr << ", ";
      cerr << allGTs[i];
    }
    cerr << endl;
    delete pDB;
    return 1;
  }

  if (gt == "unset") {
    cout << "Available GTs:" << endl;
    for (size_t i = 0; i < allGTs.size(); i++) {
      cout << allGTs[i] << endl;
    }
    return 0;
  }

  vector<string> tags = pDB->readTags(gt);
  map<string, vector<int>> iovs = pDB->readIOVs(tags);
  string gtComment = pDB->getGlobalTagComment(gt);

  cout << "GT: " << gt << endl;
  cout << "  tags: " << tags.size() << endl;
  if (!gtComment.empty()) {
    cout << "  comment: " << wrapComment(gtComment, 60, "           ") << endl;  // 11 spaces to align
  }
  cout << endl;

  int cntTag(1);
  for (const auto& tag : tags) {
    auto it = iovs.find(tag);
    size_t iovLen = (it != iovs.end()) ? it->second.size() : 0;
    string tagComment = pDB->getTagComment(tag);

    cout << setw(2) << cntTag++ << ") " << tag << endl;
    cout << "    IOV length: " << iovLen 
    << (iovLen == 0 ? " XXXXXXXXXX ERROR XXXXXXXXXX" : " ")
    << endl;

    // -- Alignment/quality payload size: read first payload and report detector units
    size_t payloadSize = 0;
    if (it != iovs.end() && !it->second.empty()) {
      int firstIov = it->second[0];
      string hash = "tag_" + tag + "_iov_" + to_string(firstIov);
      calAbs* cal = nullptr;
      if (tag.find("pixelalignment") == 0) cal = new calPixelAlignment(pDB);
      else if (tag.find("fibrealignment") == 0) cal = new calFibreAlignment(pDB);
      else if (tag.find("mppcalignment") == 0) cal = new calMppcAlignment(pDB);
      else if (tag.find("tilealignment") == 0) cal = new calTileAlignment(pDB);
      else if (tag.find("pixelqualitylm") == 0) cal = new calPixelQualityLM(pDB);
      else if (tag.find("fibrequality") == 0) cal = new calFibreQuality(pDB);
      else if (tag.find("tilequality") == 0) cal = new calTileQuality(pDB);
      else if (tag.find("pixelefficiency") == 0) cal = new calPixelEfficiency(pDB);
      else if (tag.find("pixeltimecalibration") == 0) cal = new calPixelTimeCalibration(pDB);
      if (cal) {
        cal->update(hash);
        payloadSize = cal->getPayloadSize();
        delete cal;
      }
    }
    if (payloadSize > 0) {
      cout << "    size: " << payloadSize << " detector units";

      // -- If multiple IOVs: check all payload sizes are the same
      if (it != iovs.end() && it->second.size() > 1) {
        bool allSame = true;
        size_t firstSize = payloadSize;
        calAbs* calIov = nullptr;
        if (tag.find("pixelalignment") == 0) calIov = new calPixelAlignment(pDB);
        else if (tag.find("fibrealignment") == 0) calIov = new calFibreAlignment(pDB);
        else if (tag.find("mppcalignment") == 0) calIov = new calMppcAlignment(pDB);
        else if (tag.find("tilealignment") == 0) calIov = new calTileAlignment(pDB);
        else if (tag.find("pixelqualitylm") == 0) calIov = new calPixelQualityLM(pDB);
        else if (tag.find("fibrequality") == 0) calIov = new calFibreQuality(pDB);
        else if (tag.find("tilequality") == 0) calIov = new calTileQuality(pDB);
        else if (tag.find("pixelefficiency") == 0) calIov = new calPixelEfficiency(pDB);
        else if (tag.find("pixeltimecalibration") == 0) calIov = new calPixelTimeCalibration(pDB);
        if (calIov) {
          for (size_t i = 1; i < it->second.size(); i++) {
            int iov = it->second[i];
            string hashIov = "tag_" + tag + "_iov_" + to_string(iov);
            calIov->update(hashIov);
            size_t sz = calIov->getPayloadSize();
            if (sz != firstSize) {
              if (allSame) cout << endl;  // break from "size" line before first mismatch
              allSame = false;
              cout << "    IOV " << setw(8) << iov << "  size mismatch: has " << sz << " (expected " << firstSize << ")" << endl;
            } else {
              ostringstream oss;
              oss << "    IOV " << setw(8) << iov << "  size matches: OK";
              string s = oss.str();
              s.resize(80, ' ');  // pad so \r overwrites any previous longer line
              cout << s << "\r" << flush;
            }
          }
          delete calIov;
        }
        if (allSame) {
          cout << endl << "    all IOVs same size: OK" << endl;
        }
      }

      // -- Quality vs alignment size check
      if (tag.find("pixelqualitylm") == 0 
          || tag.find("fibrequality") == 0 
          || tag.find("tilequality") == 0
          || tag.find("pixelefficiency") == 0
          || tag.find("pixeltimecalibration") == 0
        ) {
        const char* alignPrefix = nullptr;
        if (tag.find("pixelqualitylm") == 0) {
          alignPrefix = "pixelalignment";
        } else if (tag.find("fibrequality") == 0) {
          alignPrefix = "fibrealignment";
        } else if (tag.find("tilequality") == 0) {
          alignPrefix = "tilealignment";
        } else if (tag.find("pixelefficiency") == 0) {
          alignPrefix = "pixelalignment";
        } else if (tag.find("pixeltimecalibration") == 0) {
          alignPrefix = "pixelalignment";
        }
        string alignTag;
        for (const auto& t : tags) {
          if (t.size() >= strlen(alignPrefix) && t.compare(0, strlen(alignPrefix), alignPrefix) == 0) {
            alignTag = t;
            break;
          }
        }
        if (!alignTag.empty()) {
          auto itAlign = iovs.find(alignTag);
          if (itAlign != iovs.end() && !itAlign->second.empty()) {
            int firstIov = itAlign->second[0];
            string hashAlign = "tag_" + alignTag + "_iov_" + to_string(firstIov);
            calAbs* calAlign = nullptr;
            if (alignTag.find("pixelalignment") == 0) calAlign = new calPixelAlignment(pDB);
            else if (alignTag.find("fibrealignment") == 0) calAlign = new calFibreAlignment(pDB);
            else if (alignTag.find("tilealignment") == 0) calAlign = new calTileAlignment(pDB);
            if (calAlign) {
              calAlign->update(hashAlign);
              size_t alignSize = calAlign->getPayloadSize();
              delete calAlign;
              if (payloadSize == alignSize) {
                cout << "    agrees with alignment size" << endl;
              } else {
                cout << "    mismatch with alignment size (quality " << payloadSize 
                     << " vs alignment " << alignSize << ") XXXXXXXXXX ERROR XXXXXXXXXX" 
                     << endl;
              }
            }
          }
        }
      } else {
        cout << endl;
      }
    }

    if (!tagComment.empty()) {
      cout << "    comment: " << wrapComment(tagComment, 60, "             ") << endl;  // 13 spaces to align
    }
    cout << endl;
  }

  delete pDB;
  return 0;
}
