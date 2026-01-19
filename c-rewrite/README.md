# Romanian Pseudocode Interpreter (C Rewrite)

A high-performance interpreter for Romanian educational pseudocode, written in C with tree-sitter.

## Features

- **Fast Interpreter**: Native C23 implementation
- **Step-through Debugger**: Breakpoints, variable inspection
- **Code Transpiler**: Convert to C/C++/Pascal
- **Web Editor**: Standalone browser-based IDE
- **WASM Support**: Run in browser (no server)
- **Validated**: Passes all 62 BAC exam tests (2020-2025)

## Building

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build -j$(nproc)

# Test
cd build && ctest --output-on-failure
```

## Usage

```bash
# Run a program
./build/src/cli/pseudo program.pseudo

# Debug mode
./build/src/cli/pseudo --debug program.pseudo

# Transpile to C
./build/src/cli/pseudo --transpile c program.pseudo
```

## Status

See [TODO.md](../TODO.md) for implementation roadmap.

Currently implementing Phase 1: Core Interpreter.
