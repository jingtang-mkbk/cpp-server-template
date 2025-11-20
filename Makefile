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
OBJ_DIR := obj
OUT := $(BIN_DIR)/simple_http_server$(EXE)

# 自动收集 src 目录下的所有 .cpp 文件（递归）
SRC := $(wildcard src/*.cpp) $(wildcard src/**/*.cpp)
# 将 .cpp 文件路径转换为 .o 文件路径（放在 obj 目录下，保持目录结构）
OBJ := $(SRC:src/%.cpp=$(OBJ_DIR)/%.o)

.PHONY: all run clean

all: $(OUT)

# 链接所有目标文件生成可执行文件
$(OUT): $(OBJ) | $(BIN_DIR)
	$(CXX) $(OBJ) -o $(OUT) $(LDFLAGS)

# 编译单个 .cpp 文件为 .o 文件
$(OBJ_DIR)/%.o: src/%.cpp | $(OBJ_DIR)
	@mkdir -p $(@D) 2>/dev/null || mkdir $(@D) 2>nul || true
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR):
	@mkdir -p $(BIN_DIR) 2>/dev/null || mkdir $(BIN_DIR) 2>nul || true

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR) 2>/dev/null || mkdir $(OBJ_DIR) 2>nul || true

run: $(OUT)
	./$(OUT)

clean:
	rm -f $(OUT)
	rm -rf $(OBJ_DIR) 2>/dev/null || rmdir /S /Q $(OBJ_DIR) 2>nul || true
