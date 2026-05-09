    // PseudoDebugger class for step-through debugging
    class PseudoDebugger {
      constructor(wasm) {
        this.wasm = wasm;
        this.stateHistory = [];      // Array of {snapshotId, line, variables}
        this.currentHistoryIndex = -1;
        this.maxHistory = 100;
        this.previousVariables = {};
        this.isAtEnd = false;
        this.isProgramDone = false;
      }

      reset() {
        this.stateHistory = [];
        this.currentHistoryIndex = -1;
        this.previousVariables = {};
        this.isAtEnd = false;
        this.isProgramDone = false;
        if (this.wasm) {
          this.wasm._pseudo_clear_snapshots();
        }
      }

      getVariables() {
        if (!this.wasm) return [];
        const ptr = this.wasm._pseudo_get_variables_json();
        if (!ptr) return [];
        const json = this.wasm.UTF8ToString(ptr);
        this.wasm._pseudo_free_output(ptr);
        try {
          return JSON.parse(json);
        } catch (e) {
          console.error('Failed to parse variables JSON:', e);
          return [];
        }
      }

      getNextLine() {
        if (!this.wasm) return 0;
        return this.wasm._pseudo_get_next_line();
      }

      getConditionInfo() {
        if (!this.wasm) return { valid: false };
        const ptr = this.wasm._pseudo_get_condition_info();
        if (!ptr) return { valid: false };
        const json = this.wasm.UTF8ToString(ptr);
        this.wasm._pseudo_free_output(ptr);
        try {
          return JSON.parse(json);
        } catch (e) {
          return { valid: false };
        }
      }

      setDebugMode(enabled) {
        if (this.wasm) {
          this.wasm._pseudo_set_debug_mode(enabled ? 1 : 0);
        }
      }

      createSnapshot() {
        if (!this.wasm) return -1;
        return this.wasm._pseudo_create_snapshot();
      }

      restoreSnapshot(id) {
        if (!this.wasm) return false;
        return this.wasm._pseudo_restore_snapshot(id) === 1;
      }

      // Execute one statement and save state
      stepInto() {
        if (!this.wasm || this.isProgramDone) return { done: true };

        // If we stepped back and are now stepping forward through history
        if (this.currentHistoryIndex < this.stateHistory.length - 1) {
          this.currentHistoryIndex++;
          const state = this.stateHistory[this.currentHistoryIndex];
          this.restoreSnapshot(state.snapshotId);
          return {
            done: false,
            line: state.line,
            variables: state.variables,
            changedVars: this.getChangedVars(state.variables)
          };
        }

        // Create snapshot before executing
        const snapshotId = this.createSnapshot();
        const line = this.getNextLine();

        // Execute one step
        const result = this.wasm._pseudo_step();

        // Get state after execution
        const variables = this.getVariables();
        const changedVars = this.getChangedVars(variables);
        const conditionInfo = this.getConditionInfo();

        // Save to history
        const stateEntry = { snapshotId, line, variables, conditionInfo };

        if (this.stateHistory.length >= this.maxHistory) {
          this.stateHistory.shift();
        }
        this.stateHistory.push(stateEntry);
        this.currentHistoryIndex = this.stateHistory.length - 1;

        // Update previous variables for change detection
        this.previousVariables = {};
        for (const v of variables) {
          this.previousVariables[v.name] = v.value;
        }

        // 0 = EXEC_CONTINUE, 1 = EXEC_DONE, 2 = EXEC_NEEDS_INPUT, 3 = EXEC_ERROR
        const isDone = result === 1;
        const needsInput = result === 2;
        const hasError = result === 3;

        if (isDone) {
          this.isProgramDone = true;
        }

        return {
          done: isDone,
          needsInput,
          hasError,
          line: this.getNextLine(),
          variables,
          changedVars,
          conditionInfo
        };
      }

      // Execute and step over control structures
      stepOver() {
        if (!this.wasm || this.isProgramDone) return { done: true };

        // If we stepped back and are now stepping forward through history
        if (this.currentHistoryIndex < this.stateHistory.length - 1) {
          return this.stepInto(); // Just use normal step through history
        }

        // Create snapshot before executing
        const snapshotId = this.createSnapshot();
        const line = this.getNextLine();

        // Execute step over (runs until same or lower depth)
        const result = this.wasm._pseudo_step_over();

        // Get state after execution
        const variables = this.getVariables();
        const changedVars = this.getChangedVars(variables);
        const conditionInfo = this.getConditionInfo();

        // Save to history
        const stateEntry = { snapshotId, line, variables, conditionInfo };

        if (this.stateHistory.length >= this.maxHistory) {
          this.stateHistory.shift();
        }
        this.stateHistory.push(stateEntry);
        this.currentHistoryIndex = this.stateHistory.length - 1;

        // Update previous variables for change detection
        this.previousVariables = {};
        for (const v of variables) {
          this.previousVariables[v.name] = v.value;
        }

        // Check execution state
        const isDone = result === 1;
        const needsInput = result === 2;
        const hasError = result === 3;

        if (isDone) {
          this.isProgramDone = true;
        }

        return {
          done: isDone,
          needsInput,
          hasError,
          line: this.getNextLine(),
          variables,
          changedVars,
          conditionInfo
        };
      }

      // Go back one step
      stepBack() {
        if (this.currentHistoryIndex <= 0) return null;

        this.currentHistoryIndex--;
        const state = this.stateHistory[this.currentHistoryIndex];
        this.restoreSnapshot(state.snapshotId);

        // Reset program done flag since we went back
        this.isProgramDone = false;

        return {
          line: state.line,
          variables: state.variables,
          changedVars: [],
          conditionInfo: state.conditionInfo || { valid: false }
        };
      }

      // Go forward one step (if we have history)
      stepForward() {
        if (this.currentHistoryIndex >= this.stateHistory.length - 1) {
          // No more history, execute new step
          return this.stepInto();
        }

        this.currentHistoryIndex++;
        const state = this.stateHistory[this.currentHistoryIndex];
        this.restoreSnapshot(state.snapshotId);

        return {
          done: false,
          line: state.line,
          variables: state.variables,
          changedVars: []
        };
      }

      // Jump to specific point in history
      jumpToStep(index) {
        if (index < 0 || index >= this.stateHistory.length) return null;

        this.currentHistoryIndex = index;
        const state = this.stateHistory[index];
        this.restoreSnapshot(state.snapshotId);

        // Reset program done flag if we jumped back
        if (index < this.stateHistory.length - 1) {
          this.isProgramDone = false;
        }

        return {
          line: state.line,
          variables: state.variables,
          changedVars: []
        };
      }

      getChangedVars(currentVars) {
        const changed = [];
        for (const v of currentVars) {
          if (this.previousVariables[v.name] !== v.value) {
            changed.push(v.name);
          }
        }
        return changed;
      }

      canStepBack() {
        return this.currentHistoryIndex > 0;
      }

      canStepForward() {
        return !this.isProgramDone || this.currentHistoryIndex < this.stateHistory.length - 1;
      }

      getCurrentStep() {
        return this.currentHistoryIndex + 1;
      }

      getTotalSteps() {
        return this.stateHistory.length;
      }
    }
