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

#include <cstdlib>


using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::sub_array;
using bsoncxx::builder::basic::sub_document;

using std::cout;
using std::endl;

int maxrunnumber(mongocxx::database & db){
    mongocxx::collection runs = db["runs"];
    auto order = document{} << "$natural" << -1 << finalize;
    auto opts = mongocxx::options::find{};
    opts.sort(order.view());
    opts.limit(1);
    
    auto cursor = runs.find({},opts);
    auto d0 = *cursor.begin();
    return d0["_id"].get_int32();
}

double rd(){
    return static_cast<double> (rand()/(static_cast<double> (RAND_MAX/800)))-400;
}

int main(int argc, char* argv[]) {
    
    
    mongocxx::instance instance{};
    mongocxx::uri uri("mongodb://127.0.0.1:27017/?directConnection=true&serverSelectionTimeoutMS=2000&appName=mongosh+1.7.1");
    mongocxx::client client(uri);
    
    mongocxx::database db = client["mu3e"];
    
    mongocxx::collection calibrations = db["calibrations"];
    
    cout << calibrations.name() << endl;
    
    cout << maxrunnumber(db) << endl;
    
    for(int i=1; i <= maxrunnumber(db) ; i++){
        bsoncxx::builder::basic::document doc{};
        doc.append(kvp("Schema",1));
        doc.append(kvp("Version",1));
        doc.append(kvp("StartRun",i));
        doc.append(kvp("EndRun",i));
        doc.append(kvp("Type","PixelAlignment"));
        doc.append(kvp("Sensors",[](sub_array sensor){
            for(int s=0; s <3000; s++){
                sensor.append([s](sub_document subdoc){
                    subdoc.append(kvp("sensorid",s));
                    subdoc.append(kvp("vx",rd()));
                    subdoc.append(kvp("vy",rd()));
                    subdoc.append(kvp("vz",rd()));
                    subdoc.append(kvp("colx",rd()));
                    subdoc.append(kvp("coly",rd()));
                    subdoc.append(kvp("colz",rd()));
                    subdoc.append(kvp("rowx",rd()));
                    subdoc.append(kvp("rowy",rd()));
                    subdoc.append(kvp("rowz",rd()));                   
                });
            }
        }));
        
        bsoncxx::stdx::optional<mongocxx::result::insert_one> result =
        calibrations.insert_one(doc.view());
        if(!result)   cout << "Failed to insert" << endl;
    
    }

    return 0;
}
