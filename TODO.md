# C Rewrite Implementation Roadmap

> **Project**: Romanian Pseudocode Interpreter in C with Tree-sitter
> **Location**: `c-rewrite/`
> **Goal**: Native + WASM interpreter with debugger and transpiler

---

## 🎯 Milestones

- [ ] **M1**: Core Interpreter (Parser + Runtime + Linter)
- [ ] **M2**: Debugger + DAP
- [ ] **M3**: Transpiler (C/C++/Pascal)
- [ ] **M4**: WASM Build
- [ ] **M5**: Web Editor with Debug UI
- [ ] **M6**: Code Equivalence UI

---

## Phase 1: Foundation (Core Interpreter + Transpiler)

### 🏗️ Setup & Infrastructure

#### **TASK-001**: Project Structure Setup
**Priority**: High | **Effort**: 2h | **Milestone**: M1

**Description**: Create initial project structure in `c-rewrite/`

**Tasks**:
- [ ] Create directory structure:
  ```
  c-rewrite/
  ├── CMakeLists.txt
  ├── include/pseudo/
  ├── src/{linter,parser,runtime,debugger,transpiler,dap,cli,wasm,common}/
  ├── test/
  ├── integration_tests/
  ├── web/
  └── cmake/
  ```
- [ ] Create root `CMakeLists.txt` with modular subdirectories
- [ ] Setup `.gitignore` (build/, *.o, *.a, *.wasm, etc.)
- [ ] Create `README.md` with build instructions
- [ ] Symlink `integration_tests/bac` → `../testing/bac`

**Acceptance Criteria**:
- CMake configures successfully
- All directories exist
- Git ignores build artifacts

**Dependencies**: None

---

#### **TASK-002**: CMake Build System
**Priority**: High | **Effort**: 4h | **Milestone**: M1

**Description**: Setup modular CMake build system

**Tasks**:
- [ ] Create `cmake/CompilerWarnings.cmake` (enable -Wall, -Wextra, etc.)
- [ ] Create `cmake/Sanitizers.cmake` (AddressSanitizer, UBSan for debug)
- [ ] Create `cmake/TreeSitter.cmake` (find/build tree-sitter)
- [ ] Create `cmake/Emscripten.cmake` (WASM build config)
- [ ] Add options: `BUILD_TESTING`, `BUILD_WASM`, `BUILD_SHARED_LIBS`, `ENABLE_DAP`
- [ ] Setup debug/release configurations

**Acceptance Criteria**:
- `cmake -B build` succeeds
- `cmake -B build -DBUILD_WASM=ON` configures for WASM
- Compiler warnings enabled in debug mode

**Dependencies**: TASK-001

---

### 🔤 Linter Module

#### **TASK-003**: Linter Implementation
**Priority**: High | **Effort**: 6h | **Milestone**: M1

**Description**: Implement text preprocessing/linting library

**Reference**: `interpreter/linter.py`

**Tasks**:
- [x] Create `include/pseudo/linter.h` with API
- [x] Implement `src/linter/linter.c`:
  - [x] Character replacement map (≤→<=, ←→<-, etc.)
  - [x] Box character removal (┌,└,│)
  - [x] Keyword normalization (citește→citeste, dacă→daca, etc.)
  - [x] Line-by-line processing
- [x] Create `src/linter/char_map.c` with replacement tables

**API Functions**:
```c
linter_t *linter_create(const char *tab_replacement);
void linter_destroy(linter_t *linter);
char *linter_process_text(linter_t *linter, const char *text);
char *linter_process_file(linter_t *linter, const char *path);
```

**Acceptance Criteria**:
- All character replacements from Python version work
- Box characters removed correctly
- Keywords normalized
- Library compiles and links

**Dependencies**: TASK-002

---

#### **TASK-004**: Linter Unit Tests
**Priority**: Medium | **Effort**: 3h | **Milestone**: M1

**Tasks**:
- [x] Create `test/test_linter.c`
- [x] Test Unicode conversions (≤, ≠, ≥, ←, →, √)
- [x] Test box character removal
- [x] Test Romanian diacritic normalization
- [x] Test edge cases (empty input, NULL, etc.)

**Acceptance Criteria**:
- All tests pass
- `ctest` runs successfully

**Dependencies**: TASK-003

---

#### **TASK-005**: Standalone Linter CLI
**Priority**: Low | **Effort**: 2h | **Milestone**: M1

**Tasks**:
- [x] Create `src/linter/linter_cli.c`
- [x] Argument parsing (input file, output file, stdin mode)
- [x] Add to CMake build

**Usage**: `pseudo-lint input.pseudo -o cleaned.pseudo`

**Acceptance Criteria**:
- Can lint files from command line
- Works like Python linter

**Dependencies**: TASK-003

---

### 🌳 Tree-sitter Grammar

#### **TASK-006**: Tree-sitter Grammar Repository
**Priority**: High | **Effort**: 8h | **Milestone**: M1

**Description**: Create separate tree-sitter grammar repo

**Reference**: `interpreter/Pseudocode.g4`

**Tasks**:
- [ ] Create new repo `tree-sitter-pseudocode-ro`
- [ ] Setup `package.json`, `binding.gyp`
- [ ] Write `grammar.js` converting from ANTLR:
  - [ ] Statements: assign, swap, read, write, multi-stmt
  - [ ] Control flow: if/then/else, for, while, do-while, repeat-until
  - [ ] Expressions with precedence (*, /, %, +, -, comparisons, SI/SAU/NOT)
  - [ ] Special operators: floor `[expr]`, sqrt `√`, swap `<->`, assign `<-`
  - [ ] Literals: numbers, strings, identifiers
  - [ ] Comments: `#`
- [ ] Write syntax highlighting queries (`queries/highlights.scm`)
- [ ] Create grammar tests in `test/corpus/`
- [ ] Generate parser with `tree-sitter generate`
- [ ] Test with `tree-sitter test`

**Acceptance Criteria**:
- Grammar generates successfully
- All test cases pass
- Parser handles all language constructs
- No conflicts in grammar

**Dependencies**: TASK-002

---

#### **TASK-007**: Integrate Grammar as Submodule
**Priority**: High | **Effort**: 1h | **Milestone**: M1

**Tasks**:
- [ ] Add grammar repo as git submodule: `git submodule add <url> c-rewrite/grammar`
- [ ] Update CMake to build grammar parser
- [ ] Link `tree-sitter` library

**Acceptance Criteria**:
- Submodule added correctly
- Parser builds with main project

**Dependencies**: TASK-006

---

### 🔧 Parser Module

