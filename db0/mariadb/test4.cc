#include "conncpp.hpp"

#include "tests_config.h"

#include <string>
#include <iostream>
#include <system_error>

#include <chrono>
#include <time.h>
#include <cstring>

using namespace std;

int loops = 2;

static sql::Driver * driver = nullptr;

#define NSENS 3000

// ----------------------------------------------------------------------
// test4
// -----
//
// read constants and exercise 
//
// Usage:
// time ./test4 
// Sum = 8.85642e+06
// ----------------------------------------------------------------------



// ----------------------------------------------------------------------
void splitNibbles(char byte, char& nibbleH, char& nibbleL) {
  nibbleH = (byte & 0xF0) >> 4;
  nibbleL = (byte & 0xF);
}


// ----------------------------------------------------------------------
char ascii(char x) {
  if (0x0 == x) return 0x30;
  if (0x1 == x) return 0x31;
  if (0x2 == x) return 0x32;
  if (0x3 == x) return 0x33;
  if (0x4 == x) return 0x34;
  if (0x5 == x) return 0x35;
  if (0x6 == x) return 0x36;
  if (0x7 == x) return 0x37;
  if (0x8 == x) return 0x38;
  if (0x9 == x) return 0x39;
  if (0xa == x) return 0x41;
  if (0xb == x) return 0x42;
  if (0xc == x) return 0x43;
  if (0xd == x) return 0x44;
  if (0xe == x) return 0x45;
  if (0xf == x) return 0x46;
  return 'X';
}


// ----------------------------------------------------------------------
char* printString(char* data, int size) {
  char *sprt = new char[2*size+1];
  char nibbleH, nibbleL;
  for (int i = 0; i < size; ++i) {
    splitNibbles(data[i], nibbleH, nibbleL);
    sprt[2*i]     = ascii(nibbleH);
    sprt[2*i + 1] = ascii(nibbleL);
  }
  sprt[2*size] = '\0';
  return sprt;
}


// ----------------------------------------------------------------------
void putInt(int intVal, char *p) {
  memcpy(p, &intVal, sizeof(int));
}


// ----------------------------------------------------------------------
int getInt(char *p, int len) {
  int intVal(0);
  memcpy(&intVal, p, sizeof intVal);
  return intVal;
}


// ----------------------------------------------------------------------
int getFloat(char *p, int len) {
  float floatVal(0);
  memcpy(&floatVal, p, sizeof floatVal);
  return floatVal;
}


// ----------------------------------------------------------------------
double rd(double id = 800){
  return static_cast<double> (rand()/(static_cast<double> (RAND_MAX/id))) - 0.5*id;
}


// ----------------------------------------------------------------------
struct blobData {
  long int sid;
  double vx, vy, vz;
  double colx, coly, colz;
  double rowx, rowy, rowz;

  char* serialize() {
    int offset(0);
    char *sd = new char[sizeof(blobData)+1];
    if (0) cout << " sizeof(blobData) = " << sizeof(blobData)
                << " sizeof(int) = " << sizeof(int)
                << " sizeof(double) = " << sizeof(double)
                << endl;
    // -- sid
    if (0) cout << "serialize sid = " << sid  << endl;
    memcpy(sd, &sid, sizeof(long int));
    offset += sizeof(long int);
    // -- vx,vy,vz
    if (0) cout << "serialize vx = " << vx
                << " vy = " << vy 
                << " vz = " << vz
                << endl;
    memcpy(sd + offset, &vx, sizeof(double));  offset += sizeof(double);
    memcpy(sd + offset, &vy, sizeof(double));  offset += sizeof(double);
    memcpy(sd + offset, &vz, sizeof(double));  offset += sizeof(double);

    // -- colx,coly,colz
    if (0) cout << "serialize colx = " << colx
                << " coly = " << coly 
                << " colz = " << colz
                << endl;
    memcpy(sd + offset, &colx, sizeof(double));  offset += sizeof(double);
    memcpy(sd + offset, &coly, sizeof(double));  offset += sizeof(double);
    memcpy(sd + offset, &colz, sizeof(double));  offset += sizeof(double);

    // -- rowx,rowy,rowz
    if (0) cout << "serialize rowx = " << rowx
                << " rowy = " << rowy 
                << " rowz = " << rowz
                << endl;
    memcpy(sd + offset, &rowx, sizeof(double));  offset += sizeof(double);
    memcpy(sd + offset, &rowy, sizeof(double));  offset += sizeof(double);
    memcpy(sd + offset, &rowz, sizeof(double));  offset += sizeof(double);

    sd[offset] = '\0';
    if (0) cout << "serialize sd ->" << printString(sd, offset) << "<-" << endl;
    return sd;
  }

