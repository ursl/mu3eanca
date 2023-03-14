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

bool gVerbose = false;

#define NSENS 3000

// ----------------------------------------------------------------------
// test3
// -----
//
// clear, write, and read two databases: `runs` and `calibrations`
//
// Usage:
// ./test3 -c -p PASSWD                 clear all databases
// ./test3 -p PASSWD -w -n 200 -f 0     write 200 runs, starting with 0
// ./test3 -p PASSWD -r
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
void executeMakeRuns(std::unique_ptr<sql::Connection> & conn) {
  std::unique_ptr<sql::Statement> stmt(conn->createStatement());
  sql::ResultSet *res;

  std::string SQL0 = "drop table if exists runs";

  res = stmt->executeQuery(SQL0);

  
  std::string SQL = "create table runs(_id int, StartTime datetime, EndTime datetime, Frames int, DataSize bigint, BeamOn bool, Magnet bool, ShiftCrew varchar(100), RunDescription varchar(100), DeliveredCharge double, SciCatId varchar(100), MD5Sum bigint, primary key(_id));";

  res = stmt->executeQuery(SQL);

}


// ----------------------------------------------------------------------
void executeMakeCalibrations(std::unique_ptr<sql::Connection> & conn) {
  std::unique_ptr<sql::Statement> stmt(conn->createStatement());
  sql::ResultSet *res;

  std::string SQL0 = "drop table if exists calibrations";
  res = stmt->executeQuery(SQL0);

  std::string SQL = "create table calibrations(_id int auto_increment, `Schema` int, Version int, StartRun int, EndRun int, Type varchar(100), Sensors mediumblob, primary key(_id));";
  res = stmt->executeQuery(SQL);
}


// ----------------------------------------------------------------------
void executeReadRuns(std::unique_ptr<sql::Connection> & conn, string cmd = "") {
  std::unique_ptr<sql::Statement> stmt(conn->createStatement());
  sql::ResultSet *res;

  std::string SQL = "select _id, StartTime, EndTime, Frames, DataSize, BeamOn, Magnet, ShiftCrew, RunDescription, DeliveredCharge, SciCatId, MD5Sum from runs";

  std::ios_base::fmtflags f(cout.flags());

  res = stmt->executeQuery(SQL);
  cout << "res->rowsCount() = " << res->rowsCount() << endl;
  while (res->next()) {
    cout << res->getInt(1) << " "
         << res->getString(2) << " "
         << res->getString(3) << " "
         << res->getInt(4) << " "
         << res->getInt(5) << " "
         << res->getInt(6) << " "
         << res->getInt(7) << " "
         << res->getString(8) << " "
         << res->getString(9) << " "
         << res->getFloat(10) << " "
         << res->getString(11) << " "
         << res->getInt(12) << " "
         << endl;
    //cout << "RunNumber: " << res->getString("RunNumber") << endl;
  }
}


// ----------------------------------------------------------------------
void executeReadCalibrations(std::unique_ptr<sql::Connection> & conn, string cmd = "") {
  std::unique_ptr<sql::Statement> stmt(conn->createStatement());
  sql::ResultSet *res;

  std::string SQL = "select `Schema`, Version, StartRun, EndRun, Type, Sensors from calibrations";

  std::ios_base::fmtflags f(cout.flags());

  res = stmt->executeQuery(SQL);
  cout << "res->rowsCount() = " << res->rowsCount() << endl;
  
  blobData a;
  size_t sizePixelSensors = NSENS*(sizeof(blobData)) + 1;
  cout << "executeReadCalibrations sizeof(blobData) = " << sizeof(blobData) << " sizePixelSensors = " << sizePixelSensors << endl;
  char blobStr[sizePixelSensors];
  for (int i = 0; i < sizePixelSensors; ++i) blobStr[i] = 0;

  while (res->next()) {
    int StartRun = res->getInt(3);
    int EndRun = res->getInt(4);
    std::istream *blobPayload = res->getBlob("Sensors");
    
    //    std::istreambuf_iterator<char> isb = std::istreambuf_iterator<char>(*blobPayload);
    blobPayload->read(blobStr, sizePixelSensors);

    // cout << "read/printString: " << printString(blobStr, sizePixelSensors) << endl;
    for (int isens = 0; isens < NSENS; ++isens) {
      a.deSerialize(blobStr + isens*sizeof(blobData)); 
      if (isens == 0 || isens == 1 || isens == 2998 || isens == 2999) {
        cout << "StartRun = " << StartRun << " EndRun = " << EndRun << " "; a.print();
      }
    }
  }
}


