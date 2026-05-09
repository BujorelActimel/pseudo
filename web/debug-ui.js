    // Update debug UI
    function updateDebugUI(variables, changedVars = [], conditionInfo = null) {
      // Update condition display
      if (conditionInfo && conditionInfo.valid) {
        conditionDisplay.classList.add('visible');
        conditionDisplay.innerHTML = `
          <span class="condition-label">Condition:</span>
          <span class="condition-text">${escapeHtml(conditionInfo.text)}</span>
          <span class="condition-result ${conditionInfo.result ? 'true' : 'false'}">${conditionInfo.result ? 'TRUE' : 'FALSE'}</span>
        `;
      } else {
        conditionDisplay.classList.remove('visible');
        conditionDisplay.innerHTML = '';
      }

      // Update variables list
      if (variables.length === 0) {
        variablesContent.innerHTML = '<span class="variables-empty">No variables</span>';
      } else {
        variablesContent.innerHTML = `<table class="variables-table">
          <thead><tr>
            <th>Nume</th><th>Tip</th><th>Valoare</th>
          </tr></thead>
          <tbody>${variables.map(v => {
            const isChanged = changedVars.includes(v.name);
            return `<tr class="${isChanged ? 'changed' : ''}">
              <td class="variable-name">${escapeHtml(v.name)}</td>
              <td class="variable-type">${escapeHtml(v.type)}</td>
              <td class="variable-value">${escapeHtml(v.value)}</td>
            </tr>`;
          }).join('')}</tbody>
        </table>`;
      }

      // Update step counter and button states
      if (debugger_) {
        const current = debugger_.getCurrentStep();
        variablesStep.textContent = `Pas ${current}`;
        debugStepInto.disabled = debugger_.isProgramDone;
        debugContinue.disabled = debugger_.isProgramDone;
      }
    }

    function escapeHtml(str) {
      const div = document.createElement('div');
      div.textContent = str;
      return div.innerHTML;
    }

    // Highlight current statement in editor (content range, not whole line)
    function highlightDebugLine(line) {
      if (!editor) return;

      const monacoLine = line + 1;
      const model = editor.getModel();
      const lineContent = model ? model.getLineContent(monacoLine) : '';

      let startCol = 1;
      let endCol = lineContent.length + 1;
      if (lineContent.length > 0) {
        const firstNonWS = lineContent.search(/\S/);
        const lastNonWS = lineContent.trimEnd().length;
        if (firstNonWS >= 0) {
          startCol = firstNonWS + 1;
          endCol = lastNonWS + 1;
        }
      }

      debugLineDecorations = editor.deltaDecorations(debugLineDecorations, [{
        range: new monaco.Range(monacoLine, startCol, monacoLine, endCol),
        options: {
          className: 'debug-line-highlight',
          glyphMarginClassName: 'debug-line-glyph'
        }
      }]);

      editor.revealLineInCenter(monacoLine);
    }

    function clearDebugHighlight() {
      if (editor) {
        debugLineDecorations = editor.deltaDecorations(debugLineDecorations, []);
      }
    }

    // Run or debug code
    async function runCode(debug = false) {
      if (isRunning) return;

      // Check if WASM is loaded
      if (!wasmReady || !Module || !Module._pseudo_init) {
        clearConsole();
        appendToConsole('Interpretorul nu este încărcat încă. Așteptați...', 'error');
        return;
      }

      isRunning = true;
      isDebugging = debug;
      stopRequested = false;

      btnRun.style.display = 'none';
      btnDebug.style.display = 'none';
      btnStop.style.display = debug ? 'none' : 'flex';
      debugIndicator.style.display = debug ? 'inline' : 'none';

      // Show/hide debug controls and variables panel
      debugToolbarControls.classList.toggle('visible', debug);
      variablesPanel.classList.toggle('visible', debug);

      clearConsole();
      clearDebugHighlight();
      appendToConsole('Program pornit.', 'system');

      // Initialize runtime
      if (!Module._pseudo_init()) {
        const error = Module._pseudo_get_init_error();
        appendToConsole('Eroare la inițializare: ' + (error ? Module.UTF8ToString(error) : 'eroare necunoscută'), 'error');
        stopCode(true);
        return;
      }

      // Load source code
      const source = editor.getValue();
      const sourcePtr = Module.allocateUTF8(source);
      const loadResult = Module._pseudo_load(sourcePtr);
      Module._free(sourcePtr);

      if (!loadResult) {
        const error = Module._pseudo_get_error();
        appendToConsole('Eroare de sintaxă: ' + (error ? Module.UTF8ToString(error) : 'eroare necunoscută'), 'error');
        stopCode(true);
        return;
      }

      if (debug) {
        // Initialize debugger and enable debug mode (step-by-step visible actions)
        debugger_ = new PseudoDebugger(Module);
        debugger_.setDebugMode(true);
        debugger_.reset();
        updateDebugUI([], [], null);

        // Highlight first line
        const firstLine = debugger_.getNextLine();
        highlightDebugLine(firstLine);

        // Enable controls
        debugStepInto.disabled = false;
        debugContinue.disabled = false;

        appendToConsole('Mod depanare: apăsați Pas (F11) sau Continuă (F5).', 'debug');
      } else {
        // Run mode - disable debug mode for fast execution
        if (Module._pseudo_set_debug_mode) {
          Module._pseudo_set_debug_mode(0);
        }
        await executeRunMode();
      }
    }

    async function executeRunMode() {
      const MAX_OUTPUT_LINES = 1000;
      const STEPS_PER_FRAME = 2000;
      let stepCount = 0;

      const processOutput = () => {
        while (Module._pseudo_has_output()) {
          const ptr = Module._pseudo_pop_output();
          if (!ptr) continue;
          const output = Module.UTF8ToString(ptr);
          Module._pseudo_free_output(ptr);
          appendToConsole(output, 'output');
        }
      };

      // Remove oldest output lines so at most MAX_OUTPUT_LINES remain.
      // Called after the rAF yield so flushOutputBuffer has already run.
      const trimConsole = () => {
        const lines = consoleOutput.querySelectorAll('.console-line.output');
        if (lines.length <= MAX_OUTPUT_LINES) return;
        const excess = lines.length - MAX_OUTPUT_LINES;
        for (let i = 0; i < excess; i++) lines[i].remove();
      };

      // 0 = EXEC_CONTINUE, 1 = EXEC_DONE, 2 = EXEC_NEEDS_INPUT, 3 = EXEC_ERROR
      while (!stopRequested) {
        const result = Module._pseudo_step();
        processOutput();

        if (result === 1) {
          appendToConsole('Program terminat.', 'system');
          stopCode(true);
          return;
        } else if (result === 2) {
          isWaitingForInput = true;
          showInputPrompt();
          await new Promise(resolve => {
            inputCallback = (value) => {
              const inputPtr = Module.allocateUTF8(value);
              Module._pseudo_push_input(inputPtr);
              Module._free(inputPtr);
              resolve();
            };
          });
          if (stopRequested) return;
        } else if (result === 3) {
          const error = Module._pseudo_get_error();
          appendToConsole('Eroare de execuție: ' + (error ? Module.UTF8ToString(error) : 'eroare necunoscută'), 'error');
          stopCode(true);
          return;
        }

        stepCount++;
        if (stepCount % STEPS_PER_FRAME === 0) {
          // flushOutputBuffer's rAF was registered before this one, so it
          // fires first — DOM is up-to-date by the time trimConsole runs.
          await new Promise(resolve => requestAnimationFrame(resolve));
          trimConsole();
        }
      }

      processOutput();
    }

    // Debug step handlers
    async function debugStepIntoHandler() {
      if (!debugger_ || !isDebugging) return;

      const processOutput = () => {
        while (Module._pseudo_has_output()) {
          const ptr = Module._pseudo_pop_output();
          if (ptr) {
            const output = Module.UTF8ToString(ptr);
            Module._pseudo_free_output(ptr);
            appendToConsole(output, 'output');
          }
        }
      };

      const result = debugger_.stepInto();
      processOutput();

      if (result.needsInput) {
        isWaitingForInput = true;
        debugStepInto.disabled = true;
        showInputPrompt();
        inputCallback = (value) => {
          const inputPtr = Module.allocateUTF8(value);
          Module._pseudo_push_input(inputPtr);
          Module._free(inputPtr);
          debugStepInto.disabled = false;
          debugStepIntoHandler(); // Re-execute the step after input
        };
        return;
      }

      if (result.hasError) {
        const error = Module._pseudo_get_error();
        appendToConsole('Eroare de execuție: ' + (error ? Module.UTF8ToString(error) : 'eroare necunoscută'), 'error');
      }

      updateDebugUI(result.variables || [], result.changedVars || [], result.conditionInfo);
      highlightDebugLine(result.line);

      if (result.done) {
        appendToConsole('Program terminat.', 'system');
        debugStepInto.disabled = true;
      }
    }

    async function debugContinueHandler() {
      if (!debugger_ || !isDebugging || isWaitingForInput) return;

      // Disable buttons during continue
      debugStepInto.disabled = true;
      debugContinue.disabled = true;

      const processOutput = () => {
        while (Module._pseudo_has_output()) {
          const ptr = Module._pseudo_pop_output();
          if (ptr) {
            const output = Module.UTF8ToString(ptr);
            Module._pseudo_free_output(ptr);
            appendToConsole(output, 'output');
          }
        }
      };

      // Run until done/error/input using the fast run function
      const result = Module._pseudo_run();
      processOutput();

      // Get final state
      const variables = debugger_.getVariables();
      updateDebugUI(variables, [], null);
      clearDebugHighlight();

      // 0 = EXEC_CONTINUE, 1 = EXEC_DONE, 2 = EXEC_NEEDS_INPUT, 3 = EXEC_ERROR
      if (result === 2) {
        // Needs input
        isWaitingForInput = true;
        showInputPrompt();
        const line = Module._pseudo_get_line();
        highlightDebugLine(line);
        debugStepInto.disabled = false;
        debugContinue.disabled = false;
        inputCallback = (value) => {
          const inputPtr = Module.allocateUTF8(value);
          Module._pseudo_push_input(inputPtr);
          Module._free(inputPtr);
          debugContinueHandler();
        };
        return;
      }

      if (result === 1) {
        // Done
        appendToConsole('Program terminat.', 'system');
        stopCode(true);
      } else if (result === 3) {
        // Error
        const error = Module._pseudo_get_error();
        appendToConsole('Eroare de execuție: ' + (error ? Module.UTF8ToString(error) : 'eroare necunoscută'), 'error');
        stopCode(true);
      }
    }

    function stopCode(silent = false) {
      if (isDebugging && debugger_) {
        debugger_.reset();
        debugger_ = null;
      }

      stopRequested = true;
      isRunning = false;
      isDebugging = false;
      isWaitingForInput = false;
      inputCallback = null;

      const inputLine = consoleOutput.querySelector('.input-line');
      if (inputLine) inputLine.remove();

      if (Module && Module._pseudo_request_stop) {
        Module._pseudo_request_stop();
      }

      if (!silent) {
        appendToConsole('Program oprit.', 'system');
      }

      btnRun.style.display = 'flex';
      btnDebug.style.display = 'flex';
      btnStop.style.display = 'none';
      debugIndicator.style.display = 'none';
      debugToolbarControls.classList.remove('visible');
      variablesPanel.classList.remove('visible');

      clearDebugHighlight();
    }

    // Event listeners
    btnRun.addEventListener('click', () => runCode(false));
    btnDebug.addEventListener('click', () => runCode(true));
    btnStop.addEventListener('click', stopCode);
    btnClear.addEventListener('click', clearConsole);

    // Debug control event listeners
    debugStepInto.addEventListener('click', debugStepIntoHandler);
    debugContinue.addEventListener('click', debugContinueHandler);
    debugStop.addEventListener('click', stopCode);
