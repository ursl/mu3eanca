#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <string.h>
#include <dirent.h>  /// for directory reading
#include <sys/time.h>
#include <iomanip>

#include "base64.hh"
#include "cdbUtil.hh"

#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/gridfs/bucket.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/options/gridfs/bucket.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>

#include "../../common/json.h"

using json = nlohmann::ordered_json;

namespace {

// -- If BLOB (base64) is too large for a single MongoDB document, store raw bytes in GridFS and
//    replace with blobStorage / blobGridFsId / blobSize (thin payload document).
void offloadLargePayloadBlobToGridFS(json& j, mongocxx::database& db) {
  if (!j.contains("BLOB") || !j["BLOB"].is_string()) return;
  std::string b64 = j["BLOB"].get<std::string>();
  if (b64.size() <= kMaxInlinePayloadB64Chars) return;

  std::string blob = base64_decode(b64);
  std::string hashStr =
      (j.contains("hash") && j["hash"].is_string()) ? j["hash"].get<std::string>() : std::string("payload");

  mongocxx::options::gridfs::bucket bopts;
  bopts.bucket_name(kPayloadGridFsBucketName);
  mongocxx::gridfs::bucket bucket = db.gridfs_bucket(bopts);
  mongocxx::gridfs::uploader uploader = bucket.open_upload_stream(hashStr);
  uploader.write(reinterpret_cast<const std::uint8_t*>(blob.data()), blob.size());
  mongocxx::result::gridfs::upload ures = uploader.close();
  bsoncxx::types::bson_value::view idv = ures.id();
  if (idv.type() != bsoncxx::type::k_oid) {
    throw std::runtime_error("GridFS upload: file id is not ObjectId");
  }
  std::string oidHex = idv.get_oid().value.to_string();

  j.erase("BLOB");
  j["blobStorage"] = "gridfs";
  j["blobGridFsId"] = oidHex;
  j["blobSize"] = static_cast<std::int64_t>(blob.size());
}

// -- Remove GridFS file referenced by a payloads document (payloadBlobs bucket), if any.
void deletePayloadGridFsBlobIfPresent(mongocxx::database& db, bsoncxx::document::view doc) {
  auto bsIt = doc.find("blobStorage");
  auto idIt = doc.find("blobGridFsId");
  if (bsIt == doc.end() || idIt == doc.end()) return;
  if (bsIt->type() != bsoncxx::type::k_string || idIt->type() != bsoncxx::type::k_string) return;
  if (bsIt->get_string().value.to_string() != "gridfs") return;
  std::string oidHex = idIt->get_string().value.to_string();
  std::string hashInfo("?");
  if (doc.find("hash") != doc.end() && doc["hash"].type() == bsoncxx::type::k_string) {
    hashInfo = doc["hash"].get_string().value.to_string();
  }
  try {
    bsoncxx::oid oid(oidHex);
    bsoncxx::types::b_oid bid{oid};
    bsoncxx::types::bson_value::value idwrap{bid};
    mongocxx::options::gridfs::bucket bopts;
    bopts.bucket_name(kPayloadGridFsBucketName);
    mongocxx::gridfs::bucket bucket = db.gridfs_bucket(bopts);
    bucket.delete_file(idwrap.view());
    std::cout << "syncMongo: deleted GridFS blob id=" << oidHex << " hash=" << hashInfo << std::endl;
  } catch (const std::exception& e) {
    std::cerr << "syncMongo: GridFS delete failed id=" << oidHex << " hash=" << hashInfo << ": " << e.what()
              << std::endl;
  }
}

}  // namespace

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::basic::sub_array;
using bsoncxx::builder::basic::sub_document;
using bsoncxx::builder::basic::make_document;


using namespace std;

