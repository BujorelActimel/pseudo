CC = gcc
CFLAGS = -std=c23 -Wall -Wextra -Wpedantic -Iinclude -Itree-sitter-pseudo/bindings/c -Itree-sitter-pseudo/src
LDFLAGS = -lm -ltree-sitter

DEBUG_FLAGS = -g -O0 -fsanitize=address -fsanitize=undefined
RELEASE_FLAGS = -O3 -march=native

SRC_DIR = src
TEST_DIR = test
BUILD_DIR = build
GRAMMAR_DIR = tree-sitter-pseudo

MODE ?= debug

ifeq ($(MODE),release)
    CFLAGS += $(RELEASE_FLAGS)
    BIN_DIR = $(BUILD_DIR)/release
    OBJ_DIR = $(BUILD_DIR)/obj/release
else
    CFLAGS += $(DEBUG_FLAGS)
    BIN_DIR = $(BUILD_DIR)/debug
    OBJ_DIR = $(BUILD_DIR)/obj/debug
endif

# Source files: all .c files under src/ (excluding wasm/ which is Emscripten-only)
SRC_FILES = $(shell find $(SRC_DIR) -name '*.c' ! -path '$(SRC_DIR)/wasm/*')
SRC_OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/src/%.o,$(SRC_FILES))

# Grammar object file
GRAMMAR_OBJ = $(OBJ_DIR)/grammar/parser.o

# Test files: all .c files under test/
TEST_FILES = $(shell find $(TEST_DIR) -name '*.c' 2>/dev/null)
TEST_BINS = $(patsubst $(TEST_DIR)/%.c,$(BIN_DIR)/%,$(TEST_FILES))

# Main target
TARGET = $(BIN_DIR)/pseudo

.PHONY: all clean debug release test build grammar-generate grammar-test wasm

all: debug

# Internal build target
build: $(SRC_OBJ) $(GRAMMAR_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $(TARGET) $(LDFLAGS)
	@echo "Built: $(TARGET)"

# Compile source files
$(OBJ_DIR)/src/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile grammar parser (with relaxed warnings for generated code)
$(GRAMMAR_OBJ): $(GRAMMAR_DIR)/src/parser.c | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CC) -std=c11 -O2 -I$(GRAMMAR_DIR)/src -c $< -o $@

# Build test binaries (exclude cli/main.o to avoid multiple main definitions)
$(BIN_DIR)/%: $(TEST_DIR)/%.c $(filter-out $(OBJ_DIR)/src/cli/%,$(SRC_OBJ)) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "Built test: $@"

# Run all tests
test: $(TEST_BINS)
	@echo "Running all tests..."
	@for test in $(TEST_BINS); do \
		echo "Running $$test..."; \
		$$test || exit 1; \
	done
	@echo "All tests passed!"

# Create directories
$(BIN_DIR) $(OBJ_DIR):
	@mkdir -p $@

# Debug build
debug:
	@$(MAKE) MODE=debug build

# Release build
release:
	@$(MAKE) MODE=release build

# Clean
clean:
	rm -rf $(BUILD_DIR)

# Grammar targets
grammar-generate:
	cd $(GRAMMAR_DIR) && bun install && bunx tree-sitter generate

grammar-test:
	cd $(GRAMMAR_DIR) && bunx tree-sitter test

# Emscripten / WASM configuration
EMCC = emcc
WASM_FLAGS = -O2 -s WASM=1 -s MODULARIZE=1 -s EXPORT_NAME="PseudoModule"
WASM_FLAGS += -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap","UTF8ToString","stringToUTF8","lengthBytesUTF8"]'
WASM_FLAGS += -s EXPORTED_FUNCTIONS='["_malloc","_free"]'
WASM_FLAGS += -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=1 --no-entry

WASM_INCLUDES = -Iinclude -Itree-sitter-pseudo/bindings/c -Itree-sitter-pseudo/src \
                -Itree-sitter-0.25.3/lib/include \
                -Itree-sitter-0.25.3/lib/src

# Tree-sitter library source (bundled, not linked)
TS_LIB = tree-sitter-0.25.3/lib/src/lib.c

# WASM source files: all src except cli/main.c, plus bridge.c and grammar
WASM_SRC = $(filter-out $(SRC_DIR)/cli/main.c $(SRC_DIR)/wasm/bridge.c,$(SRC_FILES)) \
           $(SRC_DIR)/wasm/bridge.c \
           $(GRAMMAR_DIR)/src/parser.c \
           $(TS_LIB)

wasm:
	$(EMCC) $(WASM_FLAGS) -std=c11 -D_POSIX_C_SOURCE=200809L $(WASM_INCLUDES) $(WASM_SRC) -o web/pseudo.js
	@echo "Built: web/pseudo.js and web/pseudo.wasm"

# Help
help:
	@echo "Usage:"
	@echo "  make          - Build debug version (default)"
	@echo "  make debug    - Build debug version"
	@echo "  make release  - Build release version"
	@echo "  make test     - Build and run tests"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make wasm     - Build WebAssembly version"
	@echo ""
	@echo "Grammar targets:"
	@echo "  make grammar-generate - Generate tree-sitter parser"
	@echo "  make grammar-test     - Run tree-sitter tests"
	@echo ""
	@echo "Debug builds:   build/debug/pseudo"
	@echo "Release builds: build/release/pseudo"
	@echo "WASM output:    web/pseudo.js, web/pseudo.wasm"
