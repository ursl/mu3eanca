CXX      := g++
CXXFLAGS := -O2 -g -Wall -Wextra -std=c++17
ROOTCFLAGS := $(shell root-config --cflags)
ROOTLIBS   := $(shell root-config --libs)

SOURCES  := HistBooking.cc RelvalAnalyzer.cc runFillHistograms.cc
OBJECTS  := obj/HistBooking.o obj/RelvalAnalyzer.o obj/runFillHistograms.o
TARGET   := bin/runFillHistograms

.PHONY: all clean

all: obj bin $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(ROOTLIBS)

obj/%.o: %.cc
	$(CXX) $(CXXFLAGS) $(ROOTCFLAGS) -c $< -o $@

obj:
	mkdir -p obj

bin:
	mkdir -p bin

clean:
	rm -f $(OBJECTS) $(TARGET)
