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
