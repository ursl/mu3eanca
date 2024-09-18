#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <fstream>
#include <iostream>
#include <vector>

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using namespace std;

mongocxx::uri gUri;
mongocxx::client gClient;

// ----------------------------------------------------------------------
void retrieve_files_from_mongo(const std::string& tag, mongocxx::collection& collection, string dir) {
  // Query to find all documents with the given tag
  bsoncxx::builder::stream::document filter_builder;
  filter_builder << "tag" << tag;
  
  // Execute the query
  auto cursor = collection.find(filter_builder.view());
  
  // Iterate over the documents and retrieve the files
  for (auto&& doc : cursor) {
    // Extract the filename
    std::string filename = doc["filename"].get_utf8().value.to_string();
    
    // Extract the binary content
    auto binary_content = doc["content"].get_binary();
    
    // Write the content to a file
    string fullname = dir + "/" + filename;
    cout << "Saving to ->" << fullname << "<-" << endl;
    std::ofstream outfile(fullname, std::ios::binary);
    if (!outfile.is_open()) {
      std::cerr << "Failed to open file for writing: " << filename << std::endl;
      continue;
    }
    
    // Write the binary data to the file
    outfile.write(reinterpret_cast<const char*>(binary_content.bytes), binary_content.size);
    outfile.close();
    
    std::cout << "Retrieved and saved file: " << filename << std::endl;
  }
}


// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {

  string dirPath("."), tag("nada");
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "--dir")) {dirPath = string(argv[++i]);}
    if (!strcmp(argv[i], "-dir"))  {dirPath = string(argv[++i]);}
    if (!strcmp(argv[i], "--uri")) {gUri = bsoncxx::string::view_or_value(argv[++i]); gClient = gUri;}
    if (!strcmp(argv[i], "-u"))    {gUri = bsoncxx::string::view_or_value(argv[++i]); gClient = gUri;}
    if (!strcmp(argv[i], "-t"))    {tag = string(argv[++i]);}
    if (!strcmp(argv[i], "--tag")) {tag = string(argv[++i]);}
  }

  // Access the database and collection
  auto db = gClient["mu3e"];
  mongocxx::collection collection = db["detconfigs"];

  // Retrieve files with the specified tag
  retrieve_files_from_mongo(tag, collection, dirPath);
  
  return 0;
}

