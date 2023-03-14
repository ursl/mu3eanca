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

int ranrun(int maxrun){
    return (rand()/((RAND_MAX/maxrun)));
}

int main(int argc, char* argv[]) {
    
  // -- command line arguments
  int nruns(-1);
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i],"-n"))  {nruns   = atoi(argv[++i]);}
  }
    
    mongocxx::instance instance{};
    mongocxx::uri uri("mongodb://127.0.0.1:27017/?directConnection=true&serverSelectionTimeoutMS=2000&appName=mongosh+1.7.1");
    mongocxx::client client(uri);
    
    mongocxx::database db = client["mu3e"];
    
    mongocxx::collection calibrations = db["calibrations"];
    
    cout << calibrations.name() << endl;
    
    int maxrun = maxrunnumber(db);

    double sum = 0;

    std::cout << "nruns = " << nruns << std::endl;
    
    for(int i=0; i < nruns; i ++){

        int run = ranrun(maxrun);

        bsoncxx::document::value filter = document{} 
                << "StartRun" << run 
                << "Version" << 1 
                << "Type" << "PixelAlignment" << finalize;  
        auto result = calibrations.find_one(filter.view());

        if(result){
            bsoncxx::document::element sensors = (result->view())["Sensors"];
            if(sensors && sensors.type() == bsoncxx::type::k_array){
                auto sensarray{sensors.get_array().value};
                for(auto sd : sensarray){
                  std::cout << run << ": sensorid = "
                            << sd["sensorid"].get_int32()
                            << " vx = " << sd["vx"].get_double()
                            << std::endl;
                    sum += sd["vx"].get_double();
                    sum += sd["vy"].get_double();
                    sum += sd["vz"].get_double();
                    sum += sd["rowx"].get_double();
                    sum += sd["rowy"].get_double();
                    sum += sd["rowz"].get_double();
                    sum += sd["colx"].get_double();
                    sum += sd["coly"].get_double();
                    sum += sd["colz"].get_double();
                }
            }

        } else {
            cout << "Not found: Run " << run << endl;
        }
    }
    
    cout << "Sum " << sum << endl;

    return 0;
}
