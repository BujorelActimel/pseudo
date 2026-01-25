# C Rewrite Implementation Roadmap

> **Project**: Romanian Pseudocode Interpreter in C with Tree-sitter
> **Location**: `c-rewrite/`
> **Goal**: Native + WASM interpreter with debugger and transpiler

---

## Current Project Structure

```
c-rewrite/
├── include/pseudo/           # Public headers
│   ├── string.h             # Dynamic string type [DONE]
│   ├── hashmap.h            # String-keyed hashmap [DONE]
│   ├── linter.h             # Text preprocessing [DONE]
│   ├── value.h              # Dynamic typed values [DONE]
│   ├── environment.h        # Variable storage [DONE]
│   ├── parser.h             # Tree-sitter wrapper [IN PROGRESS]
│   ├── io.h                 # I/O abstraction [PENDING]
│   ├── runtime.h            # Interpreter API [PENDING]
│   └── transpiler.h         # Transpiler API [PENDING]
├── src/
│   ├── cli/main.c           # CLI entry point [PARTIAL]
│   ├── common/
│   │   ├── string.c         # [DONE]
│   │   └── hashmap.c        # [DONE]
│   ├── linter/linter.c      # [DONE]
│   ├── parser/parser.c      # [IN PROGRESS]
│   ├── runtime/
│   │   ├── value.c          # [DONE]
│   │   ├── environment.c    # [DONE]
│   │   └── interpreter.c    # [PENDING]
│   ├── io/
│   │   ├── io_stdio.c       # CLI I/O [PENDING]
│   │   └── io_buffered.c    # WASM I/O [PENDING]
│   ├── transpiler/
│   │   ├── transpiler.c     # [PENDING]
│   │   ├── codegen_c.c      # [PENDING]
│   │   ├── codegen_cpp.c    # [PENDING]
│   │   └── codegen_pascal.c # [PENDING]
│   ├── debugger/            # [FUTURE]
│   ├── dap/                 # [FUTURE]
│   └── wasm/
│       └── bridge.c         # WASM exports [PENDING]
├── test/
│   └── test_linter.c        # [DONE]
├── tree-sitter-pseudo/      # Grammar [DONE]
├── web/
│   ├── editor.html          # Web UI [DONE - JS interpreter]
│   ├── pseudo.js            # Emscripten output [PENDING]
│   ├── pseudo.wasm          # WASM binary [PENDING]
│   └── wasm-runner.js       # JS wrapper [PENDING]
├── Makefile                 # Build system [DONE]
└── README.md
```

---

## Architecture

```
Source Code
    ↓ (linter)
Cleaned Source
    ↓ (tree-sitter parser)
CST (TSTree) ─────────────────────────────────┐
    ↓                                         ↓
    ├─→ [Interpreter] → Execution        [Transpiler] → C/C++/Pascal code
    │        ↓
    │   I/O Abstraction
    │        ↓
    ├─→ [CLI: stdio]
    └─→ [WASM: buffered I/O]
```

**Key Design Decisions**:
1. **No separate AST** - Use tree-sitter CST directly with `ts_node_child_by_field_name()`
2. **Pluggable I/O** - `io_t` interface with stdio (CLI) and buffered (WASM) backends
3. **Dynamic typing** - `value_t` with int/float/string variants
4. **Romanian error messages** - Hardcoded for now, multi-language later
5. **Undefined variables = 0** - Match Python interpreter behavior
6. **Cooperative yielding** - Step-based interpreter for WASM + debugger support:
   - `runtime_step()` executes one statement and returns state
   - `EXEC_NEEDS_INPUT` signals JS to collect input asynchronously
   - Enables batched rendering (yield every N steps)

---

## 🎯 Milestones

- [x] **M0**: Project Setup (Makefile, structure, linter)
- [ ] **M1**: Core Interpreter (Parser + Runtime)
- [ ] **M2**: Transpiler (C/C++/Pascal)
- [ ] **M3**: WASM Build
- [ ] **M4**: Debugger + DAP
- [ ] **M5**: Web Editor

---

## Phase 1: Core Interpreter

### Completed

#### ✅ String Type (`string.h`, `string.c`)
Dynamic string with capacity management.

#### ✅ Hashmap (`hashmap.h`, `hashmap.c`)
String-keyed hashmap with open addressing and linear probing.

#### ✅ Linter (`linter.h`, `linter.c`)
Text preprocessing:
- Unicode normalization (≤→<=, ←→<-, √→sqrt, etc.)
- Romanian diacritics (ă→a, ș→s, ț→t)
- Box character removal
- Indentation normalization