  void deSerialize(char *pdata) {
    int offset(0);
    memcpy(&sid, pdata, sizeof(long int));
    // cout << "sid = " << sid << endl;
    // cout << "sd ->" << printString(pdata, sizeof(blobData)) << "<-" << endl;
    offset = sizeof(long int);
    memcpy(&vx, pdata + offset, sizeof(double)); offset += sizeof(double); 
    memcpy(&vy, pdata + offset, sizeof(double)); offset += sizeof(double); 
    memcpy(&vz, pdata + offset, sizeof(double)); offset += sizeof(double); 

    memcpy(&colx, pdata + offset, sizeof(double)); offset += sizeof(double); 
    memcpy(&coly, pdata + offset, sizeof(double)); offset += sizeof(double); 
    memcpy(&colz, pdata + offset, sizeof(double)); offset += sizeof(double); 

    memcpy(&rowx, pdata + offset, sizeof(double)); offset += sizeof(double); 
    memcpy(&rowy, pdata + offset, sizeof(double)); offset += sizeof(double); 
    memcpy(&rowz, pdata + offset, sizeof(double)); offset += sizeof(double); 
  }

  void rnd(int id, int irun = 0) {
    sid  = id;
    double did = static_cast<double>(id + irun*100. + 1);
    vx   = rd(did);
    vy   = rd(did);
    vz   = rd(did);
    colx = rd(did);
    coly = rd(did);
    colz = rd(did);
    rowx = rd(did);
    rowy = rd(did);
    rowz = rd(did);
  }

  void print() {
    cout << "sid = " << sid
         << " v(x/y/z) = (" << vx << "/" << vy << "/" << vz << ")"
         << " c(x/y/z) = (" << colx << "/" << coly << "/" << colz << ")"
         << " r(x/y/z) = (" << rowx << "/" << rowy << "/" << rowz << ")"
         << endl;
  }
};


// ----------------------------------------------------------------------
class StreamBufferData : public std::streambuf {
public:
  StreamBufferData(char *in_data, size_t in_size) {
    setg(in_data, in_data, in_data + in_size);
  }
};


// ----------------------------------------------------------------------
static sql::Connection *get_connection(const std::string & host,
                                       const std::string & user,
                                       const std::string & pass,
                                       bool useTls=TEST_USETLS) {
  try {
    if (!driver) {
      driver = sql::mariadb::get_driver_instance();
    }

    if (loops % 2 && !useTls) {
      return driver->connect(host, user, pass);
    } else {
      sql::ConnectOptionsMap connection_properties;
      connection_properties["hostName"]= host;
      connection_properties["userName"]= user;
      connection_properties["password"]= pass;
      connection_properties["useTls"]=   useTls ? "true" : "false";
      connection_properties["useServerPrepStmts"] = "true";

      return driver->connect(connection_properties);
    }
  } catch (sql::SQLException &e) {
    cout << "sql::SQLException caught during connect" << endl;
    cout << e.what() << endl;
    throw;
  }
}


#define DRIVER_TEST 1
#define TEST_COMMON_TAP_NAME "driver_test"

#include "test_common.cpp"


// ----------------------------------------------------------------------
static void driver_test_new_driver_exception() {
  // We do not export Driver interface implementation
  /*try {
    new sql::mariadb::MariaDbDriver();
    ensure("Exception not thrown", false);
  } catch (sql::InvalidArgumentException&) { }*/
}


// ----------------------------------------------------------------------
int maxRun(std::unique_ptr<sql::Connection> & conn) {
  std::unique_ptr<sql::Statement> stmt(conn->createStatement());
  sql::ResultSet *res;
  
  std::string SQL = "select _id from runs";

  std::ios_base::fmtflags f(cout.flags());

  res = stmt->executeQuery(SQL);
  int maxrun(-1);
  while (res->next()) {
    if (res->getInt(1) > maxrun) maxrun = res->getInt(1);
  }
  return maxrun; 
}


