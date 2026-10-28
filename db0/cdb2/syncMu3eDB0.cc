#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string.h>
#include <dirent.h>
#include <curl/curl.h>

#include "cdbUtil.hh"

#include "../../common/json.h"

using json = nlohmann::ordered_json;

using namespace std;

// ----------------------------------------------------------------------
// syncMu3eDB0
// -----------
// Same file walk as syncMongo, but writes go through the CDB REST API
// (POST /cdb/insert/..., DELETE /cdb/...). --host is the REST base
// (http://mu3edb0/cdb) or a hostname, which becomes http://<host>/cdb.
// ----------------------------------------------------------------------

bool gDBX(false);
static string gBase;

static size_t curlWrite(void* ptr, size_t size, size_t nmemb, void* userdata) {
  auto* s = static_cast<string*>(userdata);
  s->append(static_cast<char*>(ptr), size * nmemb);
  return size * nmemb;
}

static string restBaseFromHost(const string& host) {
  if (host.find("://") != string::npos) {
    string b = host;
    while (!b.empty() && b.back() == '/') b.pop_back();
    return b;
  }
  return "http://" + host + "/cdb";
}

static string urlEncode(const string& s) {
  CURL* curl = curl_easy_init();
  if (!curl) return s;
  char* e = curl_easy_escape(curl, s.c_str(), static_cast<int>(s.size()));
  string out = e ? e : "";
  if (e) curl_free(e);
  curl_easy_cleanup(curl);
  return out;
}

struct HttpResult {
  long code{0};
  string body;
};

static HttpResult httpRequest(const string& method, const string& url,
                              const string& body, const vector<string>& headers) {
  HttpResult r;
  CURL* curl = curl_easy_init();
  if (!curl) {
    r.body = "curl init failed";
    return r;
  }
  struct curl_slist* hdrs = nullptr;
  for (const auto& h : headers) hdrs = curl_slist_append(hdrs, h.c_str());
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
  if (method == "POST") {
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, static_cast<curl_off_t>(body.size()));
  }
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWrite);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &r.body);
  CURLcode rc = curl_easy_perform(curl);
  if (rc != CURLE_OK) {
    r.code = 0;
    r.body = curl_easy_strerror(rc);
  } else {
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &r.code);
  }
  curl_slist_free_all(hdrs);
  curl_easy_cleanup(curl);
  return r;
}

static bool httpOk(const HttpResult& r, const string& what) {
  if (r.code >= 200 && r.code < 300) {
    cout << what << " " << r.body << endl;
    return true;
  }
  cerr << what << " failed HTTP " << r.code << " " << r.body << endl;
  return false;
}


static string pathBasename(const string& path) {
  size_t p = path.rfind('/');
  return (p == string::npos) ? path : path.substr(p + 1);
}


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
// POST the file. The server stamps History, checks gt, and offloads a
// large payload BLOB to GridFS.
static bool insertOneJsonFile(const string& filePath, const string& dirName) {
  ifstream INS(filePath);
  if (!INS.is_open()) {
    cerr << "syncMu3eDB0: cannot open " << filePath << endl;
    return false;
  }
  stringstream buffer;
  buffer << INS.rdbuf();
  INS.close();
  string collectionContents = buffer.str();

  if (dirName == "configs") {
    size_t offset = string("cfgString").size() + 5;
    size_t at = collectionContents.find("cfgString");
    if (at != string::npos) {
      replaceAll(collectionContents, "\n", "\\n", at + offset);
    }
  }

  if (gDBX) {
    cout << "insert: " << filePath << endl << collectionContents << endl;
    return true;
  }

  string url = gBase + "/insert/" + dirName;
  string fileHeader = "X-Cdb-File: " + pathBasename(filePath);
  HttpResult r = httpRequest("POST", url, collectionContents,
                             {"Content-Type: application/json", fileHeader});
  if (!httpOk(r, "inserted " + filePath)) return false;

  string sfilename = "./inserted/" + pathBasename(filePath);
  ofstream ONS(sfilename);
  if (ONS) {
    ONS << collectionContents << endl;
  }
  return true;
}