#### ✅ Tree-sitter Grammar (`tree-sitter-pseudo/`)
Complete grammar for Romanian pseudocode:
- Statements: assign, swap, read, write, if, for, while, do_while, repeat
- Expressions: arithmetic, comparison, logical, floor, sqrt
- Literals: numbers, strings, identifiers

#### ✅ Value System (`value.h`, `value.c`)
Dynamic typed values:
- Types: `VALUE_INT`, `VALUE_FLOAT`, `VALUE_STRING`
- Arithmetic: +, -, *, / (always float), %, neg, sqrt, floor
- Comparison: =, !=, <, <=, >, >=
- Logical: SI/AND, SAU/OR, NOT
- Error handling: `value_error_t` enum with Romanian messages
- String concatenation for string + string

#### ✅ Environment (`environment.h`, `environment.c`)
Variable storage:
- Hash table with `string_t` keys and `value_t*` values
- Takes ownership of values
- env_set, env_get, env_has, env_clear, env_foreach

### In Progress

#### 🔄 Parser Wrapper (`parser.h`, `parser.c`)
Thin wrapper around tree-sitter:
```c
typedef struct parser parser_t;

parser_t* parser_create(void);
void parser_destroy(parser_t* parser);
parser_error_t parser_parse(parser_t* parser, const string_t* source);
TSNode parser_root(parser_t* parser);
string_t* parser_node_text(parser_t* parser, TSNode node);
```

Error type:
```c
typedef enum {
    PARSER_OK,
    PARSER_ERR_MEMORY,
    PARSER_ERR_SYNTAX,
} parser_error_type_t;

typedef struct {
    parser_error_type_t type;
    uint32_t line;
    uint32_t column;
    string_t* message;
} parser_error_t;
```

### Pending

#### ⏳ I/O Abstraction (`io.h`, `io_stdio.c`, `io_buffered.c`)

**Design**: Pluggable I/O with two backends - stdio for CLI, buffered for WASM.

```c
typedef struct io io_t;

typedef struct {
    void (*write)(io_t* io, const char* text);
    const char* (*read)(io_t* io);  // Returns NULL if input not ready (WASM)
    void (*destroy)(io_t* io);
} io_ops_t;

struct io {
    io_ops_t ops;
    void* ctx;
};

// CLI backend - blocking I/O
io_t* io_stdio_create(void);

// WASM backend - non-blocking buffered I/O
io_t* io_buffered_create(void);
void io_buffered_push_input(io_t* io, const char* value);
char* io_buffered_pop_output(io_t* io);   // Returns NULL if empty, caller owns
bool io_buffered_has_output(io_t* io);
bool io_buffered_needs_input(io_t* io);
void io_buffered_clear(io_t* io);
```

#### ⏳ Interpreter (`runtime.h`, `interpreter.c`)

**Design**: Step-based execution for WASM compatibility and debugging support.

```c
typedef struct runtime runtime_t;

typedef enum {
    EXEC_CONTINUE,    // More steps to run
    EXEC_DONE,        // Program finished
    EXEC_NEEDS_INPUT, // Waiting for input (check io_buffered_needs_input)
    EXEC_ERROR        // Error occurred (check runtime_get_error)
} exec_state_t;

// Lifecycle
runtime_t* runtime_create(io_t* io);
void runtime_destroy(runtime_t* rt);

// Execution
bool runtime_load(runtime_t* rt, const char* source);
exec_state_t runtime_step(runtime_t* rt);        // Execute one statement
exec_state_t runtime_run(runtime_t* rt);         // Run until done/input/error (CLI)
const char* runtime_get_error(runtime_t* rt);
```

Internal functions:
- `exec_stmt()` - dispatch by node type, returns exec_state_t
- `exec_assign()`, `exec_swap()`, `exec_read()`, `exec_write()`
- `exec_if()`, `exec_for()`, `exec_while()`, `exec_do_while()`, `exec_repeat()`
- `eval_expr()` - recursive expression evaluation

#### ⏳ CLI Update (`cli/main.c`)
```
./pseudo program.pseudo              # Execute
./pseudo lint program.pseudo         # Lint only (current)
./pseudo transpile c program.pseudo  # Transpile to C
```

---

## Phase 2: Transpiler

