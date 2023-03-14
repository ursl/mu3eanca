#include <iostream>
#include <vector>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <chrono>


using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

using std::cout;
using std::endl;


int main(int argc, char* argv[]) {
    
    
    mongocxx::instance instance{};
    mongocxx::uri uri("mongodb://127.0.0.1:27017/?directConnection=true&serverSelectionTimeoutMS=2000&appName=mongosh+1.7.1");
    mongocxx::client client(uri);
    
    mongocxx::database db = client["mu3e"];
    
    mongocxx::collection runs = db["runs"];
    
    cout << runs.name() << endl;
    
    auto order = document{} << "$natural" << -1 << finalize;
    
    auto opts = mongocxx::options::find{};
    opts.sort(order.view());
    opts.limit(1);
    
    auto cursor = runs.find({},opts);
    auto d0 = *cursor.begin();
    
    int runmax = 0;
    if (0) {
      runmax = d0["_id"].get_int32() + 1; 
    } 
    cout << "runmax = " << runmax << endl;
    
    for(int i=0; i < 1000 ; i++){
    //mongocxx::cursor cursor = runs.find({});
    auto builder = document{};
    bsoncxx::document::value doc_value = builder
        << "_id" << runmax 
        << "StartTime"  << bsoncxx::types::b_date(std::chrono::system_clock::now())
        << "EndTime"    << bsoncxx::types::b_date(std::chrono::system_clock::now())
        << "Frames"     << 60000 + 7*runmax
        << "DataSize"   << 700456 + 44*runmax
        << "BeamOn"     << true
        << "MagnetOn"   << true
        << "ShiftCrew"  << "Peter Pan & Donald Duck"
        << "RunDescription" << "A great data taking run"
        << "DeliveredCharge" << 33.76 
        << "SciCatId"        << "Mu3e-2021-000567"
        << "MD5Sum"          << "0xaf4544"
        << finalize;
    
    bsoncxx::stdx::optional<mongocxx::result::insert_one> result =
    runs.insert_one(doc_value.view());
    
    if(!result)  cout << "Failed to insert" << endl;
    
    runmax++;    
    }
    
    return 0;
}
