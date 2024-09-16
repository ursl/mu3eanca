#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <dirent.h>  /// for directory reading
#include <fstream>
#include <iostream>
#include <vector>

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using namespace std;

mongocxx::uri gUri;
mongocxx::client gClient;

// ----------------------------------------------------------------------
void store_file_in_mongo(const std::string& filepath, const std::string& tag, mongocxx::collection& collection) {
  // -- Open the file
  ifstream file(filepath, std::ios::binary);
  if (!file.is_open()) {
    cout << "Failed to open file: " << filepath << endl;
    return;
  }
  
  vector<uint8_t> buffer((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
  
  bsoncxx::builder::stream::document document{};

  document << "tag" << tag  // Add tag
           << "filename" << filepath  // Add filename
           << "content" << bsoncxx::types::b_binary{
    bsoncxx::binary_sub_type::k_binary,
      static_cast<uint32_t>(buffer.size()),
      buffer.data()
      };

    
  
  // Insert the document into the MongoDB collection
  collection.insert_one(document.view());
  
  cout << "Inserted file: " << filepath << " with tag: " << tag << endl;
}

// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
  mongocxx::instance instance{};  // This must be done only once per application
  mongocxx::client client{gUri};  // Connect to MongoDB
  
  // Access the database and collection
  auto db = gClient["mu3e"];
  mongocxx::collection collection = db["detconfigs"];
  
  string dirPath("."), tag("nada");
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "--dir")) {dirPath = string(argv[++i]);}
    if (!strcmp(argv[i], "-dir"))  {dirPath = string(argv[++i]);}
    if (!strcmp(argv[i], "--uri")) {gUri = bsoncxx::string::view_or_value(argv[++i]); gClient = gUri;}
    if (!strcmp(argv[i], "-u"))    {gUri = bsoncxx::string::view_or_value(argv[++i]); gClient = gUri;}
    if (!strcmp(argv[i], "-t"))    {tag = string(argv[++i]);}
    if (!strcmp(argv[i], "--tag")) {tag = string(argv[++i]);}
  }
  
  vector<string> vfiles;
  DIR *folder;
  struct dirent *entry;
  
  folder = opendir(dirPath.c_str());
  if (folder == NULL) {
    cout << "Unable to read directory ->" << dirPath << "<-" << endl;
  } else {
  
    while ((entry=readdir(folder))) {
      if (8 == entry->d_type) {
        vfiles.push_back(dirPath + "/" + entry->d_name);
      }
    }
    closedir(folder);
    sort(vfiles.begin(), vfiles.end());
  }
  
  // -- Iterate over the files and store them in MongoDB
  for (const auto& filepath : vfiles) {
    store_file_in_mongo(filepath, tag, collection);
  }
  
  return 0;
}