#### **TASK-008**: AST Data Structures
**Priority**: High | **Effort**: 4h | **Milestone**: M1

**Description**: Define AST node types and structures

**Tasks**:
- [ ] Create `include/pseudo/ast.h`
- [ ] Define `ast_node_type_t` enum (ASSIGN, SWAP, READ, WRITE, IF, FOR, etc.)
- [ ] Define `source_location_t` struct (line, column, offset)
- [ ] Define `ast_node_t` struct with union for node types:
  - [ ] Assign: name, value
  - [ ] Swap: left, right
  - [ ] Read: name list
  - [ ] Write: expression list
  - [ ] If: condition, then_block, else_block
  - [ ] For: var, start, end, step, body
  - [ ] While: condition, body
  - [ ] Do-while: body, condition
  - [ ] Repeat-until: body, condition
  - [ ] Binary expressions: op, left, right
  - [ ] Unary expressions: op, operand
  - [ ] Atoms: int, float, string, identifier
- [ ] Add AST helper functions (create, destroy, print)

**Acceptance Criteria**:
- All node types defined
- Memory management functions work
- Can represent any pseudocode program

**Dependencies**: TASK-002

---

#### **TASK-009**: Parser Implementation
**Priority**: High | **Effort**: 8h | **Milestone**: M1

**Description**: Wrap tree-sitter parser and build AST

**Tasks**:
- [ ] Create `include/pseudo/parser.h`
- [ ] Implement `src/parser/parser.c`:
  - [ ] Initialize tree-sitter parser
  - [ ] Parse source code to CST (concrete syntax tree)
  - [ ] Walk CST and build simplified AST
  - [ ] Track source locations for debugging
  - [ ] Handle parse errors
- [ ] Implement `src/parser/source_map.c` for location tracking

**API Functions**:
```c
parser_t *parser_create(void);
void parser_destroy(parser_t *parser);
ast_node_t *parser_parse(parser_t *parser, const char *source);
const char *parser_get_error(parser_t *parser);
```

**Acceptance Criteria**:
- Can parse all valid pseudocode programs
- AST correctly represents program structure
- Source locations preserved
- Parse errors reported with context

**Dependencies**: TASK-007, TASK-008

---

#### **TASK-010**: Parser Unit Tests
**Priority**: High | **Effort**: 4h | **Milestone**: M1

**Tasks**:
- [ ] Create `test/test_parser.c`
- [ ] Test parsing statements (assign, swap, read, write)
- [ ] Test control flow (if, for, while, do-while, repeat)
- [ ] Test expressions (arithmetic, comparison, logical)
- [ ] Test special operators (floor, sqrt)
- [ ] Test error handling (syntax errors)
- [ ] Verify AST structure for known programs

**Acceptance Criteria**:
- All tests pass
- Parser handles edge cases

**Dependencies**: TASK-009

---

### 🚀 Runtime Module

#### **TASK-011**: Value System
**Priority**: High | **Effort**: 4h | **Milestone**: M1

**Description**: Dynamic value type system (int/float/string)

**Reference**: `interpreter/Interpreter.py` (lines 11-98)

**Tasks**:
- [ ] Create `include/pseudo/value.h`
- [ ] Define `value_type_t` enum (INT, FLOAT, STRING)
- [ ] Define `value_t` struct with tagged union
- [ ] Implement `src/runtime/value.c`:
  - [ ] Create/destroy functions
  - [ ] Type conversions (automatic int↔float, to string, to bool)
  - [ ] Arithmetic operations (+, -, *, /, %, sqrt, floor)
  - [ ] Comparison operations (=, !=, <, <=, >, >=)
  - [ ] Logical operations (SI/AND, SAU/OR, NOT)
  - [ ] String concatenation

**Acceptance Criteria**:
- All value operations work correctly
- Type conversions match Python behavior
- Memory managed properly (no leaks)

**Dependencies**: TASK-002

---

#### **TASK-012**: Environment (Variable Storage)
**Priority**: High | **Effort**: 3h | **Milestone**: M1

**Description**: Hash map for variable storage

**Tasks**:
- [ ] Create `src/common/hashmap.c` (generic hash map)
- [ ] Create `include/pseudo/environment.h`
- [ ] Implement `src/runtime/environment.c`:
  - [ ] Variable storage (string → value_t*)
  - [ ] Default to 0 for undefined variables (Python behavior)
  - [ ] Get/set/has operations

**API Functions**:
```c
environment_t *env_create(void);
void env_destroy(environment_t *env);
void env_set(environment_t *env, const char *name, value_t *value);
value_t *env_get(environment_t *env, const char *name);
```

**Acceptance Criteria**:
- Variables stored and retrieved correctly
- Undefined variables default to 0
- No memory leaks

**Dependencies**: TASK-011

---

#### **TASK-013**: Interpreter Core
**Priority**: High | **Effort**: 12h | **Milestone**: M1

**Description**: AST visitor pattern interpreter

**Reference**: `interpreter/Interpreter.py`

**Tasks**:
- [ ] Create `include/pseudo/runtime.h`
- [ ] Implement `src/runtime/interpreter.c`:
  - [ ] Visit functions for each AST node type
  - [ ] Execute statements in order
  - [ ] Evaluate expressions recursively
  - [ ] Handle control flow (if/for/while/do-while/repeat)
  - [ ] Short-circuit evaluation for SI/SAU
  - [ ] I/O operations (redirectable for testing)
- [ ] Implement `src/runtime/io.c`:
  - [ ] Input handling (with callbacks for WASM)
  - [ ] Output buffering
- [ ] Implement `src/runtime/builtins.c`:
  - [ ] sqrt, floor, modulo

**Special Behaviors** (from Python interpreter):
- Variables default to 0 if undefined
- `scrie` concatenates without spaces
- `citeste` auto-converts to int/float
- Floor `[x]` truncates to int
- Sqrt raises error on negative

**Acceptance Criteria**:
- Can execute all 62 BAC test programs
- Matches Python interpreter output exactly
- I/O is redirectable

**Dependencies**: TASK-009, TASK-011, TASK-012

---

#### **TASK-014**: Runtime Unit Tests
**Priority**: High | **Effort**: 6h | **Milestone**: M1

**Tasks**:
- [ ] Create `test/test_runtime.c`
- [ ] Test value operations
- [ ] Test variable storage
- [ ] Test arithmetic expressions
- [ ] Test control flow
- [ ] Test I/O operations
- [ ] Test special operators (floor, sqrt)
- [ ] Test error conditions

**Acceptance Criteria**:
- All unit tests pass
- Edge cases handled

