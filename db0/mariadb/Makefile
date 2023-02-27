#<<<<<<< HEAD
ifeq ($(OSTYPE),darwin)
  INCMARIADB = /opt/software/mariadbpp/include/mariadb++/
  LIBMARIADB = /opt/software/mariadbpp/lib/
else
  INCMARIADB = -I/home/l_tester/mariadb-connector-cpp/libmariadb/include -I/home/l_tester/mariadb-connector-cpp/include
  LIBMARIADB = /usr/local/lib64/mariadb/libmariadb.so.3 /usr/local/lib64/mariadb/libmariadbcpp.so
endif

all: test0 test2 test3
	/usr/bin/c++ $(INCMARIADB) -O2 -g -DNDEBUG -std=gnu++11 -o test0.o -c test0.cc
	/usr/bin/c++ -O2 -g -DNDEBUG -rdynamic test0.o -o test0 $(LIBMARIADB)

test4: test4.cc
	/usr/bin/c++ -O0 -g -DNDEBUG -std=gnu++11 -o test4.o -c test4.cc
	/usr/bin/c++ -O0 -g -DNDEBUG -rdynamic test4.o -o test4

test3: test3.cc
	/usr/bin/c++ $(INCMARIADB) -O0 -g -DNDEBUG -std=gnu++11 -o test3.o -c test3.cc
	/usr/bin/c++ -O0 -g -DNDEBUG -rdynamic test3.o -o test3 $(LIBMARIADB)

test2: test2.cc
	/usr/bin/c++ $(INCMARIADB) -O2 -g -DNDEBUG -std=gnu++11 -o test2.o -c test2.cc
	/usr/bin/c++ -O2 -g -DNDEBUG -rdynamic test2.o -o test2 $(LIBMARIADB)

test0: test0.o
	/usr/bin/c++ $(INCMARIADB) -O2 -g -DNDEBUG -std=gnu++11 -o test0.o -c test0.cc
	/usr/bin/c++ -O2 -g -DNDEBUG -rdynamic test0.o -o test0 $(LIBMARIADB)

#=======
#all:
#	test1 test0
#
#test0:
#	/usr/bin/c++  -I/home/l_tester/mariadb-connector-cpp/libmariadb/include  -I/home/l_tester/mariadb-connector-cpp/include -I/home/l_tester/mariadb-connector-cpp/include/conncpp -I/home/l_tester/mariadb-connector-cpp/src  -O2 -g -DNDEBUG -std=gnu++11 -o test0.o -c test0.cc
#	/usr/bin/c++ -O2 -g -DNDEBUG -rdynamic test0.o -o test0  /usr/local/lib64/mariadb/libmariadb.so.3 /usr/local/lib64/mariadb/libmariadbcpp.so
#
#test1:
#	/usr/bin/c++  -I/home/l_tester/mariadb-connector-cpp/libmariadb/include  -I/home/l_tester/mariadb-connector-cpp/include -I/home/l_tester/mariadb-connector-cpp/include/conncpp -I/home/l_tester/mariadb-connector-cpp/src  -O2 -g -DNDEBUG -std=gnu++11 -o test1.o -c test1.cc
#	/usr/bin/c++ -O2 -g -DNDEBUG -rdynamic test1.o -o test1  /usr/local/lib64/mariadb/libmariadb.so.3 /usr/local/lib64/mariadb/libmariadbcpp.so
#
#>>>>>>> 609f835b198617b6defef7e67c75153640e12c12

clean:
	rm -f *.o test0 test1 test2
