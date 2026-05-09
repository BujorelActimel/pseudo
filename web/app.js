    // Set version from version.js
    document.getElementById('version-label').textContent = VERSION;

    // Tooltip
    const tooltipEl = document.createElement('div');
    tooltipEl.className = 'tooltip-popup';
    document.body.appendChild(tooltipEl);

    document.addEventListener('mouseover', (e) => {
      const target = e.target.closest('[data-tooltip]');
      if (!target) return;
      tooltipEl.textContent = target.getAttribute('data-tooltip');
      tooltipEl.style.color  = '';
      tooltipEl.style.opacity = '1';
      const r = target.getBoundingClientRect();
      const tr = tooltipEl.getBoundingClientRect();
      let top = r.bottom + 7;
      let left = r.left + r.width / 2 - tr.width / 2;
      if (top + tr.height > window.innerHeight - 4) top = r.top - tr.height - 7;
      if (left < 4) left = 4;
      if (left + tr.width > window.innerWidth - 4) left = window.innerWidth - tr.width - 4;
      tooltipEl.style.top = top + 'px';
      tooltipEl.style.left = left + 'px';
    });

    document.addEventListener('mouseout', (e) => {
      if (e.target.closest('[data-tooltip]')) tooltipEl.style.opacity = '0';
    });

    // Initialize WASM module
    let Module = null;
    let wasmReady = false;

    PseudoModule().then(mod => {
      Module = mod;
      wasmReady = true;
      console.log('WASM module loaded');
    }).catch(err => {
      console.error('Failed to load WASM module:', err);
    });

    // State
    let editor = null;
    let pseudoInterpreter = null;
    let isRunning = false;
    let isDebugging = false;
    let isWaitingForInput = false;
    let inputCallback = null;
    let stopRequested = false;
    let editorWidth = 60; // percentage

    // File system state
    let files = [];
    let activeFileId = null;
    let fileCounter = 0;

    // BAC mode state
    let bacModeEnabled = false;
    let sfDecorations = [];

    // LocalStorage persistence
    const STORAGE_KEY = 'pseudo_editor_state';

    function saveState() {
      // Save current editor content to active file first
      if (activeFileId && editor) {
        const currentFile = files.find(f => f.id === activeFileId);
        if (currentFile) {
          currentFile.content = editor.getValue();
        }
      }
      const state = {
        files: files,
        activeFileId: activeFileId,
        editorWidth: editorWidth,
        fileCounter: fileCounter,
        bacModeEnabled: bacModeEnabled
      };
      try {
        localStorage.setItem(STORAGE_KEY, JSON.stringify(state));
      } catch (e) {
        console.warn('Failed to save state:', e);
      }
    }

    function loadState() {
      const saved = localStorage.getItem(STORAGE_KEY);
      if (saved) {
        try {
          return JSON.parse(saved);
        } catch (e) {
          console.warn('Failed to parse saved state:', e);
        }
      }
      return null;
    }

    // Debounced save for content changes
    let saveTimeout = null;
    function debouncedSave() {
      if (saveTimeout) clearTimeout(saveTimeout);
      saveTimeout = setTimeout(saveState, 1000);
    }

    // DOM Elements
    const btnRun = document.getElementById('btn-run');
    const btnDebug = document.getElementById('btn-debug');
    const btnStop = document.getElementById('btn-stop');
    const btnClear = document.getElementById('btn-clear');
    const debugIndicator = document.getElementById('debug-indicator');
    const consoleOutput = document.getElementById('console-output');
    const editorPanel = document.getElementById('editor-panel');
    const consolePanel = document.getElementById('console-panel');
    const resizeHandle = document.getElementById('resize-handle');
    const fileTabs = document.getElementById('file-tabs');
    const addFileBtn = document.getElementById('add-file-btn');
    const tabsScrollLeft = document.getElementById('tabs-scroll-left');
    const tabsScrollRight = document.getElementById('tabs-scroll-right');
    const btnBacMode = document.getElementById('btn-bac-mode');
    const btnLint = document.getElementById('btn-lint');

    // Debug UI elements
    const debugToolbarControls = document.getElementById('debug-toolbar-controls');
    const debugStepInto = document.getElementById('debug-step-into');
    const debugContinue = document.getElementById('debug-continue');
    const debugStop = document.getElementById('debug-stop');
    const variablesPanel = document.getElementById('variables-panel');
    const variablesStep = document.getElementById('variables-step');
    const variablesContent = document.getElementById('variables-content');
    const conditionDisplay = document.getElementById('condition-display');

    // Debugger state
    let debugger_ = null;
    let debugLineDecorations = [];

    // Default code (Romanian pseudocode syntax) - demo with nested blocks
    const defaultCode = `# Exemplu de pseudocod cu blocuri imbricate

citeste m, n

daca m < n atunci
    repeta
        x <- m
        y <- n
        n <- n - 1
        repeta
            daca x > y atunci
                x <- x - y
            altfel
                y <- y - x
            sf
        pana cand y = 0
    pana cand x != 1
sf

scrie n + 1`;

    // Gruvbox Dark theme for Monaco
    const gruvboxTheme = {
      base: 'vs-dark',
      inherit: true,
      rules: [
        { token: '', foreground: 'ebdbb2', background: '1d2021' },
        { token: 'comment', foreground: '928374', fontStyle: 'italic' },
        { token: 'keyword', foreground: 'fb4934' },
        { token: 'keyword.control', foreground: 'fb4934' },
        { token: 'keyword.flow', foreground: 'fb4934' },
        { token: 'keyword.function', foreground: 'fabd2f' },
        { token: 'keyword.operator', foreground: 'fe8019' },
        { token: 'string', foreground: 'b8bb26' },
        { token: 'number', foreground: 'd3869b' },
        { token: 'operator', foreground: '8ec07c' },
        { token: 'delimiter', foreground: 'ebdbb2' },
        { token: 'delimiter.bracket', foreground: 'fe8019' },
        { token: 'delimiter.parenthesis', foreground: 'ebdbb2' },
        { token: 'variable', foreground: '83a598' },
        { token: 'function', foreground: 'fabd2f' },
        { token: 'type', foreground: 'fabd2f' },
        { token: 'constant', foreground: 'd3869b' },
      ],
      colors: {
        'editor.background': '#1d2021',
        'editor.foreground': '#ebdbb2',
        'editor.lineHighlightBackground': '#32302f',
        'editor.selectionBackground': '#504945',
        'editor.inactiveSelectionBackground': '#3c3836',
        'editorCursor.foreground': '#ebdbb2',
        'editorWhitespace.foreground': '#504945',
        'editorLineNumber.foreground': '#665c54',
        'editorLineNumber.activeForeground': '#ebdbb2',
        'editorIndentGuide.background': '#3c3836',
        'editorIndentGuide.activeBackground': '#504945',
        'editor.selectionHighlightBackground': '#689d6a40',
        'editorBracketMatch.background': '#68986640',
        'editorBracketMatch.border': '#689866',
        'scrollbarSlider.background': '#50494580',
        'scrollbarSlider.hoverBackground': '#665c5480',
        'scrollbarSlider.activeBackground': '#7c6f6480',
        'diffEditor.insertedTextBackground': '#98971a30',
        'diffEditor.removedTextBackground': '#cc241d30',
        'diffEditor.insertedLineBackground': '#98971a18',
        'diffEditor.removedLineBackground': '#cc241d18',
        'diffEditor.diagonalFill': '#3c383640',
        'diffEditorGutter.insertedLineBackground': '#98971a40',
        'diffEditorGutter.removedLineBackground': '#cc241d40',
      }
    };

    // Initialize Monaco Editor
    require.config({ paths: { vs: 'https://cdnjs.cloudflare.com/ajax/libs/monaco-editor/0.45.0/min/vs' } });

    require(['vs/editor/editor.main'], function () {
      // Register Pseudocode language (Romanian syntax)
      monaco.languages.register({ id: 'pseudocode' });

      monaco.languages.setMonarchTokensProvider('pseudocode', {
        ignoreCase: false,
        controlKeywords: ['daca', 'pentru', 'cat', 'repeta'],
        flowKeywords: ['atunci', 'altfel', 'executa', 'timp', 'pana', 'cand', 'sf'],
        builtinFunctions: ['citeste', 'scrie'],
        logicalOperators: ['SAU', 'sau', 'SI', 'si', 'NOT', 'not'],
        tokenizer: {
          root: [
            [/#.*$/, 'comment'],
            [/\/\/.*$/, 'comment'],
            [/"([^"\\]|\\.)*"/, 'string'],
            [/'([^'\\]|\\.)*'/, 'string'],
            [/\b\d+(\.\d+)?\b/, 'number'],
            [/<->|<-\x2D>/, 'operator'],
            [/<-/, 'operator'],
            [/==|!=|<=|>=|<|>|=/, 'operator'],
            [/[+\-*/%√]/, 'operator'],
            [/\[|\]/, 'delimiter.bracket'],
            [/[()]/, 'delimiter.parenthesis'],
            [/[;,]/, 'delimiter'],
            [/[a-zA-Z_][a-zA-Z0-9_]*/, {
              cases: {
                '@controlKeywords': 'keyword.control',
                '@flowKeywords': 'keyword.flow',
                '@builtinFunctions': 'keyword.function',
                '@logicalOperators': 'keyword.operator',
                '@default': 'variable'
              }
            }],
          ]
        }
      });

      monaco.languages.setLanguageConfiguration('pseudocode', {
        comments: {
          lineComment: '#',
        },
        brackets: [
          ['[', ']'],
          ['(', ')'],
        ],
        autoClosingPairs: [
          { open: '[', close: ']' },
          { open: '(', close: ')' },
          { open: '"', close: '"' },
          { open: "'", close: "'" },
        ],
        surroundingPairs: [
          { open: '[', close: ']' },
          { open: '(', close: ')' },
          { open: '"', close: '"' },
          { open: "'", close: "'" },
        ],
      });

      // Define theme
      monaco.editor.defineTheme('gruvbox-dark', gruvboxTheme);

      // Create editor
      editor = monaco.editor.create(document.getElementById('editor-container'), {
        value: '',
        language: 'pseudocode',
        theme: 'gruvbox-dark',
        fontFamily: "'JetBrains Mono', monospace",
        fontSize: 14,
        lineHeight: 1.6,
        padding: { top: 16 },
        minimap: { enabled: false },
        scrollBeyondLastLine: false,
        renderLineHighlight: 'line',
        cursorBlinking: 'smooth',
        cursorSmoothCaretAnimation: 'on',
        smoothScrolling: true,
        wordWrap: 'on',
        automaticLayout: true,
        lineNumbers: 'on',
        glyphMargin: true,
        folding: false,
        lineDecorationsWidth: 8,
        lineNumbersMinChars: 3,
        guides: {
          indentation: true,
          highlightActiveIndentation: true
        },
        overviewRulerBorder: false,
        hideCursorInOverviewRuler: true,
        overviewRulerLanes: 0,
        scrollbar: {
          vertical: 'auto',
          horizontal: 'hidden',
          verticalScrollbarSize: 10,
          horizontalScrollbarSize: 0,
        }
      });

      // Load saved state or initialize with default
      const savedState = loadState();
      if (savedState && savedState.files && savedState.files.length > 0) {
        files = savedState.files;
        activeFileId = savedState.activeFileId;
        fileCounter = savedState.fileCounter || files.length;
        if (savedState.editorWidth) {
          editorWidth = savedState.editorWidth;
        }
        if (savedState.bacModeEnabled !== undefined) {
          bacModeEnabled = savedState.bacModeEnabled;
        }

        renderTabs();
        const activeFile = files.find(f => f.id === activeFileId);
        if (activeFile) {
          editor.setValue(activeFile.content);
        } else if (files.length > 0) {
          activeFileId = files[0].id;
          editor.setValue(files[0].content);
          renderTabs();
        }
      } else {
        createFile('demo.pseudo', defaultCode);
      }

      // Set initial panel widths
      updatePanelWidths();

      // Apply BAC mode UI state
      updateBacModeUI();
      setTimeout(() => { updateAllDecorations(); updateLoopGlyphs(); }, 100);

      // Listen for content changes to auto-save and update decorations
      editor.onDidChangeModelContent(() => {
        debouncedSave();
        if (bacModeEnabled) {
          clearTimeout(editor._decorationUpdateTimeout);
          editor._decorationUpdateTimeout = setTimeout(updateAllDecorations, 150);
        }
        clearTimeout(editor._loopGlyphTimeout);
        editor._loopGlyphTimeout = setTimeout(updateLoopGlyphs, 300);
      });

      // Save state before page unload
      window.addEventListener('beforeunload', saveState);
    });

    // File management functions
    function createFile(name = null, content = '') {
      fileCounter++;
      const id = `file-${fileCounter}`;
      
      if (!name) {
        name = `untitled-${fileCounter}.pseudo`;
      }

      const file = { id, name, content };
      files.push(file);

      renderTabs();
      switchToFile(id);
      saveState();

      return file;
    }

    function deleteFile(id) {
      if (files.length <= 1) return;
      
      const index = files.findIndex(f => f.id === id);
      if (index === -1) return;

      if (activeFileId === id && editor) {
        files[index].content = editor.getValue();
      }

      files.splice(index, 1);

      if (activeFileId === id) {
        const newIndex = Math.min(index, files.length - 1);
        switchToFile(files[newIndex].id);
      }

      renderTabs();
      saveState();
    }

    function switchToFile(id) {
      if (activeFileId === id) return;

      if (activeFileId && editor) {
        const currentFile = files.find(f => f.id === activeFileId);
        if (currentFile) {
          currentFile.content = editor.getValue();
        }
      }

      activeFileId = id;
      const file = files.find(f => f.id === id);

      if (file && editor) {
        editor.setValue(file.content);
        editor.focus();
      }

      renderTabs();
      saveState();
    }

    function renameFile(id, newName) {
      const file = files.find(f => f.id === id);
      if (file) {
        file.name = newName.endsWith('.pseudo') ? newName : newName + '.pseudo';
        renderTabs();
        saveState();
      }
    }

    function renderTabs() {
      fileTabs.innerHTML = '';
      
      files.forEach(file => {
        const tab = document.createElement('div');
        tab.className = `file-tab ${file.id === activeFileId ? 'active' : ''}`;
        
        const nameSpan = document.createElement('span');
        nameSpan.className = 'file-tab-name';
        nameSpan.textContent = file.name;
        
        const closeBtn = document.createElement('button');
        closeBtn.className = 'file-tab-close';
        closeBtn.setAttribute('data-tooltip', 'Închide fișierul');
        closeBtn.innerHTML = `<svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M18 6L6 18M6 6l12 12"></path></svg>`;
        
        tab.appendChild(nameSpan);
        tab.appendChild(closeBtn);

        tab.addEventListener('click', (e) => {
          if (!e.target.closest('.file-tab-close')) {
            switchToFile(file.id);
          }
        });

        nameSpan.addEventListener('dblclick', (e) => {
          e.preventDefault();
          e.stopPropagation();
          
          const input = document.createElement('input');
          input.type = 'text';
          input.value = file.name.replace('.pseudo', '');
          input.className = 'file-tab-input';
          input.style.cssText = `
            background: var(--bg-hard);
            border: 1px solid var(--blue);
            border-radius: 2px;
            color: var(--fg);
            font-family: inherit;
            font-size: inherit;
            padding: 0 4px;
            width: 80px;
            outline: none;
          `;
          
          nameSpan.style.display = 'none';
          tab.insertBefore(input, closeBtn);
          input.focus();
          input.select();

          let finished = false;
          const finishRename = () => {
            if (finished) return;
            finished = true;
            const newName = input.value.trim() || 'untitled';
            renameFile(file.id, newName);
          };

          input.addEventListener('blur', finishRename);
          input.addEventListener('keydown', (e) => {
            if (e.key === 'Enter') {
              e.preventDefault();
              finishRename();
            } else if (e.key === 'Escape') {
              finished = true;
              renderTabs();
            }
          });
        });

        closeBtn.addEventListener('click', (e) => {
          e.stopPropagation();
          deleteFile(file.id);
        });

        fileTabs.appendChild(tab);
      });
    }

    addFileBtn.addEventListener('click', () => {
      createFile();
    });

    function updateScrollButtons() {
      const hasOverflow = fileTabs.scrollWidth > fileTabs.clientWidth;
      const canScrollLeft = fileTabs.scrollLeft > 0;
      const canScrollRight = fileTabs.scrollLeft < fileTabs.scrollWidth - fileTabs.clientWidth - 1;

      tabsScrollLeft.classList.toggle('visible', hasOverflow && canScrollLeft);
      tabsScrollRight.classList.toggle('visible', hasOverflow && canScrollRight);
    }

    tabsScrollLeft.addEventListener('click', () => {
      fileTabs.scrollLeft -= 100;
      setTimeout(updateScrollButtons, 150);
    });

    tabsScrollRight.addEventListener('click', () => {
      fileTabs.scrollLeft += 100;
      setTimeout(updateScrollButtons, 150);
    });

    fileTabs.addEventListener('scroll', updateScrollButtons);

    // BAC Mode toggle functionality
    function toggleBacMode() {
      bacModeEnabled = !bacModeEnabled;
      updateBacModeUI();
      updateAllDecorations();
      saveState();
    }

    function updateBacModeUI() {
      const editorContainer = document.getElementById('editor-container');
      btnBacMode.classList.toggle('active', bacModeEnabled);
      editorContainer.classList.toggle('bac-mode-enabled', bacModeEnabled);

      if (editor) {
        editor.updateOptions({
          fontLigatures: bacModeEnabled,
          // Keep indentation guides enabled - we only modify 'sf' lines
          guides: {
            indentation: true,
            highlightActiveIndentation: true
          }
        });
      }
    }

    function updateAllDecorations() {
      if (!editor) return;

      // Clear existing decorations
      sfDecorations = editor.deltaDecorations(sfDecorations, []);

      if (!bacModeEnabled) return;

      const model = editor.getModel();
      if (!model) return;

      const text = model.getValue();
      const lines = text.split('\n');
      const newDecorations = [];

      // Process each line for 'sf' replacement
      for (let lineNum = 0; lineNum < lines.length; lineNum++) {
        const line = lines[lineNum];

        // Find 'sf' keyword on this line
        const sfMatch = line.match(/^(\s*)sf\b/i);
        if (sfMatch) {
          const indent = sfMatch[1].length; // Number of leading spaces
          const sfStartCol = indent + 1; // 1-based column where 'sf' starts
          const sfEndCol = sfStartCol + 2; // 'sf' is 2 characters

          // Hide the 'sf' text and add └■ before it
          newDecorations.push({
            range: new monaco.Range(lineNum + 1, sfStartCol, lineNum + 1, sfEndCol),
            options: {
              inlineClassName: 'sf-replacement',
              beforeContentClassName: 'sf-end-marker'
            }
          });
        }
      }

      sfDecorations = editor.deltaDecorations([], newDecorations);
    }

    btnBacMode.addEventListener('click', toggleBacMode);

    // Lint function
    function lintCode() {
      const activeFile = files.find(f => f.id === activeFileId);
      if (!activeFile) return;

      const source = editor.getValue();
      const sourcePtr = Module.allocateUTF8(source);
      const resultPtr = Module._pseudo_lint(sourcePtr);
      Module._free(sourcePtr);

      if (resultPtr) {
        const linted = Module.UTF8ToString(resultPtr);
        Module._pseudo_free_output(resultPtr);

        if (linted !== source) {
          // Save cursor position
          const position = editor.getPosition();

          // Update editor content
          editor.setValue(linted);

          // Restore cursor position
          if (position) {
            editor.setPosition(position);
            editor.revealPositionInCenter(position);
          }

          // Update file content
          activeFile.content = linted;
          saveState();

          appendToConsole('Lint: cod normalizat.', 'system');
        } else {
          appendToConsole('Lint: nicio modificare necesară.', 'system');
        }
      }
    }

    btnLint.addEventListener('click', lintCode);

    // Resize functionality
    let isDragging = false;

    resizeHandle.addEventListener('mousedown', (e) => {
      isDragging = true;
      resizeHandle.classList.add('dragging');
      document.body.style.cursor = 'col-resize';
      document.body.style.userSelect = 'none';
    });

    document.addEventListener('mousemove', (e) => {
      if (!isDragging) return;
      
      const containerWidth = document.querySelector('.main').offsetWidth;
      const newWidth = (e.clientX / containerWidth) * 100;
      
      editorWidth = Math.max(20, Math.min(80, newWidth));
      updatePanelWidths();
    });

    document.addEventListener('mouseup', () => {
      if (isDragging) {
        isDragging = false;
        resizeHandle.classList.remove('dragging');
        document.body.style.cursor = '';
        document.body.style.userSelect = '';
        saveState();
      }
    });

    function updatePanelWidths() {
      editorPanel.style.width = `${editorWidth}%`;
      consolePanel.style.width = `${100 - editorWidth}%`;
    }

    // Console functions
    let outputBuffer = [];
    let flushScheduled = false;

    function clearConsole() {
      outputBuffer = [];
      consoleOutput.innerHTML = '<div class="console-placeholder">$ gata</div>';
    }

    function flushOutputBuffer() {
      if (outputBuffer.length === 0) {
        flushScheduled = false;
        return;
      }

      const placeholder = consoleOutput.querySelector('.console-placeholder');
      if (placeholder) placeholder.remove();

      const fragment = document.createDocumentFragment();
      for (const item of outputBuffer) {
        const line = document.createElement('div');
        line.className = `console-line ${item.type}`;
        line.textContent = item.content;
        fragment.appendChild(line);
      }
      outputBuffer = [];

      const existingInput = consoleOutput.querySelector('.input-line');
      if (existingInput) {
        consoleOutput.insertBefore(fragment, existingInput);
      } else {
        consoleOutput.appendChild(fragment);
      }
      consoleOutput.scrollTop = consoleOutput.scrollHeight;
      flushScheduled = false;
    }

    function appendToConsole(content, type = 'output') {
      outputBuffer.push({ content, type });

      if (!flushScheduled) {
        flushScheduled = true;
        requestAnimationFrame(flushOutputBuffer);
      }
    }

    function showInputPrompt() {
      flushOutputBuffer();

      const placeholder = consoleOutput.querySelector('.console-placeholder');
      if (placeholder) placeholder.remove();

      const inputLine = document.createElement('div');
      inputLine.className = 'input-line';
      inputLine.innerHTML = `
        <span class="input-prompt">></span>
        <input type="text" class="console-input" autofocus>
        <span class="cursor-blink">_</span>
      `;
      consoleOutput.appendChild(inputLine);

      const input = inputLine.querySelector('.console-input');
      input.focus();

      input.addEventListener('keydown', (e) => {
        if (e.key === 'Enter') {
          const value = input.value;
          inputLine.remove();
          appendToConsole(value, 'input');
          isWaitingForInput = false;
          if (inputCallback) {
            const cb = inputCallback;
            inputCallback = null;
            cb(value);
          }
        }
      });

      consoleOutput.scrollTop = consoleOutput.scrollHeight;
    }

    consoleOutput.addEventListener('click', () => {
      const input = consoleOutput.querySelector('.console-input');
      if (input) input.focus();
    });

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

        // Check execution state
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

    // ── Transpile split-view ───────────────────────────────────────────────

    const langExtMap    = { c: '.c', cpp: '.cpp', pascal: '.pas' };
    const monacoLangMap = { c: 'c', cpp: 'cpp', pascal: 'pascal' };
    const langLabels    = { c: 'C', cpp: 'C++', pascal: 'Pascal' };

    let currentTranspileLang = 'c';
    let transpileEditor      = null;
    let transpileVisible     = false;

    function getActiveFilename() {
      const f = files.find(f => f.id === activeFileId);
      return f ? f.name : 'output';
    }

    // ── Split-button: language picker ──────────────────────────────────────
    const btnTranspile      = document.getElementById('btn-transpile');
    const btnTranspileLang  = document.getElementById('btn-transpile-lang');
    const langDropdown      = document.getElementById('lang-dropdown');
    const transpileLangLabel = document.getElementById('transpile-lang-label');

    function updateLangDropdown() {
      document.querySelectorAll('.lang-option').forEach(btn => {
        btn.classList.toggle('active', btn.dataset.lang === currentTranspileLang);
      });
      transpileLangLabel.textContent = langLabels[currentTranspileLang];
    }

    btnTranspileLang.addEventListener('click', (e) => {
      e.stopPropagation();
      langDropdown.classList.toggle('open');
    });

    document.querySelectorAll('.lang-option').forEach(btn => {
      btn.addEventListener('click', () => {
        currentTranspileLang = btn.dataset.lang;
        updateLangDropdown();
        langDropdown.classList.remove('open');
      });
    });

    document.addEventListener('click', () => langDropdown.classList.remove('open'));

    updateLangDropdown();

    // ── Second Monaco instance (created on first transpile) ─────────────────
    function getOrCreateTranspileEditor() {
      if (transpileEditor) return transpileEditor;
      transpileEditor = monaco.editor.create(
        document.getElementById('transpile-editor-container'), {
          value: '',
          language: 'c',
          theme: 'gruvbox-dark',
          readOnly: true,
          fontFamily: "'JetBrains Mono', monospace",
          fontSize: 14,
          lineHeight: 1.6,
          padding: { top: 16 },
          minimap: { enabled: false },
          scrollBeyondLastLine: false,
          renderLineHighlight: 'line',
          wordWrap: 'on',
          automaticLayout: true,
          lineNumbers: 'on',
          glyphMargin: false,
          folding: false,
          lineDecorationsWidth: 8,
          lineNumbersMinChars: 3,
          overviewRulerBorder: false,
          hideCursorInOverviewRuler: true,
          overviewRulerLanes: 0,
          scrollbar: { vertical: 'auto', horizontal: 'auto', verticalScrollbarSize: 10 },
        }
      );
      return transpileEditor;
    }

    // ── Show / hide pane ───────────────────────────────────────────────────
    function showTranspilePane() {
      if (transpileVisible) return;
      transpileVisible = true;
      document.getElementById('transpile-pane').style.display = 'flex';
      document.getElementById('transpile-divider').style.display = 'block';
      // Hide console while split-view is open — too cramped otherwise
      consolePanel.style.display = 'none';
      resizeHandle.style.display = 'none';
      editorPanel.style.width = '100%';
      editor.layout();
    }

    function hideTranspilePane() {
      if (!transpileVisible) return;
      transpileVisible = false;
      document.getElementById('transpile-pane').style.display = 'none';
      document.getElementById('transpile-divider').style.display = 'none';
      // Restore console
      consolePanel.style.display = '';
      resizeHandle.style.display = '';
      updatePanelWidths();
      editor.layout();
    }

    // ── Main transpile action ──────────────────────────────────────────────
    function transpileCode() {
      if (!wasmReady || !Module || !Module._pseudo_transpile) {
        appendToConsole('Modulul nu este încărcat. Așteptați...', 'error');
        return;
      }
      const source  = editor.getValue();
      const lang    = currentTranspileLang;
      const srcPtr  = Module.allocateUTF8(source);
      const langPtr = Module.allocateUTF8(lang);
      const resPtr  = Module._pseudo_transpile(srcPtr, langPtr);
      Module._free(srcPtr);
      Module._free(langPtr);

      if (!resPtr) {
        appendToConsole('Transpilare eșuată.', 'error');
        return;
      }
      const result = Module.UTF8ToString(resPtr);
      Module._pseudo_free_output(resPtr);

      const te = getOrCreateTranspileEditor();
      monaco.editor.setModelLanguage(te.getModel(), monacoLangMap[lang]);
      te.setValue(result);

      const baseName = getActiveFilename().replace(/\.pseudo$/, '');
      document.getElementById('transpile-pane-title').textContent = baseName + langExtMap[lang];

      showTranspilePane();
    }

    btnTranspile.addEventListener('click', transpileCode);

    document.getElementById('btn-transpile-close').addEventListener('click', hideTranspilePane);

    document.getElementById('btn-transpile-copy').addEventListener('click', () => {
      if (!transpileEditor) return;
      const btn = document.getElementById('btn-transpile-copy');
      navigator.clipboard.writeText(transpileEditor.getValue())
        .then(() => {
          const r = btn.getBoundingClientRect();
          tooltipEl.textContent   = 'Copiat!';
          tooltipEl.style.color   = 'var(--green)';
          tooltipEl.style.opacity = '1';
          requestAnimationFrame(() => {
            const tr = tooltipEl.getBoundingClientRect();
            let left = r.left + r.width / 2 - tr.width / 2;
            if (left < 4) left = 4;
            if (left + tr.width > window.innerWidth - 4) left = window.innerWidth - tr.width - 4;
            tooltipEl.style.top  = (r.bottom + 7) + 'px';
            tooltipEl.style.left = left + 'px';
          });
          setTimeout(() => {
            tooltipEl.style.opacity = '0';
            tooltipEl.style.color   = '';
          }, 800);
        })
        .catch(() => appendToConsole('Copierea a eșuat.', 'error'));
    });

    // ── Loop equivalence ───────────────────────────────────────────────────

    let loopDecorations = [];
    let loopMetaMap = {};   // monacoLine (1-indexed) → {type, start_line, end_line}
    const loopPopup = document.getElementById('loop-convert-popup');
    let popupSourceLine = -1;  // 0-indexed line of the active popup's loop

    function updateLoopGlyphs() {
      if (!wasmReady || !Module || !Module._pseudo_get_all_loops || !editor) return;
      const src = editor.getValue();
      const ptr = Module.allocateUTF8(src);
      const resPtr = Module._pseudo_get_all_loops(ptr);
      Module._free(ptr);
      if (!resPtr) { loopDecorations = editor.deltaDecorations(loopDecorations, []); return; }
      const json = Module.UTF8ToString(resPtr);
      Module._pseudo_free_output(resPtr);

      let loops;
      try { loops = JSON.parse(json); } catch (_) { return; }

      loopMetaMap = {};
      loopDecorations = editor.deltaDecorations(loopDecorations,
        loops.map(l => {
          const monacoLine = l.start_line + 1;
          loopMetaMap[monacoLine] = l;
          return {
            range: new monaco.Range(monacoLine, 1, monacoLine, 1),
            options: { glyphMarginClassName: 'loop-glyph' }
          };
        })
      );
    }

    function hideLoopPopup() {
      loopPopup.classList.remove('visible');
      popupSourceLine = -1;
    }

    function showLoopPopup(monacoLine, x, y) {
      const meta = loopMetaMap[monacoLine];
      if (!meta) return;
      popupSourceLine = meta.start_line;  // 0-indexed

      loopPopup.querySelectorAll('button[data-target]').forEach(btn => {
        const isCurrentType = btn.dataset.target === meta.type;
        const isForBtn = btn.dataset.target === 'for';
        btn.disabled = isCurrentType || (isForBtn && meta.type !== 'for');
      });

      // Position popup before showing so offsetWidth is accurate
      loopPopup.style.visibility = 'hidden';
      loopPopup.classList.add('visible');
      const pw = loopPopup.offsetWidth || 215;
      const ph = loopPopup.offsetHeight || 170;
      let left = x + 8;
      let top  = y;
      if (left + pw > window.innerWidth - 8) left = x - pw - 8;
      if (top + ph > window.innerHeight - 8) top = window.innerHeight - ph - 8;
      loopPopup.style.left = left + 'px';
      loopPopup.style.top  = top + 'px';
      loopPopup.style.visibility = '';
    }

    // Glyph tooltip
    const loopTooltip = document.getElementById('loop-tooltip');
    document.getElementById('editor-container').addEventListener('mousemove', e => {
      if (e.target.classList.contains('loop-glyph')) {
        loopTooltip.style.left = (e.clientX + 14) + 'px';
        loopTooltip.style.top  = (e.clientY - 6) + 'px';
        loopTooltip.style.display = 'block';
      } else {
        loopTooltip.style.display = 'none';
      }
    });

    // Detect glyph clicks via DOM class — avoids depending on monaco.editor.MouseTargetType
    document.getElementById('editor-container').addEventListener('mousedown', e => {
      if (!e.target.classList.contains('loop-glyph')) return;
      e.stopPropagation();
      loopTooltip.style.display = 'none';

      // Get line number: try Monaco API, fall back to Y-position math
      const monacoTarget = editor.getTargetAtClientPoint(e.clientX, e.clientY);
      let monacoLine = monacoTarget?.position?.lineNumber;
      if (!monacoLine) {
        const rect = editor.getContainerDomNode().getBoundingClientRect();
        const lineHeight = editor.getOption(monaco.editor.EditorOption.lineHeight);
        monacoLine = Math.floor((e.clientY - rect.top + editor.getScrollTop()) / lineHeight) + 1;
      }

      if (!monacoLine || !loopMetaMap[monacoLine]) return;
      showLoopPopup(monacoLine, e.clientX, e.clientY);
    });

    // Close popup on mousedown outside it (fires before editor changes focus)
    document.addEventListener('mousedown', e => {
      if (e.target.classList.contains('loop-glyph')) return;
      if (!loopPopup.contains(e.target)) hideLoopPopup();
    });

    loopPopup.querySelectorAll('button[data-target]').forEach(btn => {
      btn.addEventListener('click', () => {
        const target = btn.dataset.target;
        applyLoopConversion(popupSourceLine, target);
        hideLoopPopup();
      });
    });

    // ── Diff pane (two synchronized editors) ──────────────────────────────

    let diffOrigEditor   = null;
    let diffModEditor    = null;
    let diffOrigDecIds   = [];
    let diffModDecIds    = [];
    let diffVisible      = false;
    let diffModified     = null;
    let diffScrollSync   = false;

    const loopTypeLabels = {
      for:      'pentru (for)',
      while:    'cât timp (while)',
      do_while: 'execută...cât timp',
      repeat:   'repetă...până când',
    };

    function initDiffEditors() {
      if (diffOrigEditor) return;
      const sharedOpts = {
        theme: 'gruvbox-dark',
        readOnly: true,
        language: 'pseudocode',
        fontFamily: "'JetBrains Mono', monospace",
        fontSize: 14,
        lineHeight: 1.6,
        padding: { top: 16 },
        minimap: { enabled: false },
        scrollBeyondLastLine: false,
        wordWrap: 'off',
        automaticLayout: true,
        glyphMargin: false,
        folding: false,
        lineNumbers: 'on',
        lineNumbersMinChars: 3,
        overviewRulerBorder: false,
        hideCursorInOverviewRuler: true,
        overviewRulerLanes: 0,
        scrollbar: { vertical: 'auto', horizontal: 'hidden', verticalScrollbarSize: 10 },
      };
      diffOrigEditor = monaco.editor.create(document.getElementById('diff-orig-container'), sharedOpts);
      diffModEditor  = monaco.editor.create(document.getElementById('diff-mod-container'),  sharedOpts);

      diffOrigEditor.onDidScrollChange(e => {
        if (diffScrollSync) return;
        diffScrollSync = true;
        diffModEditor.setScrollTop(e.scrollTop);
        diffScrollSync = false;
      });
      diffModEditor.onDidScrollChange(e => {
        if (diffScrollSync) return;
        diffScrollSync = true;
        diffOrigEditor.setScrollTop(e.scrollTop);
        diffScrollSync = false;
      });
    }

    function computeLineDiff(origLines, modLines) {
      const m = origLines.length, n = modLines.length;
      // LCS table (bottom-up)
      const dp = Array.from({length: m + 1}, () => new Int32Array(n + 1));
      for (let i = m - 1; i >= 0; i--)
        for (let j = n - 1; j >= 0; j--)
          dp[i][j] = origLines[i] === modLines[j]
            ? dp[i+1][j+1] + 1
            : Math.max(dp[i+1][j], dp[i][j+1]);

      // Traceback — collect 0-indexed line indices that have no LCS match
      const removed = [], added = [];
      let i = 0, j = 0;
      while (i < m && j < n) {
        if (origLines[i] === modLines[j])   { i++; j++; }
        else if (dp[i+1][j] >= dp[i][j+1]) { removed.push(i++); }
        else                                { added.push(j++);   }
      }
      while (i < m) removed.push(i++);
      while (j < n) added.push(j++);
      return { removed, added };
    }

    function toDecorations(indices, className) {
      // Merge consecutive indices into range decorations
      if (!indices.length) return [];
      const decs = [];
      let s = indices[0], e = indices[0];
      for (let k = 1; k < indices.length; k++) {
        if (indices[k] === e + 1) { e = indices[k]; }
        else {
          decs.push({ range: new monaco.Range(s+1, 1, e+1, 1), options: { isWholeLine: true, className } });
          s = e = indices[k];
        }
      }
      decs.push({ range: new monaco.Range(s+1, 1, e+1, 1), options: { isWholeLine: true, className } });
      return decs;
    }

    function showDiffPane(originalCode, modifiedCode, title) {
      diffModified = modifiedCode;
      if (transpileVisible) hideTranspilePane();

      initDiffEditors();

      diffOrigEditor.setValue(originalCode);
      diffModEditor.setValue(modifiedCode);

      // LCS-based line diff
      const { removed, added } = computeLineDiff(
        originalCode.split('\n'), modifiedCode.split('\n')
      );
      diffOrigDecIds = diffOrigEditor.deltaDecorations(diffOrigDecIds, toDecorations(removed, 'diff-removed-line'));
      diffModDecIds  = diffModEditor.deltaDecorations(diffModDecIds,   toDecorations(added,   'diff-added-line'));

      // Scroll to first changed line
      const revealLine = (removed[0] ?? added[0] ?? 0) + 1;
      diffOrigEditor.revealLineInCenter(revealLine);
      diffModEditor.revealLineInCenter(revealLine);

      document.getElementById('diff-pane-title').textContent = title;

      if (!diffVisible) {
        diffVisible = true;
        document.getElementById('editor-container').style.display = 'none';
        document.getElementById('diff-divider').style.display = 'none';
        document.getElementById('diff-pane').style.display = 'flex';
        document.getElementById('diff-pane').style.flex = '1';
        consolePanel.style.display = 'none';
        resizeHandle.style.display = 'none';
        editorPanel.style.width = '100%';
      }
    }

    function hideDiffPane() {
      if (!diffVisible) return;
      diffVisible = false;
      document.getElementById('editor-container').style.display = '';
      document.getElementById('diff-pane').style.display = 'none';
      document.getElementById('diff-pane').style.flex = '';
      consolePanel.style.display = '';
      resizeHandle.style.display = '';
      updatePanelWidths();
      editor.layout();
    }

    document.getElementById('btn-diff-close').addEventListener('click', hideDiffPane);

    document.getElementById('btn-diff-apply').addEventListener('click', () => {
      if (diffModified !== null) editor.setValue(diffModified);
      hideDiffPane();
    });

    document.getElementById('btn-diff-newtab').addEventListener('click', () => {
      if (diffModified !== null) {
        const baseName = getActiveFilename().replace(/\.pseudo$/, '');
        const suffix = document.getElementById('diff-pane-title').textContent.match(/\((\w+)\)/)?.[1] || 'equiv';
        createFile(baseName + '_' + suffix + '.pseudo', diffModified);
      }
      hideDiffPane();
    });

    function applyLoopConversion(sourceLine0, targetType) {
      if (!wasmReady || !Module || !Module._pseudo_convert_loop) return;
      const src = editor.getValue();
      const srcPtr = Module.allocateUTF8(src);
      const tPtr   = Module.allocateUTF8(targetType);
      const resPtr = Module._pseudo_convert_loop(srcPtr, sourceLine0, 0, tPtr);
      Module._free(srcPtr);
      Module._free(tPtr);
      if (!resPtr) { appendToConsole('Conversie eșuată.', 'error'); return; }
      const result = Module.UTF8ToString(resPtr);
      Module._pseudo_free_output(resPtr);

      try {
        const err = JSON.parse(result);
        if (err.error) { appendToConsole('Eroare: ' + err.error, 'error'); return; }
      } catch (_) {}

      const label = loopTypeLabels[targetType] || targetType;
      showDiffPane(src, result, 'Echivalență → ' + label);
    }

    // Keyboard shortcuts for debugging
    document.addEventListener('keydown', (e) => {
      if (!isDebugging) return;

      if (e.key === 'F11') {
        e.preventDefault();
        debugStepIntoHandler();
      } else if (e.key === 'F5' && !e.shiftKey) {
        e.preventDefault();
        debugContinueHandler();
      } else if (e.key === 'F5' && e.shiftKey) {
        e.preventDefault();
        stopCode();
      } else if (e.key === 'Escape') {
        e.preventDefault();
        stopCode();
      }
    });
