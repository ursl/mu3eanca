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
LFILES = anaLadder.o anaEnsemble.o

LBINARIES = runAnaLadder

FILES = $(LFILES)
BINARIES = $(LBINARIES)

LLIBRARIES = lib/libCalLaddertests.so lib/libAnaUtil.so lib/libCDB.so 


# -- Default rules
$(addprefix obj/,%.o) : %.cc %.hh
	$(CXX) $(CXXFLAGS) -DLOCALDIR=\"$(LDIRECTORY)\"  $(EXTHEADERS) -c $< -o $@

$(addprefix obj/,%.o) : %.cc
	$(CXX) $(CXXFLAGS) -DLOCALDIR=\"$(LDIRECTORY)\"  $(EXTHEADERS) -c $< -o $@

$(addprefix bin/,%) : $(addprefix obj/,%.o)
	$(CXX) $(LDFLAGS) -DNDEBUG -rdynamic $< -o $@ $(LLIBRARIES) $(GLIBS) 


# -- targets
all: prep lib bin

lib: lib/libCalLaddertests.so

bin: $(addprefix bin/,$(BINARIES) )

lib/libCalLaddertests.so: $(addprefix obj/,$(FILES) )
	$(CXX) $(SOFLAGS) $(addprefix obj/,$(FILES)) -o $(LLIBRARIES) $(GLIBS)


clean:
	rm -f obj/*.o lib/*.so bin/*

# -- preparatory setup
prep:
	mkdir -p obj bin lib
	cd lib && rm -f libAnaUtil.so && rm -f libCDB.so && ln -s ../../../util/lib/libAnaUtil.so && ln -s ../../../db0/cdb2/lib/libCDB.so && cd -
