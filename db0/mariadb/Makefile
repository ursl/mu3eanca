all:
	test1 test0

test0:
	/usr/bin/c++  -I/home/l_tester/mariadb-connector-cpp/libmariadb/include  -I/home/l_tester/mariadb-connector-cpp/include -I/home/l_tester/mariadb-connector-cpp/include/conncpp -I/home/l_tester/mariadb-connector-cpp/src  -O2 -g -DNDEBUG -std=gnu++11 -o test0.o -c test0.cc
	/usr/bin/c++ -O2 -g -DNDEBUG -rdynamic test0.o -o test0  /usr/local/lib64/mariadb/libmariadb.so.3 /usr/local/lib64/mariadb/libmariadbcpp.so

test1:
	/usr/bin/c++  -I/home/l_tester/mariadb-connector-cpp/libmariadb/include  -I/home/l_tester/mariadb-connector-cpp/include -I/home/l_tester/mariadb-connector-cpp/include/conncpp -I/home/l_tester/mariadb-connector-cpp/src  -O2 -g -DNDEBUG -std=gnu++11 -o test1.o -c test1.cc
	/usr/bin/c++ -O2 -g -DNDEBUG -rdynamic test1.o -o test1  /usr/local/lib64/mariadb/libmariadb.so.3 /usr/local/lib64/mariadb/libmariadbcpp.so


clean:
	rm -f *.o test0 test1
