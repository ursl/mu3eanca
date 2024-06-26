# ======================================================================
# NOTE: make all
# -----
#
# ======================================================================
.PHONY: prep clean vars

# -- create a link to an external dependency (https://github.com/ursl/util)
#    no submodule because of constant changes in util
UTIL = ../util

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

CXXFLAGS      = -g -O0 -Wall -fPIC -pipe
LD            = $(CXX)
LDFLAGS       = -g #?? -Wl -rpath$(ROOTSYS)
SOFLAGS       = -shared  #?? -Wl -rpath

CXXFLAGS     += $(ROOTCFLAGS)
GLIBS         = $(filter-out -lz, $(ROOTGLIBS)) -lTMVA -lTMVAGui
EXTHEADERS    = -I..


# -- Files
#CSTUFF =
READER = skimMu3e.o hitDataNoise.o hitDataPixel.o hitDataBase.o plotHitDataPixel.o \
         trBase.o trGen.o trRec.o plotResults.o # plotClass.o

DICT = ${READER:.o=Dict.o}
DICTHEADERS = ${READER:.o=Dict.h}
DICTSOURCES = ${READER:.o=Dict.cc}

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

$(addprefix lib/,%.pcm) :
	cd lib && ln -s $< && cd -


# -- Targets
all: vars prep lib bin

lib: prep $(addprefix obj/,$(CSTUFF) $(READER)  $(DICT))
	$(CXX) $(SOFLAGS) $(addprefix obj/,$(CSTUFF) $(READER) $(DICT)) -o lib/libAnaClasses.so $(GLIBS) -lMinuit $(UTIL)/lib/libAnaUtil.so

bin: lib obj/runSkimMu3e.o obj/runHitDataAna.o obj/runTreeReader.o  obj/runPlot.o obj/anaNoiseMaskFiles.o obj/test1.o
	$(LD) $(LDFLAGS) -o bin/runSkimMu3e obj/runSkimMu3e.o $(GLIBS) lib/libAnaClasses.so $(UTIL)/lib/libAnaUtil.so
	$(LD) $(LDFLAGS) -o bin/runHitDataAna obj/runHitDataAna.o $(GLIBS) lib/libAnaClasses.so $(UTIL)/lib/libAnaUtil.so
	$(LD) $(LDFLAGS) -o bin/runTreeReader obj/runTreeReader.o $(GLIBS) lib/libAnaClasses.so $(UTIL)/lib/libAnaUtil.so
	$(LD) $(LDFLAGS) -o bin/runPlot obj/runPlot.o $(GLIBS) lib/libAnaClasses.so  $(UTIL)/lib/libAnaUtil.so
	$(LD) $(LDFLAGS) -o bin/anaNoiseMaskFiles obj/anaNoiseMaskFiles.o $(GLIBS) lib/libAnaClasses.so  $(UTIL)/lib/libAnaUtil.so
	$(LD) $(LDFLAGS) -o bin/test1 obj/test1.o $(GLIBS)  $(UTIL)/lib/libAnaUtil.so

# -- preparatory setup
prep:
	mkdir -p obj bin lib
	cd lib && ln -f -s ../$(UTIL)/lib/libAnaUtil.so && cd -

# -- clean up
clean:
	rm -f $(addprefix obj/,$(READER) $(DICT) $(CSTUFF) runHitDataAna.o runTreeReader.o runPlot.o)
	rm -f $(DICTHEADERS)  $(DICTSOURCES)
	rm -f bin/*
	rm -f lib/*

cleanresults:
	rm -f results/*.root results/*.pdf results/*.tex results/*.tex.old
