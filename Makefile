# Cross-platform Makefile for simple_http_server
# Requires: git, g++ or clang++

# Detect Windows for .exe suffix
ifeq ($(OS),Windows_NT)
	EXE := .exe
else
	EXE :=
endif

CXX ?= g++
# Windows-specific flags for httplib
ifeq ($(OS),Windows_NT)
	CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Ithree-party/include -D_WIN32_WINNT=0x0A00
	LDFLAGS ?= -lws2_32
else
	CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Ithree-party/include
	LDFLAGS ?=
endif

BIN_DIR := bin
OUT := $(BIN_DIR)/simple_http_server$(EXE)
SRC := src/main.cpp

# Header-only dependency: cpp-httplib
HTTPLIB_HDR := three-party/include/httplib.h
HTTPLIB_REPO := https://github.com/yhirose/cpp-httplib.git
HTTPLIB_CLONE_DIR := three-party/cpp-httplib

.PHONY: all run clean distclean deps

all: $(OUT)

$(OUT): deps $(SRC) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Fetch header if missing
deps: $(HTTPLIB_HDR)

$(HTTPLIB_HDR):
	@mkdir -p three-party/include
	@if [ ! -d "$(HTTPLIB_CLONE_DIR)" ]; then \
		git clone --depth 1 $(HTTPLIB_REPO) $(HTTPLIB_CLONE_DIR); \
	fi
	@cp -f $(HTTPLIB_CLONE_DIR)/httplib.h $(HTTPLIB_HDR)

run: $(OUT)
	./$(OUT)

clean:
	rm -f $(OUT)

# Remove fetched sources and headers as well
distclean: clean
	rm -rf $(HTTPLIB_CLONE_DIR) three-party/include