// ----------------------------------------------------------------------
// syncMongo
// ---------
//
//
// [requires]
//    globaltags/*
//    tags/*
//    payloads/*  (large BLOB: >kMaxInlinePayloadB64Chars base64 -> GridFS bucket payloadBlobs)
//    configs/*
//
// Usage (CDB root = parent of globaltags/, tags/, payloads/):
//
// Full upload (same as legacy --all):
//   ./bin/syncMongo --dir <cdb-root> --host localhost --all
//   ./bin/syncMongo --dir <cdb-root> --host localhost --sync all
//
// Scoped upload (--dir is always <cdb-root>):
//   ./bin/syncMongo --dir <cdb-root> --host localhost --sync gt --name <gt> [--deep] [--del]
//   ./bin/syncMongo --dir <cdb-root> --host localhost --sync tag --name <tag> [--deep] [--del]
//   ./bin/syncMongo --dir <cdb-root> --host localhost --sync payload --name <tagDir> [--del]
//   ./bin/syncMongo --dir <cdb-root> --host localhost --sync payload --name tag_<tag>_iov_<n> [--del]
//
// Legacy (directory is collection folder itself):
//   ./bin/syncMongo --host localhost --dir ~/data/mu3e/json12/globaltags
//   ./bin/syncMongo --host localhost --dir json/payloads
//   ./bin/syncMongo --host localhost --dir runrecords
//   ./bin/syncMongo --host mu3edb0 --dir configs
//
//
// ----------------------------------------------------------------------

bool gDBX(false);

mongocxx::uri gUri;
mongocxx::client gClient;

// ----------------------------------------------------------------------
string shortTimeStamp() {
  char buffer[11];
  time_t t;
  time(&t);
  tm r;
  strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));
  struct timeval tv;
  gettimeofday(&tv, 0);
 
  tm *ltm = localtime(&t);
  int year  = 1900 + ltm->tm_year;
  int month = 1 + ltm->tm_mon;
  int day   = ltm->tm_mday;
  int hour  = ltm->tm_hour;
  int min   = ltm->tm_min;
  int sec   = ltm->tm_sec;
  std::stringstream result;
  result << year << "/"
         << std::setfill('0') << std::setw(2) << month << "/"
         << std::setfill('0') << std::setw(2) << day << " ";
  result << std::setfill('0') << std::setw(2) << hour << ":"
         << std::setfill('0') << std::setw(2) << min << ":"
         << std::setfill('0') << std::setw(2) << sec ;
  return result.str();
}


// ----------------------------------------------------------------------
static string mongoRegexEscape(const string& s) {
  string out;
  const char* special = "\\^$*+?.()|[]{}";
  for (char c : s) {
    if (strchr(special, c)) out.push_back('\\');
    out.push_back(c);
  }
  return out;
}


// ----------------------------------------------------------------------
static vector<string> parseTagsFromGtJsonFile(const string& path) {
  vector<string> tags;
  ifstream ins(path);
  if (!ins.is_open()) return tags;
  stringstream buffer;
  buffer << ins.rdbuf();
  try {
    json j = json::parse(buffer.str());
    if (!j.contains("tags") || !j["tags"].is_array()) return tags;
    for (const auto& el : j["tags"]) {
      if (el.is_string()) tags.push_back(el.get<string>());
    }
  } catch (...) {
  }
  return tags;
}


// ----------------------------------------------------------------------
static bool insertOneJsonFile(const string& filePath, const string& dirName, mongocxx::database& db) {
  ifstream INS(filePath);
  if (!INS.is_open()) {
    cerr << "syncMongo: cannot open " << filePath << endl;
    return false;
  }
  stringstream buffer;
  buffer << INS.rdbuf();
  INS.close();

  string collectionContents = buffer.str();
  json j;
  try {
    j = json::parse(collectionContents);
  } catch (const std::exception& e) {
    cerr << "syncMongo: JSON parse error in " << filePath << ": " << e.what() << endl;
    return false;
  }
  j.erase("_id");
  j["History"] = json::array({json::object({{"date", shortTimeStamp()}, {"comment", "Database entry inserted"}})});

  if (dirName == "payloads") {
    try {
      offloadLargePayloadBlobToGridFS(j, db);
    } catch (const std::exception& e) {
      cerr << "GridFS offload failed for " << filePath << ": " << e.what() << endl;
      return false;
    }
  }

  if (dirName == "runrecords") {
    if (!j.contains("EOR")) {
      cout << "EOR not found in ->" << filePath << "<- ... skipping" << endl;
      return false;
    }
    if (j["EOR"].is_array() && j["EOR"].size() > 1) {
      cout << "multiple EORs found in ->" << filePath << "<- ... skipping" << endl;
      return false;
    }
  }

  collectionContents = j.dump();

  if (dirName == "configs") {
    size_t offset = string("cfgString").size() + 5;
    replaceAll(collectionContents, "\n", "\\n", collectionContents.find("cfgString") + offset);
  }

  if (gDBX) {
    cout << "insert: " << filePath << endl << collectionContents << endl;
    return true;
  }

  auto collection = db[dirName];
  auto insert_one_result = collection.insert_one(bsoncxx::from_json(collectionContents));
  bsoncxx::oid oid = insert_one_result->inserted_id().get_oid().value;
  cout << "inserted " << filePath << " _id=" << oid.to_string() << endl;
  string sfilename = "./inserted/" + filePath.substr(filePath.rfind("/") + 1);
  ofstream ONS(sfilename);
  ONS << collectionContents << endl;
  ONS.close();
  return true;
}


