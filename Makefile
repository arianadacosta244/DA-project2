# Plain Makefile fallback (CMake is also provided).
# Build artifacts go to build/  so src/ stays clean.
#
# Usage: `make`        → builds build/regalloc
#        `make doc`    → runs Doxygen (generates Documentation/html)
#        `make clean`  → wipes build/ and Documentation/

CXX      ?= clang++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Isrc
LDFLAGS  ?=

BUILD := build

SRC := src/main.cpp \
       src/Web.cpp \
       src/Parser.cpp \
       src/InterferenceBuilder.cpp \
       src/Allocator.cpp \
       src/OutputWriter.cpp \
       src/Menu.cpp

OBJ := $(patsubst src/%.cpp,$(BUILD)/%.o,$(SRC))
BIN := $(BUILD)/regalloc

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@ $(LDFLAGS)

$(BUILD)/%.o: src/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

doc:
	doxygen Doxyfile

clean:
	rm -rf $(BUILD) Documentation

.PHONY: all doc clean
