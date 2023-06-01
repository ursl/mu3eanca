ROOTCINT      = rootcling
ROOTCFLAGS    = $(shell root-config --cflags)
ROOTGLIBS     = $(shell root-config --glibs)
ROOTLDFLAGS   = $(shell root-config --ldflags)

INCMONGODB  = -I/usr/local/include/mongocxx/v_noabi
INCMONGODB += -I/usr/local/include/bsoncxx/v_noabi
LIBMONGODB  = -lmongocxx -lbsoncxx

CXX := /usr/bin/c++

CXXFLAGS := -std=c++1z -g -O0 -Wall -fPIC -pipe 
#CXXFLAGS     += $(ROOTCFLAGS)

LD            = $(CXX)
LDFLAGS       = -g -O0 #?? -Wl -rpath$(ROOTSYS)
SOFLAGS       = -shared  #?? -Wl -rpath

#GLIBS         = $(filter-out -lz, $(ROOTGLIBS))


# -- CDB (Conditions DB) files
FILES = cdb.o cdbAscii.o cdbMongo.o calAbs.o calPixel.o calPixelAlignment.o


# -- Default rules
$(addprefix obj/,%.o) : %.cc %.hh
	$(CXX) $(CXXFLAGS) $(INCMONGODB) -c $< -o $@

$(addprefix obj/,%.o) : %.cc
	$(CXX) $(CXXFLAGS) $(INCMONGODB) -c $< -o $@


all: prep lib bin

lib: $(addprefix obj/,$(FILES) )
	$(CXX) $(SOFLAGS) $(addprefix obj/,$(FILES)) -o lib/libCDB.so 


bin: obj/testRun.o obj/testCDB.o obj/fillDB.o
	$(CXX) $(LDFLAGS) -DNDEBUG -rdynamic obj/testRun.o -o bin/testRun lib/libCDB.so $(LIBMONGODB)
	$(CXX) $(LDFLAGS) -DNDEBUG -rdynamic obj/testCDB.o -o bin/testCDB lib/libCDB.so $(LIBMONGODB)
	$(CXX) $(LDFLAGS) -DNDEBUG -rdynamic obj/fillDB.o -o bin/fillDB $(LIBMONGODB)


clean:
	rm -f obj/*.o lib/*.so bin/testCDB

# -- preparatory setup
prep:
	mkdir -p obj bin lib