// ----------------------------------------------------------------------
void clearCollection(string scollection, string pattern) {
  vector<string> idCollections;
  
  auto db = gClient["mu3e"];
  auto collection = db[scollection];
  
  if (pattern != "unset") {
    auto gtCursor     = collection.find(document{} << "gt" << open_document << "$regex" << pattern << "$options" << "i" << close_document << finalize);
    cout << "hello" << endl;
    for (auto doc : gtCursor) {
      cout << "*********** Global Tags *** " << endl;
      cout << bsoncxx::to_json(doc, bsoncxx::ExtendedJsonMode::k_relaxed) << endl;
      deletePayloadGridFsBlobIfPresent(db, doc);
      auto delete_one_result = collection.delete_one(doc);
      cout << "*** deleted" << endl;
    }

    auto tagCursor     = collection.find(document{} << "tag" << open_document << "$regex" << pattern << "$options" << "i" << close_document << finalize);
    cout << "hello" << endl;
    for (auto doc : tagCursor) {
      cout << "*********** Tags *** " << endl;
      cout << bsoncxx::to_json(doc, bsoncxx::ExtendedJsonMode::k_relaxed) << endl;
      deletePayloadGridFsBlobIfPresent(db, doc);
      auto delete_one_result = collection.delete_one(doc);
      cout << "*** deleted" << endl;
    }
    
    auto hashCursor    = collection.find(document{} << "hash" << open_document << "$regex" << pattern << "$options" << "i" << close_document << finalize);
    for (auto doc : hashCursor) {
      if (doc["hash"]) {
        cout << doc["hash"].get_string().value.to_string() << " ... deleted" << endl;
      }
      deletePayloadGridFsBlobIfPresent(db, doc);
      auto delete_one_result = collection.delete_one(doc);
    }
    
    auto cfgHashCursor = collection.find(document{} << "cfgHash" << open_document << "$regex" << pattern << "$options" << "i" << close_document << finalize);
    for (auto doc : cfgHashCursor) {
      cout << "*********** cfgHash *** " << endl;
      cout << bsoncxx::to_json(doc, bsoncxx::ExtendedJsonMode::k_relaxed) << endl;
      deletePayloadGridFsBlobIfPresent(db, doc);
      auto delete_one_result = collection.delete_one(doc);
      cout << "*** deleted" << endl;
    }
  } else {
    auto cursor_all = collection.find({});
    cout << "collection " << collection.name()
         << " contains these documents:" << endl;
    for (auto doc : cursor_all) {
      cout << bsoncxx::to_json(doc, bsoncxx::ExtendedJsonMode::k_relaxed) << endl;
      // string h = doc["cfgHash"].get_string().value.to_string();
      // cout << "** h = " << h << endl;
      deletePayloadGridFsBlobIfPresent(db, doc);
      auto delete_one_result = collection.delete_one(doc);
      cout << "delete_one_result = " << delete_one_result->deleted_count() << endl;
    }
}

}


// ----------------------------------------------------------------------
static void deleteGtExact(mongocxx::database& db, const string& gt) {
  auto r = db["globaltags"].delete_one(document{} << "gt" << gt << finalize);
  if (r) cout << "delete globaltags gt=" << gt << " count=" << r->deleted_count() << endl;
}


