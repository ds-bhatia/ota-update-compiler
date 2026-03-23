const sampleSelect = document.getElementById('sampleSelect');
const loadSampleBtn = document.getElementById('loadSampleBtn');
const compileBtn = document.getElementById('compileBtn');
const codeInput = document.getElementById('codeInput');
const filenameInput = document.getElementById('filenameInput');
const engineStatus = document.getElementById('engineStatus');
const resultBadge = document.getElementById('resultBadge');
const messageBox = document.getElementById('messageBox');
const violationsList = document.getElementById('violationsList');
const hintsList = document.getElementById('hintsList');
const rawOutput = document.getElementById('rawOutput');

async function fetchSamples() {
  const res = await fetch('/api/samples');
  const data = await res.json();
  sampleSelect.innerHTML = '';

  for (const name of data.samples) {
    const opt = document.createElement('option');
    opt.value = name;
    opt.textContent = name;
    sampleSelect.appendChild(opt);
  }

  if (data.samples.length > 0) {
    sampleSelect.value = 'secure.c';
  }
}

async function loadSample() {
  if (!sampleSelect.value) return;
  const res = await fetch(`/api/samples/${encodeURIComponent(sampleSelect.value)}`);
  const data = await res.json();
  if (data.error) {
    setMessage(data.error, 'bad');
    return;
  }
  codeInput.value = data.code;
  filenameInput.value = data.name;
  setMessage(`Loaded ${data.name}`, 'neutral');
}

function setBadge(state) {
  resultBadge.className = 'badge';
  if (state === 'passed') {
    resultBadge.classList.add('passed');
    resultBadge.textContent = 'Passed';
  } else if (state === 'failed') {
    resultBadge.classList.add('failed');
    resultBadge.textContent = 'Blocked';
  } else {
    resultBadge.classList.add('idle');
    resultBadge.textContent = 'Idle';
  }
}

function setMessage(text, mode) {
  messageBox.className = `message ${mode}`;
  messageBox.textContent = text;
}

function renderList(listEl, items, formatItem) {
  listEl.innerHTML = '';
  if (!items || items.length === 0) {
    const li = document.createElement('li');
    li.textContent = 'None';
    listEl.appendChild(li);
    return;
  }

  for (const item of items) {
    const li = document.createElement('li');
    li.textContent = formatItem(item);
    listEl.appendChild(li);
  }
}

async function runCompile() {
  compileBtn.disabled = true;
  compileBtn.textContent = 'Compiling...';
  engineStatus.textContent = 'Engine: compiling';

  try {
    const res = await fetch('/api/compile', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        filename: filenameInput.value.trim() || 'firmware_demo.c',
        code: codeInput.value,
      }),
    });

    const data = await res.json();
    if (!res.ok) {
      setBadge('failed');
      setMessage(data.error || 'Compile request failed.', 'bad');
      return;
    }

    if (data.ok) {
      setBadge('passed');
      setMessage(data.message, 'good');
    } else {
      setBadge('failed');
      setMessage(data.message, 'bad');
    }

    renderList(violationsList, data.violations, (v) => v);
    renderList(hintsList, data.hints, (h) => {
      const col = h.column ? `:${h.column}` : '';
      return `L${h.line}${col} ${h.label}`;
    });
    rawOutput.textContent = data.rawOutput || '(no output)';
  } catch (err) {
    setBadge('failed');
    setMessage(`Unexpected error: ${err.message}`, 'bad');
  } finally {
    compileBtn.disabled = false;
    compileBtn.textContent = 'Run secure-clang';
    engineStatus.textContent = 'Engine: ready';
  }
}

loadSampleBtn.addEventListener('click', loadSample);
compileBtn.addEventListener('click', runCompile);

(async function init() {
  await fetchSamples();
  await loadSample();
})();
