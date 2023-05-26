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
FILES = cdb.o cdbAscii.o cdbMongo.o calAbs.o calPixel.o


# -- Default rules
$(addprefix obj/,%.o) : %.cc %.hh
	$(CXX) $(CXXFLAGS) $(INCMONGODB) -c $< -o $@

$(addprefix obj/,%.o) : %.cc
	$(CXX) $(CXXFLAGS) $(INCMONGODB) -c $< -o $@


all: prep lib bin

lib: $(addprefix obj/,$(FILES) main.o)
#	$(CXX) $(INCMONGODB) -O0 -g -DNDEBUG -std=gnu++11 -o obj/main.o -c main.cc
	$(CXX) $(SOFLAGS) $(addprefix obj/,$(FILES)) -o lib/libCDB.so 


bin: obj/main.o
	$(CXX) $(LDFLAGS) -DNDEBUG -rdynamic obj/main.o -o bin/main lib/libCDB.so $(LIBMONGODB)


clean:
	rm -f obj/*.o lib/*.so bin/main 

# -- preparatory setup
prep:
	mkdir -p obj bin lib
