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

  conn->setSchema("Mu3e");
  res = stmt->executeQuery("select RunNumber, RunDescription, Mu3eSchema, StartTime, EndTime  from RunCollection");
  while (res->next()) {
    cout << "RunNumber: " << res->getString("RunNumber") << endl;
    // Access column data by alias or column name 
    cout << "StartTime: " << res->getString("StartTime") << endl;
    // Access column data by numeric offset, 1 is the first column 
    // cout << res->getString(1) << endl;
    // playing
    cout << "Mu3eSchema: " << res->getString("Mu3eSchema") << endl;
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

  const std::string host(argc >=2 ? argv[1] : HOST_ENV_OR_DEFAULT);
  const std::string user(argc >=3 ? argv[2] : UID_ENV_OR_DEFAULT);
  const std::string pass(argc >=4 ? argv[3] : PASSWD_ENV_OR_DEFAULT);
  const std::string database(argc >=5 ? argv[4] : SCHEMA_ENV_OR_DEFAULT);
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
  
  if (!conn->isValid(10)) {
    printf("\n# ERR: Connection is not valid at %s::%d\n", CPPCONN_FUNC, __LINE__);
    printf("not ok\n");
    return 1;
  } else {
    std::cout << "all is well" << std::endl;
  }

  executeQuery(conn, "");
  
  return 0;
}
