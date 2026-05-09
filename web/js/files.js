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
