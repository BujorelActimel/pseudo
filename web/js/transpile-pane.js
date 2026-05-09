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