#### ⏳ Transpiler Core (`transpiler.h`, `transpiler.c`)
```c
typedef enum {
    LANG_C,
    LANG_CPP,
    LANG_PASCAL,
} target_lang_t;

typedef struct transpiler transpiler_t;

transpiler_t* transpiler_create(void);
void transpiler_destroy(transpiler_t* t);
char* transpiler_convert(transpiler_t* t, const char* source, target_lang_t lang);
```

#### ⏳ Code Generators
- `codegen_c.c` - C with stdio.h, math.h
- `codegen_cpp.c` - C++ with iostream
- `codegen_pascal.c` - Free Pascal

---

## Phase 3: WASM Build

**Design**: Cooperative yielding - interpreter runs in steps, yields to JS for I/O.

```
┌──────────────────────────────────────────┐
│              JavaScript                   │
├──────────────────────────────────────────┤
│  while (state !== DONE) {                │
│      state = wasm.step();                │
│      if (state === NEEDS_INPUT) {        │
│          await getUserInput();           │
│          wasm.provideInput(value);       │
│      }                                   │
│      flushOutput();                      │
│      await yieldToRender();  // batch    │
│  }                                       │
└──────────────────────────────────────────┘
```

#### ⏳ WASM Bridge (`src/wasm/bridge.c`)
```c
#include <emscripten.h>

static runtime_t* g_runtime = NULL;
static io_t* g_io = NULL;

EMSCRIPTEN_KEEPALIVE
void pseudo_init(void) {
    g_io = io_buffered_create();
    g_runtime = runtime_create(g_io);
}

EMSCRIPTEN_KEEPALIVE
int pseudo_load(const char* source) {
    return runtime_load(g_runtime, source) ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE
int pseudo_step(void) {
    return (int)runtime_step(g_runtime);  // Returns exec_state_t
}

EMSCRIPTEN_KEEPALIVE
const char* pseudo_get_output(void) {
    return io_buffered_pop_output(g_io);  // NULL if empty
}

EMSCRIPTEN_KEEPALIVE
void pseudo_provide_input(const char* value) {
    io_buffered_push_input(g_io, value);
}

EMSCRIPTEN_KEEPALIVE
const char* pseudo_get_error(void) {
    return runtime_get_error(g_runtime);
}

EMSCRIPTEN_KEEPALIVE
void pseudo_reset(void) {
    io_buffered_clear(g_io);
}
```

#### ⏳ Emscripten Build (Makefile target)
```makefile
EMCC_FLAGS = -O3 -s WASM=1 -s EXPORTED_RUNTIME_METHODS='["UTF8ToString","stringToUTF8"]'
EMCC_EXPORTS = -s EXPORTED_FUNCTIONS='["_pseudo_init","_pseudo_load","_pseudo_step","_pseudo_get_output","_pseudo_provide_input","_pseudo_get_error","_pseudo_reset","_malloc","_free"]'

wasm: $(WASM_OBJS)
	emcc $(EMCC_FLAGS) $(EMCC_EXPORTS) $^ -o web/pseudo.js
```

#### ⏳ JavaScript Runner (`web/wasm-runner.js`)
```javascript
const EXEC_CONTINUE = 0, EXEC_DONE = 1, EXEC_NEEDS_INPUT = 2, EXEC_ERROR = 3;

class PseudoRunner {
    constructor(wasmModule) {
        this.wasm = wasmModule;
        this.wasm._pseudo_init();
    }

    async run(source, callbacks) {
        const srcPtr = this.allocString(source);
        this.wasm._pseudo_load(srcPtr);
        this.wasm._free(srcPtr);

        let stepCount = 0;
        while (true) {
            const state = this.wasm._pseudo_step();

            // Flush output
            let outPtr;
            while ((outPtr = this.wasm._pseudo_get_output())) {
                callbacks.onOutput(UTF8ToString(outPtr));
                this.wasm._free(outPtr);
            }

            if (state === EXEC_DONE) break;
            if (state === EXEC_ERROR) {
                throw new Error(UTF8ToString(this.wasm._pseudo_get_error()));
            }
            if (state === EXEC_NEEDS_INPUT) {
                const value = await callbacks.onInput();
                const valPtr = this.allocString(value);
                this.wasm._pseudo_provide_input(valPtr);
                this.wasm._free(valPtr);
            }

            // Yield every 50 steps to allow rendering
            if (++stepCount % 50 === 0) {
                await new Promise(r => requestAnimationFrame(r));
            }
        }
    }

    allocString(str) {
        const len = lengthBytesUTF8(str) + 1;
        const ptr = this.wasm._malloc(len);
        stringToUTF8(str, ptr, len);
        return ptr;
    }
}
```

---

## Phase 4: Debugger

