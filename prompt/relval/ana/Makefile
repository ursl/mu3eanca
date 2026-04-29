CXX      := g++
CXXFLAGS := -O0 -g -Wall -Wextra -std=c++17
ROOTCFLAGS := $(shell root-config --cflags)
ROOTLIBS   := $(shell root-config --libs)

SOURCES  := runFillHistograms.cc fillHist.cc  # compareHist.cc
OBJECTS  := obj/runFillHistograms.o obj/fillHist.o # obj/compareHist.o 
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
