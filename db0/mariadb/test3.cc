/*
 * Copyright (c) 2008, 2018, Oracle and/or its affiliates. All rights reserved.
 *               2020, 2021 MariaDB Corporation AB
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2.0, as
 * published by the Free Software Foundation.
 *
 * This program is also distributed with certain software (including
 * but not limited to OpenSSL) that is licensed under separate terms,
 * as designated in a particular file or component or in included license
 * documentation.  The authors of MySQL hereby grant you an
 * additional permission to link the program and your derivative works
 * with the separately licensed software that they have included with
 * MySQL.
 *
 * Without limiting anything contained in the foregoing, this file,
 * which is part of MySQL Connector/C++, is also subject to the
 * Universal FOSS Exception, version 1.0, a copy of which can be found at
 * http://oss.oracle.com/licenses/universal-foss-exception.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License, version 2.0, for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
 */


#include "conncpp.hpp"

#include "tests_config.h"

#include <string>
#include <iostream>
#include <system_error>

#include <chrono>
#include <time.h>

using namespace std;

int loops = 2;

static sql::Driver * driver = nullptr;

// ----------------------------------------------------------------------
static sql::Connection *get_connection(const std::string & host,
                                       const std::string & user,
                                       const std::string & pass, bool useTls=TEST_USETLS) {
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
void executeQuery(std::unique_ptr<sql::Connection> & conn, string cmd = "") {
  std::unique_ptr<sql::Statement> stmt(conn->createStatement());
  sql::ResultSet *res;

  std::string SQL = "select RunNumber, Mu3eSchema, StartTime, EndTime, Frames, DataSize, BeamOn, Magnet, RunType from RunCollection";

  res = stmt->executeQuery(SQL);
  cout << "res->rowsCount() = " << res->rowsCount() << endl;
  while (res->next()) {
    cout << res->getInt(1) << " "
         << res->getInt(2) << " "
         << res->getString(3) << " "
         << res->getString(4) << " "
         << res->getString(5) << " "
         << res->getString(6) << " "
         << res->getString(7) << " "
         << res->getString(8) << " "
         << res->getString(9) << " "
         << endl;
    //cout << "RunNumber: " << res->getString("RunNumber") << endl;
  }
}

// ----------------------------------------------------------------------
void executeWrite(std::unique_ptr<sql::Connection> & conn, int first, int nruns) {
  unique_ptr<sql::Statement> stmt(conn->createStatement());
  string SQL0 = "INSERT INTO RunCollection (RunNumber, Mu3eSchema, StartTime, EndTime, Frames, DataSize, BeamOn, Magnet, ShiftCrew, RunDescription, DeliveredCharge, SciCatId, RunType) VALUES ";

  int Mu3eSchema(1001);
  int BeamOn(1900);
  int Frames(20100);
  int DataSize(30000001);
  int DeliveredCharge(720000);
  int RunType(22001);
  float Magnet(1.1);
  string StartTime, EndTime, ShiftCrew("N. Sense, S. Body"), RunDescription("test"), SciCatId("Mu3e-2023-000567");

  char buff[70];
  struct tm my_time;
  my_time.tm_year = 123; // = year 2012
  my_time.tm_mon  = 0;    // = 10th month
  my_time.tm_mday = 31;   // = 9th day
  my_time.tm_hour = 8;   // = 8 hours
  my_time.tm_min  = 5;   
  my_time.tm_sec  = 7;   

  for (int irun = first; irun <= first+nruns; ++irun) {
    DataSize = 700456 + 44*irun;
    Frames   = 60000 + 7*irun;

    my_time.tm_min += 1;    // add 1 min
    if (strftime(buff, sizeof buff, "%Y-%m-%d %H:%M:%S", &my_time)) {
      StartTime = string(buff);
    }
    
    my_time.tm_sec += 2;    // add 2 secs
    if (strftime(buff, sizeof buff, "%Y-%m-%d %H:%M:%S", &my_time)) {
      EndTime = string(buff);
    }

    
    stringstream s;
    s << "("
      << "'" << irun << "', "
      << "'" << Mu3eSchema << "', "
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
      << "'" << RunType << "'"
      << ")";

    string SQL1 = s.str();
    cout << SQL0+SQL1 << endl;
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
  bool doRead(false), doWrite(false);
  int first(-1), nruns(-1);
  string pass;
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i],"-f"))  {first   = atoi(argv[++i]);}
    if (!strcmp(argv[i],"-n"))  {nruns   = atoi(argv[++i]);}
    if (!strcmp(argv[i],"-r"))  {doRead  = true;}
    if (!strcmp(argv[i],"-w"))  {doWrite = true;}
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

  if (doWrite) {
    executeWrite(conn, first, nruns);
  }
  
  if (doRead) {
    executeQuery(conn, "");
  }

  
  
  return 0;
}
