ROOTCINT      = rootcling
ROOTCFLAGS    = $(shell root-config --cflags)
ROOTGLIBS     = $(shell root-config --glibs)
ROOTLDFLAGS   = $(shell root-config --ldflags)

LHOST = $(shell hostname)
LDIRECTORY = $(shell pwd)


EXTHEADERS    = -I../..

UTIL         := ../../util

CXXFLAGS     := -std=c++1z -g -O0 -Wall -fPIC -pipe 
CXXFLAGS     += $(ROOTCFLAGS)

LD           := $(CXX)
LDFLAGS      := -g -O0  #?? -Wl -rpath$(ROOTSYS)
SOFLAGS      += -g -shared  #?? -Wl -rpath

GLIBS         = $(filter-out -lz, $(ROOTGLIBS))

OSUFFIX := .o

# -- ana files
LFILES = anaLadder.o plotEnsemble.o
FILES = $(LFILES)

LBINARIES = runAnaLadder
BINARIES = $(LBINARIES)

LLIBRARIES = lib/libAnaUtil.so lib/libCDB.so 


# -- Default rules
$(addprefix obj/,%.o) : %.cc %.hh
	$(CXX) $(CXXFLAGS) -DLOCALDIR=\"$(LDIRECTORY)\"  $(EXTHEADERS) -c $< -o $@

$(addprefix obj/,%.o) : %.cc
	$(CXX) $(CXXFLAGS) -DLOCALDIR=\"$(LDIRECTORY)\"  $(EXTHEADERS) -c $< -o $@


# -- targets
all: prep lib bin

lib: $(addprefix obj/,$(FILES))
	echo "hallo lib"
	$(CXX) $(SOFLAGS) $(addprefix obj/,$(FILES)) -o lib/libCalLaddertests.so  $(LLIBRARIES) $(GLIBS)

bin: lib obj/runAnaLadder.o
	echo "hallo bin"
	$(LD) $(LDFLAGS) -o bin/runAnaLadder obj/runAnaLadder.o lib/libCalLaddertests.so  $(LLIBRARIES) $(GLIBS)
#	$(CXX) $(CXXFLAGS)  $(EXTHEADERS) -c runAnaLadder.cc -o bin/runAnaLadder.o



clean:
	rm -f obj/*.o lib/*.so bin/*

# -- preparatory setup
prep:
	mkdir -p obj bin lib
	cd lib && rm -f libAnaUtil.so && rm -f libCDB.so && ln -s ../../../util/lib/libAnaUtil.so && ln -s ../../../db0/cdb2/lib/libCDB.so && cd -
