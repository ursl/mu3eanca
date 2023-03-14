INCMONGODB = -I/usr/local/include/mongocxx/v_noabi -I/usr/local/include/bsoncxx/v_noabi
LIBMONGODB = -lmongocxx -lbsoncxx

ifeq ($(OSTYPE),darwin)
  INCMONGODB = /opt/software/FIXME/include/mariadb++/
  LIBMONGODB = /opt/software/FIXME/lib/
endif

CXX := /usr/bin/c++

CXXFLAGS := -std=c++1z

all: test1 test2 test3

test3: test3.cc
	$(CXX) $(INCMONGODB) -O0 -g -DNDEBUG -std=gnu++11 -o test3.o -c test3.cc
	$(CXX) -O0 -g -DNDEBUG -rdynamic test3.o -o test3 $(LIBMONGODB)

test2: test2.cc
	$(CXX) $(INCMONGODB) -O2 -g -DNDEBUG -std=gnu++11 -o test2.o -c test2.cc
	$(CXX) -O2 -g -DNDEBUG -rdynamic test2.o -o test2 $(LIBMONGODB)

test1: test1.cc
	$(CXX) $(INCMONGODB) -O2 -g -DNDEBUG -std=gnu++11 -o test1.o -c test1.cc
	$(CXX) -O2 -g -DNDEBUG -rdynamic test1.o -o test1 $(LIBMONGODB)

clean:
	rm -f *.o test1 test2 test3 

