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
