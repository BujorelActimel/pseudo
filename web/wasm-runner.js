/**
 * PseudoInterpreter - JavaScript wrapper for the WASM-compiled C interpreter
 */
class PseudoInterpreter {
  constructor() {
    this.module = null;
    this.initialized = false;
    this.running = false;
    this.stopRequested = false;

    // Bound C functions (set after init)
    this._init = null;
    this._load = null;
    this._step = null;
    this._pushInput = null;
    this._hasOutput = null;
    this._popOutput = null;
    this._freeOutput = null;
    this._getError = null;
    this._getLine = null;
    this._reset = null;
    this._needsInput = null;
  }

  /**
   * Initialize the WASM module
   */
  async init() {
    if (this.initialized) return;

    // Load the WASM module
    this.module = await PseudoModule();

    // Bind C functions using cwrap
    this._init = this.module.cwrap('pseudo_init', 'number', []);
    this._getInitError = this.module.cwrap('pseudo_get_init_error', 'string', []);
    this._load = this.module.cwrap('pseudo_load', 'number', ['string']);
    this._step = this.module.cwrap('pseudo_step', 'number', []);
    this._pushInput = this.module.cwrap('pseudo_push_input', null, ['string']);
    this._hasOutput = this.module.cwrap('pseudo_has_output', 'number', []);
    this._popOutput = this.module.cwrap('pseudo_pop_output', 'number', []);
    this._freeOutput = this.module.cwrap('pseudo_free_output', null, ['number']);
    this._getError = this.module.cwrap('pseudo_get_error', 'string', []);
    this._getLine = this.module.cwrap('pseudo_get_line', 'number', []);
    this._reset = this.module.cwrap('pseudo_reset', null, []);
    this._needsInput = this.module.cwrap('pseudo_needs_input', 'number', []);
    this._requestStop = this.module.cwrap('pseudo_request_stop', null, []);

    // Initialize the runtime
    const initResult = this._init();
    if (!initResult) {
      const error = this._getInitError() || 'Unknown initialization error';
      throw new Error(`WASM initialization failed: ${error}`);
    }

    this.initialized = true;
  }

  /**
   * Run the given code
   * @param {string} code - Source code to execute
   * @param {Object} options - Execution options
   * @param {function(string)} options.onOutput - Called for each output
   * @param {function(): Promise<string>} options.onNeedsInput - Called when input needed
   * @param {function(string)} options.onError - Called on errors
   * @param {function(number)} options.onStep - Called on each step in debug mode
   * @param {boolean} options.debug - Enable debug mode
   */
  async run(code, options = {}) {
    if (!this.initialized) {
      throw new Error('Interpreter not initialized. Call init() first.');
    }

    if (this.running) {
      throw new Error('Interpreter is already running.');
    }

    const {
      onOutput = () => {},
      onNeedsInput = async () => '',
      onError = () => {},
      onStep = () => {},
      debug = false,
    } = options;

    this.running = true;
    this.stopRequested = false;

    // Reset and initialize for new run
    this._reset();
    this._init();

    // Load the source code
    const loadResult = this._load(code);
    if (!loadResult) {
      const error = this._getError() || 'Failed to load source code';
      onError(error);
      this.running = false;
      return;
    }

    // Execution state constants
    const EXEC_CONTINUE = 0;
    const EXEC_DONE = 1;
    const EXEC_NEEDS_INPUT = 2;
    const EXEC_ERROR = 3;

    let stepCount = 0;

    // Main execution loop
    while (!this.stopRequested) {
      // Drain any pending output
      while (this._hasOutput()) {
        const ptr = this._popOutput();
        if (ptr) {
          const text = this.module.UTF8ToString(ptr);
          this._freeOutput(ptr);
          onOutput(text);
        }
      }

      // Execute one step
      const state = this._step();

      // Report line in debug mode
      if (debug) {
        const line = this._getLine();
        onStep(line);
      }

      // Handle state
      if (state === EXEC_DONE) {
        // Drain final output
        while (this._hasOutput()) {
          const ptr = this._popOutput();
          if (ptr) {
            const text = this.module.UTF8ToString(ptr);
            this._freeOutput(ptr);
            onOutput(text);
          }
        }
        break;
      }

      if (state === EXEC_ERROR) {
        const error = this._getError() || 'Unknown error';
        onError(error);
        break;
      }

      if (state === EXEC_NEEDS_INPUT) {
        // Drain any pending output before requesting input
        while (this._hasOutput()) {
          const ptr = this._popOutput();
          if (ptr) {
            const text = this.module.UTF8ToString(ptr);
            this._freeOutput(ptr);
            onOutput(text);
          }
        }
        // Request input from JS
        const inputValue = await onNeedsInput();
        if (this.stopRequested) break;
        this._pushInput(inputValue);
      }

      // Yield to browser every N steps to keep UI responsive
      stepCount++;
      if (stepCount % 100 === 0) {
        await this._yield();
      }

      // Add delay in debug mode
      if (debug) {
        await this._delay(100);
      }
    }

    this.running = false;
  }

  /**
   * Stop the current execution
   */
  stop() {
    this.stopRequested = true;
    // Request the C runtime to stop (checked in loops)
    if (this._requestStop) {
      this._requestStop();
    }
    // Force reset running state so a new run can start
    this.running = false;
  }

  /**
   * Check if currently running
   */
  isRunning() {
    return this.running;
  }

  /**
   * Yield to browser for rendering
   */
  _yield() {
    return new Promise(resolve => requestAnimationFrame(resolve));
  }

  /**
   * Delay for debugging
   */
  _delay(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
  }
}

// Export for module systems, or attach to window for browser
if (typeof module !== 'undefined' && module.exports) {
  module.exports = { PseudoInterpreter };
} else if (typeof window !== 'undefined') {
  window.PseudoInterpreter = PseudoInterpreter;
}
