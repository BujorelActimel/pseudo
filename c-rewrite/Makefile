CC = gcc
CFLAGS = -std=c23 -Wall -Wextra -Wpedantic -Iinclude
LDFLAGS = -lm -ltree-sitter

# Debug flags
DEBUG_FLAGS = -g -O0 -fsanitize=address -fsanitize=undefined
# Release flags
RELEASE_FLAGS = -O3 -march=native

# Default to debug
CFLAGS += $(DEBUG_FLAGS)

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = $(BUILD_DIR)/bin
OBJ_DIR = $(BUILD_DIR)/obj

# Source files (will add as we implement)
LINTER_SRC = $(wildcard $(SRC_DIR)/linter/*.c)
PARSER_SRC = $(wildcard $(SRC_DIR)/parser/*.c)
RUNTIME_SRC = $(wildcard $(SRC_DIR)/runtime/*.c)
COMMON_SRC = $(wildcard $(SRC_DIR)/common/*.c)
CLI_SRC = $(wildcard $(SRC_DIR)/cli/*.c)

ALL_SRC = $(LINTER_SRC) $(PARSER_SRC) $(RUNTIME_SRC) $(COMMON_SRC) $(CLI_SRC)
ALL_OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(ALL_SRC))

# Targets
TARGET = $(BIN_DIR)/pseudo

.PHONY: all clean debug release test wasm

all:
	@if [ -n "$(ALL_SRC)" ]; then \
		$(MAKE) $(TARGET); \
	else \
		echo "No source files found yet. Start implementing TASK-003!"; \
	fi

# Link
$(TARGET): $(ALL_OBJ) | $(BIN_DIR)
	@if [ -n "$(ALL_OBJ)" ]; then \
		$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS); \
		echo "Built: $@"; \
	fi

# Compile
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Create directories
$(BIN_DIR) $(OBJ_DIR):
	@mkdir -p $@

# Debug build (default)
debug:
	@$(MAKE) CFLAGS="$(CFLAGS) $(DEBUG_FLAGS)"

# Release build
release:
	@$(MAKE) CFLAGS="$(CFLAGS) $(RELEASE_FLAGS)"

# Clean
clean:
	rm -rf $(BUILD_DIR)

# Run tests
test: $(TARGET)
	@echo "Running integration tests..."
	@# Will add test runner later

# WASM build (when needed)
wasm:
	emcc $(CFLAGS) $(ALL_SRC) -o build/pseudo.js \
		-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap'] \
		-sALLOW_MEMORY_GROWTH=1 \
		$(LDFLAGS)

# Help
help:
	@echo "Usage:"
	@echo "  make          - Build debug version"
	@echo "  make release  - Build optimized version"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make test     - Run tests"
	@echo "  make wasm     - Build for web"