**Dependencies**: TASK-013

---

### 🧪 Integration Testing

#### **TASK-015**: Integration Test Runner
**Priority**: High | **Effort**: 6h | **Milestone**: M1

**Description**: Test runner for 62 BAC exam tests

**Reference**: `scripts/test.bash`

**Tasks**:
- [ ] Create `integration_tests/test_runner.c`
- [ ] Discover test directories (`testing/bac/2020-2025/`)
- [ ] For each test:
  - [ ] Read source, input, expected output
  - [ ] Lint source
  - [ ] Parse to AST
  - [ ] Execute with input
  - [ ] Compare output
  - [ ] Report pass/fail with diff
- [ ] Integrate with CTest
- [ ] Add colored output (green/red)

**Acceptance Criteria**:
- All 62 tests pass
- Output matches Python interpreter exactly
- Can run individual tests or full suite

**Dependencies**: TASK-013

---

#### **TASK-016**: Fix Failing Tests
**Priority**: High | **Effort**: Variable | **Milestone**: M1

**Description**: Debug and fix any failing BAC tests

**Tasks**:
- [ ] Run integration tests
- [ ] For each failure:
  - [ ] Analyze diff
  - [ ] Identify root cause
  - [ ] Fix interpreter/parser
  - [ ] Verify fix doesn't break other tests
- [ ] Document any edge cases found

**Acceptance Criteria**:
- **100% of 62 BAC tests pass**
- Output matches Python interpreter

**Dependencies**: TASK-015

---

### 🔄 Transpiler Module

#### **TASK-017**: Transpiler Core
**Priority**: High | **Effort**: 6h | **Milestone**: M3

**Description**: Core transpiler with code generation

**Tasks**:
- [ ] Create `include/pseudo/transpiler.h`
- [ ] Define `target_language_t` enum (C, CPP, PASCAL)
- [ ] Implement `src/transpiler/transpiler.c`:
  - [ ] AST visitor for code generation
  - [ ] Indentation management
  - [ ] Output buffer management
  - [ ] Language-agnostic helpers

**API Functions**:
```c
transpiler_t *transpiler_create(void);
void transpiler_destroy(transpiler_t *trans);
char *transpiler_convert_full(transpiler_t *trans, ast_node_t *ast, target_language_t lang);
char *transpiler_convert_node(transpiler_t *trans, ast_node_t *node, target_language_t lang);
```

**Acceptance Criteria**:
- Core transpiler infrastructure works
- Can generate code for any AST node

**Dependencies**: TASK-009

---

#### **TASK-018**: C Code Generator
**Priority**: High | **Effort**: 8h | **Milestone**: M3

**Description**: Generate C code from pseudocode AST

**Tasks**:
- [ ] Implement `src/transpiler/codegen_c.c`:
  - [ ] Program structure (includes, main function)
  - [ ] Variable declarations (infer types from usage)
  - [ ] Statements (assign, swap, if, loops)
  - [ ] Expressions (arithmetic, comparison, logical)
  - [ ] I/O (`scanf`, `printf`)
  - [ ] Special operators (sqrt → `sqrt()`, floor → cast)

**Example Output**:
```c
#include <stdio.h>
#include <math.h>

int main() {
    int n;
    scanf("%d", &n);

    int suma = 0;
    for (int i = 1; i <= n; i++) {
        suma = suma + i;
    }

    printf("%d", suma);
    return 0;
}
```

**Acceptance Criteria**:
- Generated C code compiles with `gcc`
- Output matches pseudocode interpreter
- Code is readable and idiomatic

**Dependencies**: TASK-017

---

#### **TASK-019**: C++ Code Generator
**Priority**: Medium | **Effort**: 6h | **Milestone**: M3

**Description**: Generate C++ code from pseudocode AST

**Tasks**:
- [ ] Implement `src/transpiler/codegen_cpp.c`:
  - [ ] Program structure (includes, main)
  - [ ] Use `std::cin` / `std::cout`
  - [ ] Use `std::string` for strings
  - [ ] Modern C++ style (auto, range-based for where applicable)

**Acceptance Criteria**:
- Generated C++ code compiles with `g++`
- Output matches interpreter
- Uses modern C++ idioms

**Dependencies**: TASK-017

---

#### **TASK-020**: Pascal Code Generator
**Priority**: Medium | **Effort**: 6h | **Milestone**: M3

**Description**: Generate Pascal code from pseudocode AST

**Tasks**:
- [ ] Implement `src/transpiler/codegen_pascal.c`:
  - [ ] Program structure (program name, var declarations, begin/end)
  - [ ] Use `ReadLn`, `Write`, `WriteLn`
  - [ ] Pascal-style loops (for-to-do, while-do, repeat-until)
  - [ ] Case-insensitive keywords

**Example Output**:
```pascal
program SumaProgram;
var
    n, suma, i: Integer;
begin
    ReadLn(n);
    suma := 0;
    for i := 1 to n do
        suma := suma + i;
    Write(suma);
end.
```

**Acceptance Criteria**:
- Generated Pascal code compiles with Free Pascal
- Output matches interpreter

**Dependencies**: TASK-017

---

#### **TASK-021**: Loop Transformation Engine
**Priority**: High | **Effort**: 8h | **Milestone**: M3

**Description**: Transform loops between different types

**Tasks**:
- [ ] Implement `src/transpiler/loop_transform.c`:
  - [ ] Analyze loop structure (init, condition, increment, body)
  - [ ] Extract loop components
  - [ ] Transform to for/while/do-while/repeat-until
  - [ ] Preserve semantics
- [ ] Handle edge cases (empty body, complex conditions)

**API Functions**:
```c
char *transpiler_convert_loop_as(transpiler_t *trans, ast_node_t *loop_node,
                                  loop_type_t target_loop, target_language_t lang);
```

**Transformations**:
- `pentru` → `cat timp` (add init before, increment in body)
- `pentru` → `repeta...pana cand` (negate condition)
- `cat timp` → `pentru` (extract init/increment if pattern matches)
- All combinations

**Acceptance Criteria**:
- All loop transformations work
- Semantics preserved (same output)
- Edge cases handled

**Dependencies**: TASK-017

---

#### **TASK-022**: Transpiler Unit Tests
**Priority**: High | **Effort**: 6h | **Milestone**: M3

**Tasks**:
- [ ] Create `test/test_transpiler.c`
- [ ] Test transpilation to C/C++/Pascal
- [ ] Test loop transformations
- [ ] Verify generated code compiles
- [ ] Compare output of transpiled code with interpreter

