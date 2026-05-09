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