static bool deleteGtExact(const string& gt) {
  string url = gBase + "/globaltags/" + urlEncode(gt);
  return httpOk(httpRequest("DELETE", url, "", {}), "delete globaltags gt=" + gt);
}

static bool deleteTagExact(const string& tag) {
  string url = gBase + "/tags/" + urlEncode(tag);
  return httpOk(httpRequest("DELETE", url, "", {}), "delete tags tag=" + tag);
}

static bool deletePayloadHashExact(const string& hash) {
  string url = gBase + "/payloads/hash/" + urlEncode(hash);
  return httpOk(httpRequest("DELETE", url, "", {}), "delete payloads hash=" + hash);
}

static bool deletePayloadsForTagDir(const string& tagdir) {
  string url = gBase + "/payloads?tagDir=" + urlEncode(tagdir);
  return httpOk(httpRequest("DELETE", url, "", {}), "delete payloads for tag dir " + tagdir);
}

static bool clearCollection(const string& scollection, const string& pattern) {
  string url = gBase + "/clear/" + urlEncode(scollection);
  if (pattern != "unset") url += "?pat=" + urlEncode(pattern);
  else url += "?all=1";
  return httpOk(httpRequest("DELETE", url, "", {}), "clearCollection(" + scollection + ")");
}


static void printUsage(const char* prog) {
  cout <<
    "syncMu3eDB0 — upload JSON CDB files through the REST API.\n"
    "\n"
    "On-disk layout:\n"
    "  CDB root contains: globaltags/, tags/, payloads/ (and optionally configs/, runrecords/).\n"
    "  Payload files live under payloads/<tag>/<runblock>/tag_<tag>_iov_<n>.\n"
    "\n"
    "Options:\n"
    "  --dir <path>   With --sync: CDB root. Legacy: path to one collection folder.\n"
    "  --host <url>   REST base (default http://mu3edb0/cdb). A bare hostname\n"
    "                 becomes http://<host>/cdb.\n"
    "  --all          Upload globaltags + tags + payloads.\n"
    "  --sync <what>  all | gt | tag | payload\n"
    "  --name <name>  Required for gt, tag, payload.\n"
    "  --deep         With gt or tag: also upload member tags and payloads.\n"
    "  --del          Delete matching documents before insert.\n"
    "  --noupload     After optional --del, skip inserts.\n"
    "  --pat <re>     Legacy: regex when clearing one collection directory.\n"
    "  -d             Debug (print the JSON, do not POST).\n"
    "  --help, -?     This message.\n"
    "\n"
    "POST /insert/<collection>  body = the JSON file, header X-Cdb-File = basename.\n"
    "The server stamps History, checks gt against the filename, and stores a\n"
    "payload BLOB larger than 10 MB of base64 in GridFS (bucket payloadBlobs).\n"
    "\n"
    "Examples:\n"
    "  " << prog << " --dir ~/cdb --host http://mu3edb0/cdb --sync payload --name tag_foo_iov_1\n"
    "  " << prog << " --dir ~/cdb --host mu3edb0 --sync tag --name eventstuffv2_datav7.1=2026V0 --del\n"
    "\n";
}


