CC = gcc
CFLAGS = -std=c23 -Wall -Wextra -Wpedantic -Iinclude
LDFLAGS = -lm -ltree-sitter

DEBUG_FLAGS = -g -O0 -fsanitize=address -fsanitize=undefined
RELEASE_FLAGS = -O3 -march=native

SRC_DIR = src
TEST_DIR = test
BUILD_DIR = build

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

# Source files: all .c files under src/
SRC_FILES = $(shell find $(SRC_DIR) -name '*.c')
SRC_OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/src/%.o,$(SRC_FILES))

# Test files: all .c files under test/
TEST_FILES = $(shell find $(TEST_DIR) -name '*.c' 2>/dev/null)
TEST_BINS = $(patsubst $(TEST_DIR)/%.c,$(BIN_DIR)/%,$(TEST_FILES))

# Main target
TARGET = $(BIN_DIR)/pseudo

.PHONY: all clean debug release test build

all: debug

# Internal build target
build: $(SRC_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $(TARGET) $(LDFLAGS)
	@echo "Built: $(TARGET)"

# Compile source files
$(OBJ_DIR)/src/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

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

# Help
help:
	@echo "Usage:"
	@echo "  make          - Build debug version (default)"
	@echo "  make debug    - Build debug version"
	@echo "  make release  - Build release version"
	@echo "  make test     - Build and run tests"
	@echo "  make clean    - Remove build artifacts"
	@echo ""
	@echo "Debug builds:   build/debug/pseudo"
	@echo "Release builds: build/release/pseudo"
