ifeq ($(OSTYPE),darwin)
  INCMARIADB = /opt/software/mariadbpp/include/mariadb++/
  LIBMARIADB = /opt/software/mariadbpp/lib/
else
  INCMARIADB = /home/l_tester/mariadb-connector-cpp/libmariadb/include
  LIBMARIADB = /usr/local/lib64/mariadb/libmariadb.so.3 /usr/local/lib64/mariadb/libmariadbcpp.so
endif

all:
	/usr/bin/c++  -I$(INCMARIADB) -O2 -g -DNDEBUG -std=gnu++11 -o test0.o -c test0.cc
	/usr/bin/c++ -O2 -g -DNDEBUG -rdynamic test0.o -o test0  

clean:
	rm *.o test0
