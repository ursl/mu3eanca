# ======================================================================
# NOTE: make all
# -----
#
# ======================================================================
.PHONY: prep clean vars

# -- create a link to an external dependency (https://github.com/ursl/util)
#    no submodule because of constant changes in util
UTIL := ../../util
CDB  := ../../db0/cdb2

ROOTCINT      = rootcling
ROOTCFLAGS    = $(shell root-config --cflags)
ROOTGLIBS     = $(shell root-config --glibs)
ROOTLDFLAGS   = $(shell root-config --ldflags)

ifdef SCRAM_ARCH
  CXX         := $(shell scramv1 tool info gcc-cxxcompiler | grep CXX= | sed s/CXX=//)
else
  ifdef CXXCOMPILER
    CXX         := $(CXXCOMPILER)
  else
    CXX         := c++
  endif
endif

CXXFLAGS      = -std=c++1z -g -O0 -Wall -fPIC -pipe
LD            = $(CXX)
LDFLAGS       = -g #?? -Wl -rpath$(ROOTSYS)
SOFLAGS       = -g -shared  #?? -Wl -rpath

CXXFLAGS     += $(ROOTCFLAGS)
GLIBS         = $(filter-out -lz, $(ROOTGLIBS)) -lTMVA -lTMVAGui -lMinuit 
EXTHEADERS    = -I.. -I$(CDB)

OTHERLIBS 	  = $(UTIL)/lib/libAnaUtil.so $(CDB)/lib/libCDB.so


# -- Files
#CSTUFF =
LFILES = 
BINARIES = pixelHistograms pixelFillQualityLM

DICT = ${LFILES :.o=Dict.o}
DICTHEADERS = ${LFILES :.o=Dict.h}
DICTSOURCES = ${LFILES :.o=Dict.cc}

# -- Default rules
$(addprefix obj/,%.o) : %.cc
	$(CXX) $(CXXFLAGS) $(EXTHEADERS) -c $< -o $@

$(addprefix obj/,%.o) : %.cc %.hh
	$(CXX) $(CXXFLAGS) $(EXTHEADERS) -c $< -o $@

$(addprefix obj/,%.o) : %.cc %.hh
	$(CXX) $(CXXFLAGS) $(EXTHEADERS) -c $< -o $@

$(addprefix obj/,%.o) : %.cc
	$(CXX) $(CXXFLAGS) $(EXTHEADERS) -c $< -o $@

%Dict.cc : %.hh %LinkDef.h
	$(ROOTCINT) -f $@ -c $(EXTHEADERS) $^
	mv $(subst Dict.cc,Dict_rdict.pcm,$@) lib

%Dict.cc : %.hh
	$(ROOTCINT) -f $@ -c $(EXTHEADERS) $<
	mv $(subst Dict.cc,Dict_rdict.pcm,$@) lib

$(addprefix bin/,%) : $(addprefix obj/,%.o)
	$(LD) $(LDFLAGS) -o $@  $<  $(GLIBS) $(OTHERLIBS)
 

$(addprefix lib/,%.pcm) :
	cd lib && ln -s $< && cd -


# -- Targets
all: vars prep lib bin

#lib: prep $(addprefix obj/,$(CSTUFF) $(LFILES )  $(DICT))
#	# $(CXX) $(SOFLAGS) $(addprefix obj/,$(CSTUFF) $(LFILES ) $(DICT)) -o lib/libAnaClasses.so $(OTHERLIBS) $(GLIBS) 

bin: lib $(addprefix bin/,$(BINARIES) )

# -- preparatory setup
prep:
	mkdir -p obj bin lib
	cd lib && ln -f -s ../$(UTIL)/lib/libAnaUtil.so && cd -
	cd lib && ln -f -s ../$(CDB)/lib/libCDB.so && cd -

# -- clean up
clean:
	rm -f $(addprefix obj/,$(LFILES ) $(DICT) $(CSTUFF) runHitDataAna.o runTreeLFILES .o runPlot.o)
	rm -f $(DICTHEADERS)  $(DICTSOURCES)
	rm -f bin/*
	rm -f lib/*
	rm -f obj/*

cleanresults:
	rm -f results/*.root results/*.pdf results/*.tex results/*.tex.old
