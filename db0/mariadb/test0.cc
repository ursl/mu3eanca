#include "conncpp.hpp"
#include "tests_config.h"

#include <string>
#include <iostream>
#include <system_error>

// ----------------------------------------------------------------------
// test0.cc
// --------
//
// Starting point for mariadb studies
//
// Usage: ./test0 127.0.0.1 root #### test
//                                +- the usual l_tester password!
//
//
// ----------------------------------------------------------------------

using namespace std;

int loops = 2;

static sql::Driver * driver = nullptr;

static sql::Connection *
get_connection(const std::string & host, const std::string & user, const std::string & pass, bool useTls=TEST_USETLS) {
  try {
    /* There will be concurrency problem if we had threads, but don't have, then it's ok */
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
      /* We need to run tests for client- and server-side prepared statements. That also gives
       much more sense for these tests to be run twice */
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

static void driver_test_new_driver_exception() {
}

/* {{{	*/
int main(int argc, const char **argv)
{
  driver_test_new_driver_exception();

  return run_tests(argc, argv);
}
/* }}} */