**Design**: Builds on step-based execution from Phase 1/3.

#### ⏳ Debug Extensions to Runtime
```c
// Line tracking (already available from tree-sitter nodes)
uint32_t runtime_get_current_line(runtime_t* rt);
uint32_t runtime_get_current_column(runtime_t* rt);

// Variable inspection
char* runtime_get_variables_json(runtime_t* rt);  // {"varname": value, ...}

// Breakpoints
void runtime_add_breakpoint(runtime_t* rt, uint32_t line);
void runtime_remove_breakpoint(runtime_t* rt, uint32_t line);
void runtime_clear_breakpoints(runtime_t* rt);
exec_state_t runtime_continue(runtime_t* rt);  // Run until breakpoint/done/input
```

#### ⏳ WASM Debug Exports (`src/wasm/bridge.c`)
```c
EMSCRIPTEN_KEEPALIVE
int pseudo_get_line(void) {
    return runtime_get_current_line(g_runtime);
}

EMSCRIPTEN_KEEPALIVE
const char* pseudo_get_variables(void) {
    return runtime_get_variables_json(g_runtime);
}

EMSCRIPTEN_KEEPALIVE
void pseudo_add_breakpoint(int line) {
    runtime_add_breakpoint(g_runtime, line);
}

EMSCRIPTEN_KEEPALIVE
int pseudo_continue(void) {
    return (int)runtime_continue(g_runtime);
}
```

#### ⏳ DAP Server (CLI only)
Debug Adapter Protocol over stdio for VS Code integration.
- Separate executable: `pseudo-dap`
- Handles: initialize, launch, setBreakpoints, threads, stackTrace, scopes, variables, continue, next, stepIn, disconnect

---

## Phase 5: Web Editor

**Current**: `c-rewrite/web/editor.html` - JS interpreter with buffered output rendering

**Integration with WASM**:
```javascript
// Replace JS interpreter with WASM runner
const runner = new PseudoRunner(wasmModule);

async function runCode(debug = false) {
    clearConsole();
    appendToConsole('Program started.', 'system');

    try {
        await runner.run(editor.getValue(), {
            onOutput: (text) => appendToConsole(text),
            onInput: () => waitForInput(),  // Returns promise
            onStep: debug ? (line) => highlightLine(line) : null,
        });
        appendToConsole('Program finished.', 'system');
    } catch (err) {
        appendToConsole(`Error: ${err.message}`, 'error');
    }
}
```

**Features**:
- Monaco editor with pseudocode language highlighting
- Run/Debug/Stop controls
- Buffered console output with input prompt
- Debug mode: step, continue, breakpoints, variable inspector
- Line highlighting during debug
- Transpiler panel (C/C++/Pascal equivalents)

---

## Build System

Using Makefile (not CMake):

```makefile
make          # Debug build with sanitizers
make release  # Optimized build
make test     # Run unit tests
make clean    # Clean build artifacts

make grammar-generate  # Regenerate tree-sitter parser
make grammar-test      # Run tree-sitter tests
```

Compiler flags:
- Debug: `-std=c23 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address,undefined`
- Release: `-std=c23 -Wall -Wextra -Wpedantic -O3 -march=native`

---

## Testing Strategy

1. **Unit tests** - per-module tests in `test/`
2. **Integration tests** - 62 BAC exam programs
3. **Transpiler tests** - compile and run generated code

---

## Special Behaviors

From Python interpreter (to match exactly):
- Variables default to 0 if undefined
- `scrie` concatenates without spaces
- `citeste` auto-converts to int/float if valid number pattern, otherwise string
- Floor `[x]` truncates to int
- Division `/` always returns float (use `[x/10]` for int division)
- Sqrt raises error on negative numbers
- Type mismatch is an error (e.g., `"abc" - 5`)

---

## Error Messages (Romanian)

```c
"Fara eroare"
"Tipuri incompatibile"
"Impartire la zero"
"Nu se poate calcula radicalul unui numar negativ"
"Eroare de sintaxa"
"Nu s-a putut aloca memorie"
```

---

## Next Steps

1. ✅ Complete parser wrapper implementation
2. ⏳ Implement I/O abstraction (io_stdio first)
3. ⏳ Implement interpreter core
4. ⏳ Update CLI with run mode
5. ⏳ Run integration tests against 62 BAC programs
6. ⏳ Implement transpiler

---

## Dependencies

- tree-sitter (system library, `-ltree-sitter`)
- math library (`-lm`)
- C23 compiler (GCC 14+ or Clang 18+)