// ----------------------------------------------------------------------
int ranrun(int maxrun){
  return (rand()/((RAND_MAX/maxrun)));
}


// ----------------------------------------------------------------------
bool getSensors(std::unique_ptr<sql::Connection> & conn, int run, vector<blobData> &v) {
  bool result(false);
  std::unique_ptr<sql::Statement> stmt(conn->createStatement());
  sql::ResultSet *res;

  std::string SQL = "select `Schema`, Version, StartRun, EndRun, Type, Sensors from calibrations";

  std::ios_base::fmtflags f(cout.flags());

  res = stmt->executeQuery(SQL);
  
  blobData a;
  size_t sizePixelSensors = NSENS*(sizeof(blobData)) + 1;
  char blobStr[sizePixelSensors];
  for (int i = 0; i < sizePixelSensors; ++i) blobStr[i] = 0;

  while (res->next()) {
    if (run != res->getInt(3)) continue;
    std::istream *blobPayload = res->getBlob("Sensors");
    blobPayload->read(blobStr, sizePixelSensors);
    for (int isens = 0; isens < NSENS; ++isens) {
      a.deSerialize(blobStr + isens*sizeof(blobData)); 
      v[isens] = a;
    }
    result = true;
    delete blobPayload;
    break;
  }
  delete res;
  return result;
}


// ----------------------------------------------------------------------
int main(int argc, const char **argv) {
  driver_test_new_driver_exception();

  std::unique_ptr<sql::Connection> conn;
  int last_error_total = 0;
  int i;

  // -- command line arguments
  int first(-1), nruns(-1);
  string pass;
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i],"-f"))  {first   = atoi(argv[++i]);}
    if (!strcmp(argv[i],"-n"))  {nruns   = atoi(argv[++i]);}
    if (!strcmp(argv[i],"-p"))  {pass    = argv[++i];}
  }

  // -- Configure Connection
  string host("127.0.0.1");
  string user("root");
  string database("Mu3e");

  const bool useTls= USETLS_ENV_OR_DEFAULT;
  
  std::cout << "# Host=" << host << std::endl;
  std::cout << "# User=" << user << std::endl;
  
  std::string connect_method("unknown");
  if (host.find("tcp://", (size_t)0) != std::string::npos) {
    connect_method = "tcp";
  } else if (host.find("unix://", (size_t)0) != std::string::npos) {
    connect_method = "socket";
  } else {
    connect_method = "socket";
  }		
  
  std::cout << "# connect_method=" << connect_method << std::endl;
  
  try {
    conn.reset(get_connection(host, user, pass, useTls));
  } catch (sql::SQLException &e) {
    printf("\n# ERR: Caught sql::SQLException at %s::%d  [%s] (%d/%s)\n",
           CPPCONN_FUNC, __LINE__, e.what(), e.getErrorCode(), e.getSQLStateCStr());
    printf("not ok\n");
    return 1;
  }

  conn->setSchema(database);
 
  if (!conn->isValid(10)) {
    printf("\n# ERR: Connection is not valid at %s::%d\n", CPPCONN_FUNC, __LINE__);
    printf("not ok\n");
    return 1;
  } else {
    std::cout << "all is well" << std::endl;
  }

  cout << "maxrun() = " << maxRun(conn) << endl;


  int maxrun = maxRun(conn);
  
  double sum = 0;

  vector<blobData> a;
  a.reserve(NSENS);
  for (int i = 0; i < NSENS; ++i) {
    blobData bd;
    a.push_back(bd);
  }
  for (int i=0; i < 10000; ++i) {
    int run = ranrun(maxrun);
    bool ok = getSensors(conn, run, a);
    if (0 == i%100) {
      cout << "Trying run " << run << " ok = " << ok
           << " a.size() = " << a.size() 
           << " i = " << i 
           << endl;
    }
    if (ok) {
      for (auto sd : a){
        sum += sd.vx;
        sum += sd.vy;
        sum += sd.vz;
        sum += sd.rowx;
        sum += sd.rowy;
        sum += sd.rowz;
        sum += sd.colx;
        sum += sd.coly;
        sum += sd.colz;
      }
      
    } else {
      cout << "Not found: Run " << run << endl;
    }
  }
  cout << "Sum = " << sum << endl;
  return 0;
}
