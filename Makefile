# Cross-platform Makefile for simple_http_server
# Requires: g++ or clang++
# Dependencies: httplib.h and json.hpp should be in three-party/include/

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

# 自动收集 src 目录下的所有 .cpp 文件（递归）
SRC := $(wildcard src/*.cpp) $(wildcard src/**/*.cpp)

.PHONY: all run clean

all: $(OUT)

$(OUT): $(SRC) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

run: $(OUT)
	./$(OUT)

clean:
	rm -f $(OUT)