**Test Approach**:
1. Take subset of BAC tests
2. Transpile to C/C++/Pascal
3. Compile transpiled code
4. Run with same input
5. Verify output matches

**Acceptance Criteria**:
- Transpiled code compiles
- Output matches interpreter for all tested programs

**Dependencies**: TASK-018, TASK-019, TASK-020, TASK-021

---

#### **TASK-023**: Standalone Transpiler CLI
**Priority**: Low | **Effort**: 3h | **Milestone**: M3

**Tasks**:
- [ ] Create `src/cli/transpiler_cli.c`
- [ ] Arguments: `--transpile <c|cpp|pascal>`, `--show-loop-as <type>`
- [ ] Output to stdout or file

**Usage**:
```bash
pseudo-transpile program.pseudo --to c
pseudo-transpile program.pseudo --to pascal -o program.pas
pseudo-transpile program.pseudo --show-loop-as while
```

**Acceptance Criteria**:
- Can transpile from command line
- Loop transformations work

**Dependencies**: TASK-017

---

### 🖥️ Native CLI Tool

#### **TASK-024**: Command-Line Interface
**Priority**: High | **Effort**: 6h | **Milestone**: M1

**Description**: Main CLI tool for running programs

**Tasks**:
- [ ] Create `src/cli/main.c`
- [ ] Argument parsing:
  - [ ] `--debug` / `-d`: Enable debug mode
  - [ ] `--quiet` / `-q`: Suppress input prompts
  - [ ] `--lint`: Lint mode only
  - [ ] `--transpile <lang>`: Transpile mode
  - [ ] `--dap`: Run as DAP server (Phase 2)
- [ ] Mode selection (run, debug, lint, transpile, dap)
- [ ] Error handling and reporting
- [ ] Exit codes

**Usage**:
```bash
pseudo program.pseudo              # Run
pseudo --debug program.pseudo      # Interactive debug
pseudo --quiet program.pseudo      # No input prompts (testing)
pseudo --lint program.pseudo       # Lint only
pseudo --transpile c program.pseudo # Transpile
```

**Acceptance Criteria**:
- CLI works for all modes
- Error messages are clear
- Compatible with existing test scripts

**Dependencies**: TASK-013, TASK-003, TASK-017

---

## Phase 2: Debugger Implementation

### 🐛 Debugger Core

#### **TASK-025**: Debugger Data Structures
**Priority**: High | **Effort**: 4h | **Milestone**: M2

**Description**: Debugger state and data structures

**Tasks**:
- [ ] Create `include/pseudo/debugger.h`
- [ ] Define `debugger_state_t` enum (RUNNING, PAUSED, STOPPED, STEPPING)
- [ ] Define `breakpoint_t` struct (id, line, column, condition)
- [ ] Define `debugger_t` struct:
  - [ ] State
  - [ ] Breakpoint list
  - [ ] Current execution location
  - [ ] Step control (over/into/out)
  - [ ] Runtime reference
  - [ ] Event callbacks

**Acceptance Criteria**:
- All data structures defined
- API documented

**Dependencies**: TASK-013

---

#### **TASK-026**: Breakpoint Management
**Priority**: High | **Effort**: 4h | **Milestone**: M2

**Description**: Add/remove/check breakpoints

**Tasks**:
- [ ] Implement `src/debugger/breakpoints.c`:
  - [ ] Add breakpoint (line-based)
  - [ ] Remove breakpoint (by id)
  - [ ] Check if should break at line
  - [ ] Conditional breakpoints (evaluate expression)
  - [ ] List all breakpoints

**API Functions**:
```c
uint32_t debugger_add_breakpoint(debugger_t *dbg, uint32_t line, const char *condition);
void debugger_remove_breakpoint(debugger_t *dbg, uint32_t id);
bool debugger_should_break(debugger_t *dbg, uint32_t line);
```

**Acceptance Criteria**:
- Can add/remove breakpoints
- Conditional breakpoints work
- No duplicate breakpoints

**Dependencies**: TASK-025

---

#### **TASK-027**: Debugger Core Implementation
**Priority**: High | **Effort**: 8h | **Milestone**: M2

**Description**: Step control and execution management

**Tasks**:
- [ ] Implement `src/debugger/debugger.c`:
  - [ ] Continue execution
  - [ ] Step over (next statement)
  - [ ] Step into (enter loops/functions)
  - [ ] Step out (exit current scope)
  - [ ] Pause execution
  - [ ] Variable inspection
  - [ ] Expression evaluation

**Integration with Runtime**:
- [ ] Add hook: `runtime_set_debugger(runtime_t *rt, debugger_t *dbg)`
- [ ] Before each statement: call `debugger_pre_statement_hook(dbg, node)`
- [ ] Hook checks breakpoints, handles stepping

**Acceptance Criteria**:
- All step commands work
- Breakpoints pause execution
- Can inspect variables at any point

**Dependencies**: TASK-026

---

#### **TASK-028**: Watch Expressions
**Priority**: Medium | **Effort**: 4h | **Milestone**: M2

**Description**: Evaluate expressions in debug context

**Tasks**:
- [ ] Implement `src/debugger/watches.c`:
  - [ ] Parse expression string
  - [ ] Evaluate against current environment
  - [ ] Return value or error
  - [ ] List active watches

**Acceptance Criteria**:
- Can evaluate any valid expression
- Works with current variable values

**Dependencies**: TASK-027

---

#### **TASK-029**: Debugger Unit Tests
**Priority**: High | **Effort**: 4h | **Milestone**: M2

**Tasks**:
- [ ] Create `test/test_debugger.c`
- [ ] Test breakpoint management
- [ ] Test step over/into/out
- [ ] Test variable inspection
- [ ] Test expression evaluation
- [ ] Test edge cases

**Acceptance Criteria**:
- All debugger features tested
- Tests pass

**Dependencies**: TASK-027

---

### 🔌 Debug Adapter Protocol (DAP)

#### **TASK-030**: JSON Parser/Serializer
**Priority**: High | **Effort**: 4h | **Milestone**: M2

**Description**: Minimal JSON handling for DAP

**Tasks**:
- [ ] Option 1: Use `cJSON` library (bundled)
- [ ] Option 2: Minimal custom parser
- [ ] Implement `src/dap/json_parser.c`:
  - [ ] Parse DAP request messages
  - [ ] Serialize DAP response/event messages
  - [ ] Handle JSON-RPC format

**Acceptance Criteria**:
- Can parse DAP requests
- Can serialize responses
- Handles errors gracefully

**Dependencies**: TASK-002

---