// ----------------------------------------------------------------------
void executeWriteRuns(std::unique_ptr<sql::Connection> & conn, int first, int nruns) {
  unique_ptr<sql::Statement> stmt(conn->createStatement());
  string SQL0 = "INSERT INTO runs (_id, StartTime, EndTime, Frames, DataSize, BeamOn, Magnet, ShiftCrew, RunDescription, DeliveredCharge, SciCatId, MD5Sum) VALUES ";

  bool BeamOn(true);
  int Frames(20100);
  int DataSize(30000001);
  double DeliveredCharge(33.76);
  int RunType(22001);
  bool Magnet(true);
  string StartTime, EndTime, ShiftCrew("Peter Pan & Donald Duck"),
    RunDescription("A great data taking run"), SciCatId("Mu3e-2023-000567");
  int MD5Sum(0xaf4544);

  char buff[70];
  struct tm my_time;
  my_time.tm_year = 123; // = year 2012
  my_time.tm_mon  = 1;    // = 10th month
  my_time.tm_mday = 14;   // = 9th day
  my_time.tm_hour = 8;   // = 8 hours
  my_time.tm_min  = 5;   
  my_time.tm_sec  = 7;   

  for (int irun = first; irun < first+nruns; ++irun) {
    DataSize = 700456 + 44*irun;
    Frames   = 60000 + 7*irun;

    my_time.tm_min = irun/58; 
    my_time.tm_sec = irun%58; 
    if (strftime(buff, sizeof buff, "%Y-%m-%d %H:%M:%S", &my_time)) {
      StartTime = string(buff);
    }
    
    my_time.tm_sec += 1;
    if (strftime(buff, sizeof buff, "%Y-%m-%d %H:%M:%S", &my_time)) {
      EndTime = string(buff);
    }

    stringstream s;
    s << "("
      << "'" << irun << "', "
      << "'" << StartTime << "', "
      << "'" << EndTime << "', "
      << "'" << Frames << "', "
      << "'" << DataSize << "', "
      << "'" << BeamOn << "', "
      << "'" << Magnet << "', "
      << "'" << ShiftCrew << "', "
      << "'" << RunDescription << "', "
      << "'" << DeliveredCharge << "', "
      << "'" << SciCatId << "', "
      << "'" << MD5Sum << "'"
      << ")";

    string SQL1 = s.str();
    string SQL = SQL0+SQL1;
    if (gVerbose) cout << SQL << endl;

    sql::ResultSet *res = stmt->executeQuery(SQL);
  }
}