int main(int argc, char* argv[]) {
  for (int i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-?")) {
      printUsage(argc > 0 ? argv[0] : "syncMu3eDB0");
      return 0;
    }
  }

  string dirPath("fixme"), pattern("unset"), host("mu3edb0");
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

  gBase = restBaseFromHost(host);
  cout << "REST ->" << gBase << "<-" << endl;
  curl_global_init(CURL_GLOBAL_DEFAULT);

  if (!dirPath.empty() && dirPath.back() == '/') {
    dirPath.pop_back();
  }

  int status = 0;

  if (!syncKind.empty() && syncKind != "all") {
    if (dirPath == "fixme" || dirPath.empty()) {
      cerr << "syncMu3eDB0: --dir <cdb-root> required with --sync" << endl;
      status = 1;
    } else if (syncName.empty()) {
      cerr << "syncMu3eDB0: --name required with --sync " << syncKind << endl;
      status = 1;
    } else if (syncKind == "gt") {
      string gtFile = pathJoin(pathJoin(dirPath, "globaltags"), syncName);
      if (!noDeletion) {
        if (deep) {
          for (const string& t : parseTagsFromGtJsonFile(gtFile)) {
            deletePayloadsForTagDir(t);
            deleteTagExact(t);
          }
        }
        deleteGtExact(syncName);
      }
      if (!noUpload) {
        if (!insertOneJsonFile(gtFile, "globaltags")) {
          cerr << "syncMu3eDB0: failed to insert global tag file " << gtFile << endl;
          status = 1;
        } else if (deep) {
          for (const string& t : parseTagsFromGtJsonFile(gtFile)) {
            string tf = pathJoin(pathJoin(dirPath, "tags"), t);
            if (fileExists(tf)) insertOneJsonFile(tf, "tags");
            else cerr << "syncMu3eDB0: missing tag file (skip): " << tf << endl;
            for (const string& pf : allPayloadPaths(pathJoin(pathJoin(dirPath, "payloads"), t))) {
              insertOneJsonFile(pf, "payloads");
            }
          }
        }
      }
    } else if (syncKind == "tag") {
      string tagFile = pathJoin(pathJoin(dirPath, "tags"), syncName);
      if (!noDeletion) {
        if (deep) deletePayloadsForTagDir(syncName);
        deleteTagExact(syncName);
      }
      if (!noUpload) {
        if (!insertOneJsonFile(tagFile, "tags")) {
          cerr << "syncMu3eDB0: failed to insert tag file " << tagFile << endl;
          status = 1;
        } else if (deep) {
          for (const string& pf : allPayloadPaths(pathJoin(pathJoin(dirPath, "payloads"), syncName))) {
            insertOneJsonFile(pf, "payloads");
          }
        }
      }
    } else if (syncKind == "payload") {
      if (isPayloadHashBasename(syncName)) {
        string pf = pathJoin(pathJoin(dirPath, "payloads"), payloadSubPathFromHash(syncName));
        if (!noDeletion) deletePayloadHashExact(syncName);
        if (!noUpload && !insertOneJsonFile(pf, "payloads")) {
          cerr << "syncMu3eDB0: failed to insert payload file " << pf << endl;
          status = 1;
        }
      } else {
        string pdir = pathJoin(pathJoin(dirPath, "payloads"), syncName);
        if (!noDeletion) deletePayloadsForTagDir(syncName);
        vector<string> pfiles = allPayloadPaths(pdir);
        if (!noUpload) {
          if (pfiles.empty()) {
            cerr << "syncMu3eDB0: no payload files under " << pdir << endl;
            status = 1;
          }
          for (const string& pf : pfiles) insertOneJsonFile(pf, "payloads");
        }
      }
    } else {
      cerr << "syncMu3eDB0: unknown --sync " << syncKind << " (use gt|tag|payload|all)" << endl;
      status = 1;
    }
  } else if (all || syncKind == "all") {
    if (dirPath == "fixme" || dirPath.empty()) {
      cerr << "syncMu3eDB0: --dir <cdb-root> required" << endl;
      status = 1;
    } else {
      string exe = argv[0];
      string h = " --host " + gBase;
      string nu = noUpload ? " --noupload" : "";
      int e1 = system((exe + " --dir " + pathJoin(dirPath, "globaltags") + h + nu).c_str());
      int e2 = system((exe + " --dir " + pathJoin(dirPath, "tags") + h + nu).c_str());
      int e3 = system((exe + " --dir " + pathJoin(dirPath, "payloads") + h + nu).c_str());
      status = (e1 != 0 || e2 != 0 || e3 != 0) ? 1 : 0;
    }
  } else {
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
        curl_global_cleanup();
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
        if (pattern != "unset" && string::npos == it.find(pattern)) continue;
        insertOneJsonFile(it, dirName);
      }
    }
  }

  curl_global_cleanup();
  return status;
}