#### **TASK-031**: DAP Protocol Implementation
**Priority**: High | **Effort**: 12h | **Milestone**: M2

**Description**: Implement DAP server (stdio only)

**Reference**: [Debug Adapter Protocol Specification](https://microsoft.github.io/debug-adapter-protocol/)

**Tasks**:
- [ ] Create `include/pseudo/dap.h`
- [ ] Implement `src/dap/dap_server.c`:
  - [ ] Message loop (read from stdin, write to stdout)
  - [ ] Request dispatch
  - [ ] Event emission
- [ ] Implement `src/dap/dap_protocol.c`:
  - [ ] Request handlers:
    - [ ] `initialize`: Capability negotiation
    - [ ] `launch`: Start program
    - [ ] `setBreakpoints`: Set/update breakpoints
    - [ ] `continue`: Resume execution
    - [ ] `next`: Step over
    - [ ] `stepIn`: Step into
    - [ ] `stepOut`: Step out
    - [ ] `pause`: Pause execution
    - [ ] `scopes`: Get variable scopes
    - [ ] `variables`: Get variables in scope
    - [ ] `evaluate`: Evaluate expression
    - [ ] `stackTrace`: Get call stack
  - [ ] Event emitters:
    - [ ] `stopped`: Execution paused
    - [ ] `continued`: Execution resumed
    - [ ] `terminated`: Program finished
    - [ ] `output`: Program output

**Acceptance Criteria**:
- DAP server runs over stdio
- Handles all required requests
- Emits events correctly
- Works with VS Code debugger extension

**Dependencies**: TASK-027, TASK-030

---

#### **TASK-032**: DAP Integration Testing
**Priority**: Medium | **Effort**: 4h | **Milestone**: M2

**Description**: Test DAP server with VS Code

**Tasks**:
- [ ] Create VS Code launch configuration
- [ ] Test basic debugging workflow:
  - [ ] Start debugging
  - [ ] Set breakpoints
  - [ ] Step through code
  - [ ] Inspect variables
  - [ ] Continue/pause
  - [ ] Evaluate expressions
- [ ] Document setup process

**Acceptance Criteria**:
- VS Code can debug pseudocode programs
- All debugger features work through VS Code

**Dependencies**: TASK-031

---

#### **TASK-033**: Update CLI with DAP Mode
**Priority**: High | **Effort**: 2h | **Milestone**: M2

**Tasks**:
- [ ] Add `--dap` flag to CLI
- [ ] Route to DAP server when flag present
- [ ] Update documentation

**Usage**: `pseudo --dap program.pseudo` (VS Code launches this)

**Acceptance Criteria**:
- CLI can run as DAP server
- VS Code integration works

**Dependencies**: TASK-031

---

## Phase 3: WASM Compilation

### 🌐 WASM Build

#### **TASK-034**: Emscripten Setup
**Priority**: High | **Effort**: 4h | **Milestone**: M4

**Description**: Configure WASM build with Emscripten

**Tasks**:
- [ ] Install Emscripten SDK
- [ ] Update `cmake/Emscripten.cmake`:
  - [ ] Set Emscripten flags
  - [ ] Configure exports
  - [ ] Set memory/environment options
- [ ] Create `scripts/build_wasm.sh`
- [ ] Test WASM build

**Acceptance Criteria**:
- CMake configures for WASM
- `./scripts/build_wasm.sh` builds WASM artifacts
- Output: `pseudo.js`, `pseudo.wasm`

**Dependencies**: TASK-002

---

#### **TASK-035**: WASM Bindings
**Priority**: High | **Effort**: 8h | **Milestone**: M4

**Description**: Expose interpreter API to JavaScript

**Tasks**:
- [ ] Create `src/wasm/wasm_bindings.c`
- [ ] Implement exported functions:
  - [ ] Linter: `pseudo_lint_text()`
  - [ ] Parser: `pseudo_parse()`
  - [ ] Runtime execution: `pseudo_step()`, `pseudo_continue()`, `pseudo_run_to_end()`, `pseudo_reset()`
  - [ ] Runtime I/O: `pseudo_set_input()`, `pseudo_get_output()`, `pseudo_clear_output()`
  - [ ] Runtime state: `pseudo_is_running()`, `pseudo_is_finished()`, `pseudo_get_current_line()`
  - [ ] Debugger breakpoints: `pseudo_add_breakpoint()`, `pseudo_remove_breakpoint()`, etc.
  - [ ] Debugger variables: `pseudo_get_variable_json()`, `pseudo_get_all_variables_json()`
  - [ ] Transpiler: `pseudo_transpile_to_c()`, `pseudo_transpile_to_cpp()`, `pseudo_transpile_to_pascal()`
  - [ ] Transpiler selection: `pseudo_transpile_range_to_*()`, `pseudo_transform_loop_json()`
- [ ] Add `EMSCRIPTEN_KEEPALIVE` macros
- [ ] Configure Emscripten exports

**Acceptance Criteria**:
- All functions exported
- Can call from JavaScript
- Memory management correct (no leaks)

**Dependencies**: TASK-034

---

#### **TASK-036**: WASM I/O Redirection
**Priority**: High | **Effort**: 4h | **Milestone**: M4

**Description**: Redirect I/O for browser environment

**Tasks**:
- [ ] Implement `src/wasm/wasm_io.c`:
  - [ ] Input queue (set from JavaScript)
  - [ ] Output buffer (accumulated, retrieved from JavaScript)
  - [ ] Hook into runtime I/O

**Acceptance Criteria**:
- `citeste` reads from JavaScript-provided input
- `scrie` accumulates in buffer
- Can retrieve output via `pseudo_get_output()`

**Dependencies**: TASK-035

---

#### **TASK-037**: WASM JavaScript Wrapper
**Priority**: High | **Effort**: 6h | **Milestone**: M4

**Description**: JavaScript API wrapper for WASM module

**Tasks**:
- [ ] Create `web/pseudo-runtime.js`:
  - [ ] Load WASM module
  - [ ] Wrap C API with clean JavaScript API
  - [ ] Handle string conversions (JS ↔ C)
  - [ ] Memory management (allocate/free)
  - [ ] Error handling

**Example API**:
```javascript
const runtime = await PseudoRuntime.init();
runtime.parse("citeste n\nscrie n");
runtime.setInput("5");
runtime.runToEnd();
const output = runtime.getOutput(); // "5"
```

**Acceptance Criteria**:
- JavaScript wrapper works
- Can run programs in browser
- Memory managed correctly

**Dependencies**: TASK-035

---

#### **TASK-038**: WASM Testing
**Priority**: High | **Effort**: 4h | **Milestone**: M4

**Description**: Test WASM build

**Tasks**:
- [ ] Create simple HTML test page
- [ ] Load WASM module
- [ ] Test basic operations:
  - [ ] Lint text
  - [ ] Parse code
  - [ ] Execute program
  - [ ] Get output
  - [ ] Set breakpoints
  - [ ] Step execution
  - [ ] Transpile code
- [ ] Test memory usage (no leaks)

**Acceptance Criteria**:
- WASM runs in browser
- All operations work
- No memory leaks
- Performance acceptable

**Dependencies**: TASK-037

---

#### **TASK-039**: Standalone Linter WASM
**Priority**: Medium | **Effort**: 3h | **Milestone**: M4

**Description**: Separate minimal WASM for linting only

**Tasks**:
- [ ] Create minimal WASM build with only linter
- [ ] Export `pseudo_lint_text()` only
- [ ] Optimize for size (should be < 50KB)

**Purpose**: Fast loading for real-time linting in editor

**Acceptance Criteria**:
- Linter WASM builds separately
- Size < 100KB
- Works independently

**Dependencies**: TASK-034

---

## Phase 4: Web Editor with Debug UI

### 🎨 Web Interface

#### **TASK-040**: Web App Structure
**Priority**: High | **Effort**: 4h | **Milestone**: M5

**Description**: Basic HTML/CSS/JS structure

**Tasks**:
- [ ] Create `web/index.html`:
  - [ ] Basic layout (editor pane, sidebar)
  - [ ] Load Monaco Editor (CDN)
  - [ ] Load WASM modules
  - [ ] No tracking scripts, no analytics
- [ ] Create `web/style.css`:
  - [ ] Clean, minimal design
  - [ ] Responsive layout
  - [ ] Dark mode support (optional)
- [ ] Create `web/editor.js` (main entry point)

**Acceptance Criteria**:
- HTML loads in browser
- Layout looks good
- No external dependencies (except Monaco CDN)

**Dependencies**: TASK-037

---

#### **TASK-041**: Monaco Editor Integration
**Priority**: High | **Effort**: 6h | **Milestone**: M5

**Description**: Setup Monaco with pseudocode language

**Tasks**:
- [ ] Register pseudocode language
- [ ] Create Monarch tokenizer for syntax highlighting:
  - [ ] Keywords (citeste, scrie, daca, pentru, etc.)
  - [ ] Operators (<-, <->, <=, >=, etc.)
  - [ ] Numbers, strings, comments
  - [ ] Special symbols (√, [])
- [ ] Configure editor options
- [ ] Add breakpoint decorations support
- [ ] Add current line highlighting
- [ ] Integrate linter for real-time error checking

**Acceptance Criteria**:
- Syntax highlighting works
- Can click gutter to set breakpoints
- Current execution line highlighted
- Real-time linting shows errors

**Dependencies**: TASK-040

---

#### **TASK-042**: Debug Controls UI
**Priority**: High | **Effort**: 6h | **Milestone**: M5

**Description**: Debug toolbar and controls

**Tasks**:
- [ ] Create debug toolbar:
  - [ ] Run button (▶)
  - [ ] Step button (↷)
  - [ ] Continue button (⏩)
  - [ ] Pause button (⏸)
  - [ ] Reset button (⏹)
- [ ] Wire buttons to WASM API:
  - [ ] Run → `pseudo_run_to_end()`
  - [ ] Step → `pseudo_step()`
  - [ ] Continue → `pseudo_continue()`
  - [ ] Reset → `pseudo_reset()`
- [ ] Handle breakpoints:
  - [ ] Click gutter → `pseudo_add_breakpoint(line)`
  - [ ] Click again → `pseudo_remove_breakpoint(line)`
- [ ] Update UI on state changes

**Acceptance Criteria**:
- All debug controls work
- Execution pauses at breakpoints
- Can step through code
- UI reflects current state

**Dependencies**: TASK-041

---

#### **TASK-043**: Variable Inspector
**Priority**: High | **Effort**: 4h | **Milestone**: M5

**Description**: Show current variable values

**Tasks**:
- [ ] Create variables panel in sidebar
- [ ] Fetch variables after each step: `pseudo_get_all_variables_json()`
- [ ] Display as list:
  ```
  x = 10 (int)
  y = 3.14 (float)
  s = "hello" (string)
  ```
- [ ] Update in real-time during execution
- [ ] Highlight changed variables

**Acceptance Criteria**:
- Variables displayed correctly
- Updates after each step
- Shows type and value

**Dependencies**: TASK-042

---

#### **TASK-044**: Output Console
**Priority**: High | **Effort**: 3h | **Milestone**: M5

**Description**: Display program output

**Tasks**:
- [ ] Create output panel in sidebar
- [ ] After each step, fetch output: `pseudo_get_output()`
- [ ] Display as text
- [ ] Auto-scroll to bottom
- [ ] Clear on reset

**Acceptance Criteria**:
- Output displayed correctly
- Updates in real-time
- `scrie` statements appear immediately

**Dependencies**: TASK-042

---

#### **TASK-045**: Input Dialog
**Priority**: High | **Effort**: 3h | **Milestone**: M5

**Description**: Handle `citeste` statements

**Tasks**:
- [ ] Detect when program needs input
- [ ] Show modal dialog with variable name prompt
- [ ] Collect input from user
- [ ] Send to WASM: `pseudo_set_input(value)`
- [ ] Continue execution

**Alternative**: Pre-load all inputs before execution

**Acceptance Criteria**:
- User can provide input
- `citeste` statements work
- Can handle multiple inputs

**Dependencies**: TASK-042

---

#### **TASK-046**: File Operations
**Priority**: Medium | **Effort**: 4h | **Milestone**: M5

**Description**: Load/save pseudocode files

**Tasks**:
- [ ] Add "Open File" button (file input)
- [ ] Add "Save File" button (download)
- [ ] Add "New File" button (clear editor)
- [ ] Use browser File API
- [ ] Remember last edited code (localStorage)

**Acceptance Criteria**:
- Can open `.pseudo` files
- Can save files
- Code persists in localStorage

**Dependencies**: TASK-040

---

#### **TASK-047**: Example Programs
**Priority**: Low | **Effort**: 2h | **Milestone**: M5

**Description**: Provide sample programs

**Tasks**:
- [ ] Add "Examples" dropdown
- [ ] Include 5-10 example programs:
  - [ ] Hello world
  - [ ] Sum 1..n
  - [ ] Factorial
  - [ ] GCD
  - [ ] Prime check
  - [ ] Array operations
- [ ] Load example into editor on click

**Acceptance Criteria**:
- Examples work
- Cover common patterns
- Well-commented

**Dependencies**: TASK-040

---

## Phase 5: Code Equivalence UI

### 🔀 Transpiler Integration

#### **TASK-048**: Transpiler UI Panel
**Priority**: High | **Effort**: 6h | **Milestone**: M6

**Description**: Side panel for code equivalence

**Tasks**:
- [ ] Create equivalence panel (hidden by default)
- [ ] Add language tabs (C, C++, Pascal)
- [ ] Add close button
- [ ] Style to match editor

**Acceptance Criteria**:
- Panel toggles on/off
- Tabs switch between languages
- Looks good

**Dependencies**: TASK-040

---

#### **TASK-049**: Selection-Based Transpilation
**Priority**: High | **Effort**: 6h | **Milestone**: M6

**Description**: Transpile selected code

**Tasks**:
- [ ] Add context menu action: "Show in C/C++/Pascal"
- [ ] On trigger:
  - [ ] Get selection (start line, end line)
  - [ ] Call WASM: `pseudo_transpile_range_to_c(start, end)`
  - [ ] Get C/C++/Pascal code
  - [ ] Display in equivalence panel
- [ ] Show all three languages at once (or tabs)

**Acceptance Criteria**:
- Can select code and see equivalents
- All three languages generated
- Code is formatted nicely

**Dependencies**: TASK-048

---

#### **TASK-050**: Loop Transformation Viewer
**Priority**: High | **Effort**: 6h | **Milestone**: M6

**Description**: Show loop variations

**Tasks**:
- [ ] Detect if cursor/selection is on a loop
- [ ] Add context menu action: "Show Loop Variations"
- [ ] Call WASM: `pseudo_transform_loop_json(line)`
- [ ] Parse JSON response:
  ```json
  {
    "for": "pentru i <- 1, 10 executa ...",
    "while": "i <- 1; cat timp i <= 10 executa ...",
    "do_while": "i <- 1; executa ... cat timp i <= 10",
    "repeat_until": "i <- 1; repeta ... pana cand i > 10"
  }
  ```
- [ ] Display in dropdown or tabs
- [ ] Show in currently selected language (C/C++/Pascal)

**Acceptance Criteria**:
- Can view loop variations
- All 4 loop types shown
- Works for all supported loops

**Dependencies**: TASK-049

---

#### **TASK-051**: Full Program Transpilation
**Priority**: Medium | **Effort**: 3h | **Milestone**: M6

**Description**: Transpile entire program

**Tasks**:
- [ ] Add button: "View Full Program in C/C++/Pascal"
- [ ] Call WASM: `pseudo_transpile_to_c()`, etc.
- [ ] Display in new tab or modal
- [ ] Add "Copy to Clipboard" button
- [ ] Add "Download" button

**Acceptance Criteria**:
- Can transpile full program
- Can copy or download
- Generated code compiles

**Dependencies**: TASK-048

---

#### **TASK-052**: Syntax Highlighting for Transpiled Code
**Priority**: Low | **Effort**: 2h | **Milestone**: M6

**Description**: Highlight C/C++/Pascal code

**Tasks**:
- [ ] Use Monaco editor for equivalence panel (instead of `<pre>`)
- [ ] Set language (c, cpp, pascal)
- [ ] Make read-only
- [ ] Apply syntax highlighting

**Acceptance Criteria**:
- Transpiled code is syntax highlighted
- Looks professional

**Dependencies**: TASK-049

---

## Phase 6: Polish & Documentation

### 📝 Documentation

#### **TASK-053**: User Documentation
**Priority**: Medium | **Effort**: 6h

**Tasks**:
- [ ] Create `c-rewrite/README.md`:
  - [ ] Project overview
  - [ ] Build instructions (native + WASM)
  - [ ] Usage examples
  - [ ] CLI arguments
  - [ ] DAP setup for VS Code
- [ ] Create `c-rewrite/docs/`:
  - [ ] Architecture overview
  - [ ] API documentation
  - [ ] Grammar specification
  - [ ] Transpiler design
- [ ] Add comments to web interface

**Acceptance Criteria**:
- Clear build instructions
- Examples provided
- API documented

**Dependencies**: None (can be done anytime)

---

#### **TASK-054**: Developer Guide
**Priority**: Low | **Effort**: 4h

**Tasks**:
- [ ] Document code structure
- [ ] Explain design decisions
- [ ] Add contributing guide
- [ ] Document testing approach
- [ ] Code style guide

**Acceptance Criteria**:
- Developers can understand codebase
- Can extend easily

**Dependencies**: TASK-053

---

### 🎨 Polish

#### **TASK-055**: Error Messages
**Priority**: Medium | **Effort**: 4h

**Description**: Improve error reporting

**Tasks**:
- [ ] Add context to parse errors (show line, column, snippet)
- [ ] Add runtime error messages (line number, what went wrong)
- [ ] Display errors in web UI (error panel)
- [ ] Color-code errors (red) vs warnings (yellow)

**Acceptance Criteria**:
- Errors are clear and helpful
- Show source location
- Suggest fixes when possible

**Dependencies**: TASK-009, TASK-013

---

#### **TASK-056**: Performance Optimization
**Priority**: Low | **Effort**: Variable

**Description**: Optimize hot paths

**Tasks**:
- [ ] Profile interpreter (identify bottlenecks)
- [ ] Optimize value operations
- [ ] Optimize hash map lookups
- [ ] Use arena allocator for AST
- [ ] Optimize WASM build size (closure compiler, strip)

**Acceptance Criteria**:
- C interpreter faster than Python
- WASM size < 500KB
- No unnecessary allocations

**Dependencies**: All core tasks

---

#### **TASK-057**: Cross-Browser Testing
**Priority**: Medium | **Effort**: 3h

**Description**: Test web editor in different browsers

**Tasks**:
- [ ] Test in Chrome/Edge
- [ ] Test in Firefox
- [ ] Test in Safari
- [ ] Fix any browser-specific issues
- [ ] Test mobile (optional)

**Acceptance Criteria**:
- Works in all major browsers
- UI responsive
- WASM loads correctly

**Dependencies**: TASK-046

---

#### **TASK-058**: Accessibility
**Priority**: Low | **Effort**: 4h

**Description**: Make web editor accessible

**Tasks**:
- [ ] Add keyboard shortcuts
- [ ] Add ARIA labels
- [ ] Test with screen reader
- [ ] Ensure sufficient color contrast
- [ ] Add focus indicators

**Acceptance Criteria**:
- Passes accessibility audit
- Keyboard navigable
- Screen reader friendly

**Dependencies**: TASK-046

---

### 🚢 Deployment

#### **TASK-059**: Web Hosting
**Priority**: Medium | **Effort**: 2h

**Description**: Deploy web editor

**Tasks**:
- [ ] Choose hosting (GitHub Pages, Netlify, Vercel, etc.)
- [ ] Setup deployment
- [ ] Configure custom domain (optional)
- [ ] Add README with deployment instructions

**Acceptance Criteria**:
- Web editor accessible online
- Loads quickly
- Works offline after first load

**Dependencies**: TASK-046

---

#### **TASK-060**: Native Binaries
**Priority**: Low | **Effort**: 4h

**Description**: Build and distribute native binaries

**Tasks**:
- [ ] Setup CI/CD (GitHub Actions)
- [ ] Build for Linux/Mac/Windows
- [ ] Create releases on GitHub
- [ ] Add installation instructions

**Acceptance Criteria**:
- Binaries available for download
- Easy to install

**Dependencies**: TASK-024

---

## 🎯 Priority Summary

### Must Have (High Priority)
1. **Core Interpreter** (TASK-001 to TASK-016): Linter, Parser, Runtime, Tests
2. **Transpiler** (TASK-017 to TASK-023): C/C++/Pascal generation, Loop transforms
3. **Debugger** (TASK-025 to TASK-029): Step, Breakpoints, Variables
4. **WASM** (TASK-034 to TASK-039): Browser execution
5. **Web UI** (TASK-040 to TASK-047): Monaco editor, Debug controls
6. **Code Equivalence** (TASK-048 to TASK-052): Transpiler UI

### Should Have (Medium Priority)
- DAP Server (TASK-030 to TASK-033): VS Code integration
- Documentation (TASK-053)
- Error messages (TASK-055)
- Cross-browser testing (TASK-057)
- Deployment (TASK-059)

### Nice to Have (Low Priority)
- Standalone CLIs (TASK-005, TASK-023)
- Developer guide (TASK-054)
- Performance optimization (TASK-056)
- Accessibility (TASK-058)
- Native binaries (TASK-060)

---

## 📊 Progress Tracking

Total Tasks: **60**
- [ ] Phase 1: Core (16 tasks)
- [ ] Phase 2: Debugger (9 tasks)
- [ ] Phase 3: WASM (6 tasks)
- [ ] Phase 4: Web UI (8 tasks)
- [ ] Phase 5: Transpiler UI (5 tasks)
- [ ] Phase 6: Polish (16 tasks)

Estimated Total Effort: **~250 hours**

---

## 🔗 Dependencies Graph

```
TASK-001 (Setup)
  └─> TASK-002 (CMake)
       ├─> TASK-003 (Linter)
       │    ├─> TASK-004 (Linter Tests)
       │    └─> TASK-005 (Linter CLI)
       ├─> TASK-006 (Grammar)
       │    └─> TASK-007 (Submodule)
       │         └─> TASK-009 (Parser)
       │              ├─> TASK-010 (Parser Tests)
       │              ├─> TASK-013 (Runtime)
       │              │    ├─> TASK-014 (Runtime Tests)
       │              │    ├─> TASK-015 (Integration Tests)
       │              │    │    └─> TASK-016 (Fix Tests) ⭐
       │              │    ├─> TASK-024 (CLI)
       │              │    ├─> TASK-025 (Debugger Structs)
       │              │    │    └─> TASK-026 (Breakpoints)
       │              │    │         └─> TASK-027 (Debugger Core)
       │              │    │              ├─> TASK-028 (Watches)
       │              │    │              ├─> TASK-029 (Debugger Tests)
       │              │    │              └─> TASK-031 (DAP)
       │              │    │                   └─> TASK-032 (DAP Tests)
       │              │    │                        └─> TASK-033 (CLI DAP)
       │              │    └─> TASK-034 (Emscripten)
       │              │         └─> TASK-035 (WASM Bindings)
       │              │              └─> TASK-037 (JS Wrapper)
       │              │                   └─> TASK-040 (Web Structure)
       │              │                        └─> TASK-041 (Monaco)
       │              │                             └─> TASK-042 (Debug UI)
       │              │                                  └─> ... (Web tasks)
       │              └─> TASK-017 (Transpiler Core)
       │                   ├─> TASK-018 (C Codegen)
       │                   ├─> TASK-019 (C++ Codegen)
       │                   ├─> TASK-020 (Pascal Codegen)
       │                   ├─> TASK-021 (Loop Transform)
       │                   │    └─> TASK-022 (Transpiler Tests)
       │                   │         └─> TASK-023 (Transpiler CLI)
       │                   └─> TASK-048 (Transpiler UI)
       │                        └─> ... (Code equivalence tasks)
       └─> TASK-008 (AST Structs)
            └─> TASK-009 (Parser)
```

---

## 🛠️ Development Workflow

### Getting Started
1. Complete TASK-001, TASK-002 (setup)
2. Implement linter (TASK-003 to TASK-005)
3. Create grammar (TASK-006, TASK-007)
4. Build parser and runtime (TASK-008 to TASK-013)
5. **Milestone: Get 62 tests passing** (TASK-015, TASK-016)

### Iteration Strategy
- Work on one task at a time
- Test thoroughly before moving on
- Keep tests passing (green)
- Document as you go

### Testing Strategy
- Write unit tests for each module
- Run integration tests frequently
- Test WASM in browser early
- Fix bugs immediately

---

## 📌 Notes

- **Start with the basics**: Get interpreter working first (M1), then add features
- **Test early, test often**: All 62 BAC tests must pass before Phase 2
- **WASM is critical**: Test browser integration early to avoid surprises
- **Transpiler is fun**: The code equivalence feature will be unique and educational
- **Keep it simple**: No server, no tracking, no complexity
- **Performance**: C should be much faster than Python, WASM should be fast enough

---

## 🎓 Learning Resources

- [Tree-sitter Documentation](https://tree-sitter.github.io/)
- [Emscripten Documentation](https://emscripten.org/docs/)
- [Monaco Editor Playground](https://microsoft.github.io/monaco-editor/playground.html)
- [Debug Adapter Protocol](https://microsoft.github.io/debug-adapter-protocol/)
- [CMake Tutorial](https://cmake.org/cmake/help/latest/guide/tutorial/)

---

## ✅ Next Steps

**Start here**: TASK-001 (Project Structure Setup)

Good luck! 🚀