// ----------------------------------------------------------------------
static void deleteTagExact(mongocxx::database& db, const string& tag) {
  auto r = db["tags"].delete_one(document{} << "tag" << tag << finalize);
  if (r) cout << "delete tags tag=" << tag << " count=" << r->deleted_count() << endl;
}


// ----------------------------------------------------------------------
static void deletePayloadHashExact(mongocxx::database& db, const string& hash) {
  auto c = db["payloads"];
  auto existing = c.find_one(document{} << "hash" << hash << finalize);
  if (existing) {
    deletePayloadGridFsBlobIfPresent(db, existing->view());
  }
  auto r = c.delete_one(document{} << "hash" << hash << finalize);
  if (r) cout << "delete payloads hash=" << hash << " count=" << r->deleted_count() << endl;
}


// ----------------------------------------------------------------------
static void deletePayloadsForTagDir(mongocxx::database& db, const string& tagdir) {
  string rx = "^tag_" + mongoRegexEscape(tagdir) + "_iov_";
  auto c = db["payloads"];
  auto filter = document{} << "hash" << open_document << "$regex" << rx << close_document << finalize;
  bsoncxx::document::view fv = filter.view();
  for (auto doc : c.find(fv)) {
    deletePayloadGridFsBlobIfPresent(db, doc);
  }
  auto r = c.delete_many(fv);
  if (r) cout << "delete payloads for tag dir " << tagdir << " count=" << r->deleted_count() << endl;
}


// ----------------------------------------------------------------------
static void printSyncMongoUsage(const char* prog) {
  cout <<
    "syncMongo — upload JSON CDB files to MongoDB (database mu3e).\n"
    "\n"
    "On-disk layout:\n"
    "  CDB root contains: globaltags/, tags/, payloads/ (and optionally configs/, runrecords/).\n"
    "  Payload files live under payloads/<tag>/<runblock>/tag_<tag>_iov_<n>.\n"
    "\n"
    "Options:\n"
    "  --dir <path>   With --sync: CDB root. Legacy: path to one collection folder\n"
    "                 (globaltags, tags, payloads, runrecords, or configs).\n"
    "  --host <host>  MongoDB host (default localhost), port 27017.\n"
    "  --all          Upload full tree: globaltags + tags + payloads (same as --sync all).\n"
    "  --sync <what>  all | gt | tag | payload\n"
    "  --name <name>  Required for gt, tag, payload (see below).\n"
    "  --deep         With gt or tag: also upload member tags and/or all payload files\n"
    "                 under payloads/<tag>/ for each tag.\n"
    "  --del          Delete matching Mongo documents before insert (see below).\n"
    "  --noupload     After optional --del, skip all inserts (delete-only / no re-upload).\n"
    "  --pat <re>     Legacy: regex when clearing/scanning a single collection directory.\n"
    "  -d             Debug (no insert; print parsed documents).\n"
    "  --help, -?     This message.\n"
    "\n"
    "--sync modes (--dir is the CDB root):\n"
    "  all       Run legacy upload on globaltags/, tags/, and payloads/.\n"
    "  gt        Insert globaltags/<name>. With --deep: for each tag listed in that GT file,\n"
    "            insert tags/<tag> if present, then every file under payloads/<tag>/.\n"
    "  tag       Insert tags/<name>. With --deep: every file under payloads/<name>/.\n"
    "  payload   If --name is tag_<tag>_iov_<n>: single payload file.\n"
    "            Otherwise --name is a tag directory: all files under payloads/<name>/.\n"
    "\n"
    "  With --del: exact delete on gt / tag / hash, or all payloads with hash matching\n"
    "  ^tag_<name>_iov_ for a tag-directory payload sync. With --sync gt --deep --del,\n"
    "  deletes use the on-disk GT file to know which tags to clear.\n"
    "  Payload deletes also remove large-BLOB GridFS files (bucket payloadBlobs) when present.\n"
    "\n"
    "Large BLOB: base64 larger than the inline limit is stored in GridFS (bucket payloadBlobs).\n"
    "\n"
    "Examples:\n"
    "  " << prog << " --dir ~/cdb --host localhost --all\n"
    "  " << prog << " --dir ~/cdb --host localhost --sync gt --name datav6.3=2025 --deep --del\n"
    "  " << prog << " --dir ~/cdb --host localhost --sync payload --name pixeltimecalibration_mcidealv6.5\n"
    "  " << prog << " --dir ~/cdb --host localhost --sync payload --name tag_foo_iov_1 --del\n"
    "  " << prog << " --dir ~/cdb --host localhost --sync tag --name mytag --del --deep --noupload\n"
    "\n";
}


// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  // mongocxx::instance already created in cdbMongo.cc (in libCDB.so)

  for (int i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-?")) {
      printSyncMongoUsage(argc > 0 ? argv[0] : "syncMongo");
      return 0;
    }
  }

  string dirPath("fixme"), pattern("unset"), host("localhost");
  string syncKind;
  string syncName;
  bool deep(false);
  bool all(false);
  bool noDeletion(true);
  bool noUpload(false);

  for (int i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "-d")) {
      gDBX = true;
    } else if (!strcmp(argv[i], "--all")) {
      all = true;
    } else if (!strcmp(argv[i], "--dir") && i + 1 < argc) {
      dirPath = string(argv[++i]);
    } else if (!strcmp(argv[i], "--del")) {
      noDeletion = false;
    } else if (!strcmp(argv[i], "--host") && i + 1 < argc) {
      host = string(argv[++i]);
    } else if (!strcmp(argv[i], "--pat") && i + 1 < argc) {
      pattern = string(argv[++i]);
    } else if (!strcmp(argv[i], "--sync") && i + 1 < argc) {
      syncKind = string(argv[++i]);
    } else if (!strcmp(argv[i], "--name") && i + 1 < argc) {
      syncName = string(argv[++i]);
    } else if (!strcmp(argv[i], "--deep")) {
      deep = true;
    } else if (!strcmp(argv[i], "--noupload")) {
      noUpload = true;
    }
  }

  string uriString = "mongodb://" + host + ":27017";
  cout << "uriString ->" << uriString << "<-" << endl;
  gUri = bsoncxx::string::view_or_value(uriString);
  gClient = gUri;
  cout << "gUri set" << endl;

  if (!dirPath.empty() && dirPath.back() == '/') {
    dirPath.pop_back();
  }

  auto db = gClient["mu3e"];

  // -- Scoped sync first (--all must not override --sync gt|tag|payload)
  if (!syncKind.empty() && syncKind != "all") {
    if (dirPath == "fixme" || dirPath.empty()) {
      cerr << "syncMongo: --dir <cdb-root> required with --sync" << endl;
      return 1;
    }
    if (syncName.empty()) {
      cerr << "syncMongo: --name required with --sync " << syncKind << endl;
      return 1;
    }

    if (syncKind == "gt") {
      string gtFile = pathJoin(pathJoin(dirPath, "globaltags"), syncName);
      if (!noDeletion) {
        if (deep) {
          vector<string> ttags = parseTagsFromGtJsonFile(gtFile);
          for (const string& t : ttags) {
            deletePayloadsForTagDir(db, t);
            deleteTagExact(db, t);
          }
        }
        deleteGtExact(db, syncName);
      }
      if (!noUpload) {
        if (!insertOneJsonFile(gtFile, "globaltags", db)) {
          cerr << "syncMongo: failed to insert global tag file " << gtFile << endl;
          return 1;
        }
        if (deep) {
          vector<string> ttags = parseTagsFromGtJsonFile(gtFile);
          for (const string& t : ttags) {
            string tf = pathJoin(pathJoin(dirPath, "tags"), t);
            if (fileExists(tf)) {
              insertOneJsonFile(tf, "tags", db);
            } else {
              cerr << "syncMongo: missing tag file (skip): " << tf << endl;
            }
            string pdir = pathJoin(pathJoin(dirPath, "payloads"), t);
            vector<string> pfiles = allPayloadPaths(pdir);
            for (const string& pf : pfiles) {
              insertOneJsonFile(pf, "payloads", db);
            }
          }
        }
      }
      return 0;
    }

    if (syncKind == "tag") {
      string tagFile = pathJoin(pathJoin(dirPath, "tags"), syncName);
      if (!noDeletion) {
        if (deep) {
          deletePayloadsForTagDir(db, syncName);
        }
        deleteTagExact(db, syncName);
      }
      if (!noUpload) {
        if (!insertOneJsonFile(tagFile, "tags", db)) {
          cerr << "syncMongo: failed to insert tag file " << tagFile << endl;
          return 1;
        }
        if (deep) {
          string pdir = pathJoin(pathJoin(dirPath, "payloads"), syncName);
          for (const string& pf : allPayloadPaths(pdir)) {
            insertOneJsonFile(pf, "payloads", db);
          }
        }
      }
      return 0;
    }

    if (syncKind == "payload") {
      if (isPayloadHashBasename(syncName)) {
        string pf = pathJoin(pathJoin(dirPath, "payloads"), payloadSubPathFromHash(syncName));
        if (!noDeletion) {
          deletePayloadHashExact(db, syncName);
        }
        if (!noUpload) {
          if (!insertOneJsonFile(pf, "payloads", db)) {
            cerr << "syncMongo: failed to insert payload file " << pf << endl;
            return 1;
          }
        }
        return 0;
      }
      string pdir = pathJoin(pathJoin(dirPath, "payloads"), syncName);
      if (!noDeletion) {
        deletePayloadsForTagDir(db, syncName);
      }
      vector<string> pfiles = allPayloadPaths(pdir);
      if (!noUpload) {
        if (pfiles.empty()) {
          cerr << "syncMongo: no payload files under " << pdir << endl;
          return 1;
        }
        for (const string& pf : pfiles) {
          insertOneJsonFile(pf, "payloads", db);
        }
      }
      return 0;
    }

    cerr << "syncMongo: unknown --sync " << syncKind << " (use gt|tag|payload|all)" << endl;
    return 1;
  }

  // -- Full tree: globaltags + tags + payloads (legacy --all or --sync all)
  if (all || syncKind == "all") {
    if (dirPath == "fixme" || dirPath.empty()) {
      cerr << "syncMongo: --dir <cdb-root> required" << endl;
      return 1;
    }
    string exe = argv[0];
    string h = " --host " + host;
    string nu = noUpload ? " --noupload" : "";
    int e1 = system((exe + " --dir " + pathJoin(dirPath, "globaltags") + h + nu).c_str());
    int e2 = system((exe + " --dir " + pathJoin(dirPath, "tags") + h + nu).c_str());
    int e3 = system((exe + " --dir " + pathJoin(dirPath, "payloads") + h + nu).c_str());
    return (e1 != 0 || e2 != 0 || e3 != 0) ? 1 : 0;
  }

  // -- Legacy: --dir points at globaltags, tags, payloads, runrecords, or configs
  string dirName = dirPath.substr(dirPath.rfind("/") + 1);
  vector<string> vfiles;
  if (dirName == "runrecords") {
    vfiles = allRunRecordPaths(dirPath);
  } else if (dirName == "payloads") {
    vfiles = allPayloadPaths(dirPath);
  } else {
    DIR* folder = opendir(dirPath.c_str());
    if (folder == NULL) {
      cout << "Unable to read directory ->" << dirPath << "<-" << endl;
      return 0;
    }
    struct dirent* entry;
    while ((entry = readdir(folder))) {
      if (8 == entry->d_type) {
        vfiles.push_back(dirPath + "/" + entry->d_name);
      }
    }
    closedir(folder);
    sort(vfiles.begin(), vfiles.end());
  }

  cout << "dirPath ->" << dirPath << "<-" << endl;
  cout << "dirName ->" << dirName << "<-" << endl;
  if (!noDeletion) {
    if (dirName != "runrecords" && dirName != "configs") {
      cout << "clearCollection(" << dirName << ");" << endl;
      clearCollection(dirName, pattern);
    } else {
      cout << "NO! Do NOT clearCollection(" << dirName << ");" << endl;
    }
  }

  if (!noUpload) {
    for (const string& it : vfiles) {
      if (pattern != "unset" && string::npos == it.find(pattern)) {
        continue;
      }
      insertOneJsonFile(it, dirName, db);
    }
  }

  return 0;
}
