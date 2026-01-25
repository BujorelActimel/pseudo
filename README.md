# Pseudo++ - Romanian Pseudocode Interpreter

A high-performance interpreter for Romanian educational pseudocode, written in C with tree-sitter parsing.

## Requirements

### Build Dependencies
- **GCC** 13+ (with C23 support) or Clang 16+
- **tree-sitter** library (`libtree-sitter`)
- **Make**

### WASM Build (optional)
- **Emscripten** SDK (for WebAssembly builds)

### Installing Dependencies

**Arch Linux:**
```bash
sudo pacman -S gcc make tree-sitter
# For WASM: install emscripten from AUR
```

**Ubuntu/Debian:**
```bash
sudo apt install gcc make libtree-sitter-dev
# For WASM: follow https://emscripten.org/docs/getting_started/downloads.html
```

## Building

```bash
# Debug build (with AddressSanitizer)
make

# Release build (optimized)
make release

# WASM build (requires Emscripten)
make wasm

# Run tests
make test

# Run integration tests
bash ./scripts/run_integration_tests.sh

# Clean build artifacts
make clean
```

## Usage

### Command Line
```bash
./build/release/pseudo program.pseudo
```

### Web Editor
```bash
# Build WASM first
make wasm

# Serve web directory
python -m http.server --directory web

# Open in browser
# http://localhost:8000/editor.html
```

## Language Features

### Variables and Assignment
```
x <- 10
nume <- "Ana"
```

### Arithmetic Operations
```
suma <- a + b
diferenta <- a - b
produs <- a * b
cat <- a / b
rest <- a % b
```

### Mathematical Functions
```
rad <- √16          # Square root
intreg <- [3.14]    # Floor function
```

### Input/Output
```
scrie "Mesaj"       # Output
scrie a, b, c       # Multiple values
citeste x           # Read single value
citeste a, b        # Read multiple values
```

### Conditionals
```
daca conditie atunci
    # statements
altfel
    # statements
sf
```

### Loops
```
# For loop
pentru i <- 1, 10 executa
    scrie i
sf

# For loop with step
pentru i <- 10, 1, -1 executa
    scrie i
sf

# While loop
cat timp conditie executa
    # statements
sf

# Repeat-until loop
repeta
    # statements
pana cand conditie
```

### Logical Operators
```
daca a > 0 si b > 0 atunci    # AND
daca a > 0 sau b > 0 atunci   # OR
daca not conditie atunci      # NOT
```

### Swap Operation
```
a <-> b    # Swap values of a and b
```
