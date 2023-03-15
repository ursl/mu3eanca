INCMARIADB = -I/home/l_tester/mariadb-connector-cpp/libmariadb/include -I/home/l_tester/mariadb-connector-cpp/include
LIBMARIADB = /usr/local/lib64/mariadb/libmariadb.so.3 /usr/local/lib64/mariadb/libmariadbcpp.so

ifeq ($(OSTYPE),darwin)
  INCMARIADB = /opt/software/mariadbpp/include/mariadb++/
  LIBMARIADB = /opt/software/mariadbpp/lib/
endif

CXX := /usr/bin/c++

all: test0 test2 test3 test4 testCstrings

testCstrings: testCstrings.cc
	$(CXX) -O0 -g -DNDEBUG -std=gnu++11 -o testCstrings.o -c testCstrings.cc
	$(CXX) -O0 -g -DNDEBUG -rdynamic testCstrings.o -o testCstrings

test4: test4.cc
	$(CXX) $(INCMARIADB) -O0 -g -DNDEBUG -std=gnu++11 -o test4.o -c test4.cc
	$(CXX) -O0 -g -DNDEBUG -rdynamic test4.o -o test4 $(LIBMARIADB)

test3: test3.cc
	$(CXX) $(INCMARIADB) -O0 -g -DNDEBUG -std=gnu++11 -o test3.o -c test3.cc
	$(CXX) -O0 -g -DNDEBUG -rdynamic test3.o -o test3 $(LIBMARIADB)

test2: test2.cc
	$(CXX) $(INCMARIADB) -O2 -g -DNDEBUG -std=gnu++11 -o test2.o -c test2.cc
	$(CXX) -O2 -g -DNDEBUG -rdynamic test2.o -o test2 $(LIBMARIADB)

test0: test0.cc
	$(CXX) $(INCMARIADB) -O2 -g -DNDEBUG -std=gnu++11 -o test0.o -c test0.cc
	$(CXX) -O2 -g -DNDEBUG -rdynamic test0.o -o test0 $(LIBMARIADB)

clean:
	rm -f *.o test0 test1 test2 test3 test4 testCstrings
