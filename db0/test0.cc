/* 
//////////////////////////////////////////////////////////////////////
merlin-l-001>c++ -fPIC -I/psi/home/langenegger/data/mysql-connector-cpp -I/usr/include/mysql -c test0.cc
merlin-l-001>c++ -shared /usr/lib64/mysql/libmysqlclient_r.so test0.o -o test0
merlin-l-001>./test0 
Segmentation fault
////////////////////////////////////////////////////////////////////// 
*/


#include "mysql.h"

#include <cppconn/driver.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

using namespace std;

int main(void) {
cout << endl;
cout << "Running 'SELECT 'Hello World!'  AS _message'..." << endl;

  sql::Driver *driver;
  sql::Connection *con;
  sql::Statement *stmt;
  sql::ResultSet *res;

  /* Create a connection */
  driver = get_driver_instance();
  con = driver->connect("tcp://127.0.0.1:3306", "root", "root");
  /* Connect to the MySQL test database */
  con->setSchema("test");

  stmt = con->createStatement();
  res = stmt->executeQuery("SELECT 'Hello World!' AS _message");
  while (res->next()) {
    cout << "\t... MySQL replies: ";
    /* Access column data by alias or column name */
    cout << res->getString("_message") << endl;
    cout << "\t... MySQL says it again: ";
    /* Access column data by numeric offset, 1 is the first column */
    cout << res->getString(1) << endl;
  }
  delete res;
  delete stmt;
  delete con;

cout << endl;

return EXIT_SUCCESS;
}
