CXX      ?= clang++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Isrc
LDFLAGS  ?=

BUILD := build

SRC := src/main.cpp \
       src/core/Web.cpp \
       src/io/Parser.cpp \
       src/io/OutputWriter.cpp \
       src/algo/InterferenceBuilder.cpp \
       src/algo/Allocator.cpp \
       src/ui/Menu.cpp

OBJ := $(patsubst src/%.cpp,$(BUILD)/%.o,$(SRC))
BIN := $(BUILD)/regalloc

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@ $(LDFLAGS)

$(BUILD)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

doc:
	doxygen Doxyfile

clean:
	rm -rf $(BUILD) Documentation

.PHONY: all doc clean
