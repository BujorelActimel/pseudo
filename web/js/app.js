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
