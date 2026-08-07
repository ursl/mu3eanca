CXX      := g++
CXXFLAGS := -O0 -g -Wall -Wextra -std=c++17
ROOTCFLAGS := $(shell root-config --cflags)
ROOTLIBS   := $(shell root-config --libs)

COMMON_OBJS := obj/fillHist.o obj/compareHist.o
FILL_TARGET := bin/runFillHistograms
CMP_TARGET  := bin/runCompareHistograms
TARGETS     := $(FILL_TARGET) $(CMP_TARGET)

.PHONY: all clean

all: obj bin $(TARGETS)

$(FILL_TARGET): obj/runFillHistograms.o obj/fillHist.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(ROOTLIBS)

$(CMP_TARGET): obj/runCompareHistograms.o obj/compareHist.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(ROOTLIBS)

obj/%.o: %.cc
	$(CXX) $(CXXFLAGS) $(ROOTCFLAGS) -c $< -o $@

obj:
	mkdir -p obj

bin:
	mkdir -p bin

clean:
	rm -f obj/*.o $(TARGETS)