// ----------------------------------------------------------------------
void executeWriteCalibrations(std::unique_ptr<sql::Connection> & conn, int first, int nruns) {
  unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement("INSERT INTO calibrations (`Schema`, Version, StartRun, EndRun, Type, Sensors) VALUES (?,?,?,?,?,?)"));

  int Schema(1);
  int Version(1);
  int StartRun(-1);
  int EndRun(-1);

  stringstream s1;

  blobData a;

  size_t sizePixelSensors = NSENS*sizeof(blobData) + 1;
  char pixelSensors[sizePixelSensors];
  cout << "executeWriteCalibrations sizeof(blobData) = " << sizeof(blobData) << " sizePixelSensors = " << sizePixelSensors << endl;
  for (int i = 0; i < sizePixelSensors; ++i) pixelSensors[i] = 0;
  
  for (int irun = first; irun < first+nruns; ++irun) {
    stmt->setInt(1, 1);
    stmt->setInt(2, 1);
    stmt->setInt(3, irun);
    stmt->setInt(4, irun);
    stmt->setString(5, "PixelAlignment");

    for (int isens = 0; isens < NSENS; ++isens) {
      a.rnd(isens);
      if (gVerbose) {
        if (isens == 0 || isens == 1 || isens == 2998 || isens == 2999) {
          cout << "StartRun = " << irun << " EndRun = " << irun << " "; 
          a.print();
        }
      }
      char *test_data = a.serialize();
      memcpy(pixelSensors + isens*sizeof(blobData), test_data, sizeof(blobData));
    }    
    // cout << "write/printString: " <<  printString(pixelSensors, sizePixelSensors) << endl;
    StreamBufferData buffer0(pixelSensors, sizePixelSensors);
    std::istream test_s0(&buffer0);
    stmt->setBlob(6, &test_s0);

    bool ok = stmt->execute();
 }
}


// ----------------------------------------------------------------------
vector<string> runCommand(std::unique_ptr<sql::Connection> conn, string cmd) {
  std::unique_ptr<sql::Statement> stmt(conn->createStatement());

  stmt->execute(cmd.c_str());

  std::unique_ptr<sql::ResultSet> rset(stmt->getResultSet());
  int found = 0;
  vector<string> lines; 
  while (rset->next()) {
    std::string engine(rset->getString("Engine")), support(rset->getString("Support"));
    std::cout << "# " << engine << "::" << support << std::endl;
    if (engine == "InnoDB" && (support == "YES" || support == "DEFAULT")) {
      found = 1;
      break;
    }
  }

}


// ----------------------------------------------------------------------
int main(int argc, const char **argv) {
  driver_test_new_driver_exception();

  std::unique_ptr<sql::Connection> conn;
  int last_error_total = 0;
  int i;

  // -- command line arguments
  bool doRead(false), doWrite(false), doMake(false), doTest(false);
  int first(-1), nruns(-1);
  string pass;
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i],"-c"))  {doMake   = true;}
    if (!strcmp(argv[i],"-f"))  {first    = atoi(argv[++i]);}
    if (!strcmp(argv[i],"-n"))  {nruns    = atoi(argv[++i]);}
    if (!strcmp(argv[i],"-r"))  {doRead   = true;}
    if (!strcmp(argv[i],"-w"))  {doWrite  = true;}
    if (!strcmp(argv[i],"-p"))  {pass     = argv[++i];}
    if (!strcmp(argv[i],"-t"))  {doTest   = true;}
    if (!strcmp(argv[i],"-v"))  {gVerbose = true;}
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

  if (doMake) {
    executeMakeRuns(conn);
    executeMakeCalibrations(conn);
  }

  if (doWrite) {
    executeWriteRuns(conn, first, nruns);
    executeWriteCalibrations(conn, first, nruns);
  }
  
  if (doRead) {
    executeReadRuns(conn, "");
    executeReadCalibrations(conn, "");
  }

  if (doTest) {
    cout << "doTest" << endl;
    int intVal(1);
    char data[4];
    memcpy(data, "D", 1);
    memcpy(data+1, "E", 1);
    memcpy(data+2, "A", 1);
    memcpy(data+3, "D", 1);
    cout << "data ->";
    for (int i = 0; i < 4; ++i) cout << data[i];
    cout << "<-" << endl;

    char nibbleH, nibbleL;
    data[0] = 0xDA;
    data[0] = 47;
    //    splitNibbles(data[0], nibbleH, nibbleL); 
    //    char nibbleP[3];
    cout << "printString ->" <<  printString(data, 2) << "<-" << endl;
    
  }
  
  return 0;
}
