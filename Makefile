CXX := g++
CXXFLAGS := -std=c++17

LLVM_INCLUDE := $(shell llvm-config --includedir)
LLVM_LIBS := $(shell llvm-config --libs)
LLVM_LDFLAGS := $(shell llvm-config --ldflags)
LLVM_SYSTEM_LIBS := $(shell llvm-config --system-libs)

SRC := src/*.cpp
TARGET := bin/compiler

$(shell mkdir -p bin)

build:
	$(CXX) $(CXXFLAGS) -g -I$(LLVM_INCLUDE) $(SRC) -o $(TARGET) $(LLVM_LIBS) $(LLVM_LDFLAGS) $(LLVM_SYSTEM_LIBS)

clean:
	rm -f $(TARGET)