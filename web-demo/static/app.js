const sampleSelect = document.getElementById('sampleSelect');
const loadSampleBtn = document.getElementById('loadSampleBtn');
const compileBtn = document.getElementById('compileBtn');
const demoButtons = document.getElementById('demoButtons');
const codeInput = document.getElementById('codeInput');
const filenameInput = document.getElementById('filenameInput');
const resultBadge = document.getElementById('resultBadge');
const messageBox = document.getElementById('messageBox');
const energyBox = document.getElementById('energyBox');
const energyPhases = document.getElementById('energyPhases');
const violationsList = document.getElementById('violationsList');
const hintsList = document.getElementById('hintsList');
const rawOutput = document.getElementById('rawOutput');
const historyList = document.getElementById('historyList');
const historyChart = document.getElementById('historyChart');
const exportJsonBtn = document.getElementById('exportJsonBtn');
const exportMdBtn = document.getElementById('exportMdBtn');
const themeToggle = document.getElementById('themeToggle');

const runHistory = [];
let lastCompilePayload = null;
const THEME_KEY = 'ota-demo-theme';

const demoScenarios = [
  { label: 'Secure Baseline', sample: 'secure.c', expected: 'pass' },
  { label: 'Signature Missing', sample: 'insecure_rule_signature_missing.c', expected: 'fail' },
  { label: 'Rollback Violation', sample: 'insecure_rule_rollback_missing.c', expected: 'fail' },
  { label: 'Weak Crypto', sample: 'insecure_rule_weak_md5.c', expected: 'fail' },
  { label: 'Logging Leak', sample: 'insecure_rule_logging_puts.c', expected: 'fail' },
];

function renderDemoButtons() {
  demoButtons.innerHTML = '';
  for (const s of demoScenarios) {
    const btn = document.createElement('button');
    btn.className = 'demo-btn';
    btn.type = 'button';
    btn.innerHTML = `${s.label}<em>${s.expected.toUpperCase()}</em>`;
    btn.addEventListener('click', async () => {
      sampleSelect.value = s.sample;
      await loadSample();
    });
    demoButtons.appendChild(btn);
  }
}

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

function getInitialTheme() {
  const stored = localStorage.getItem(THEME_KEY);
  if (stored === 'dark' || stored === 'light') {
    return stored;
  }
  if (window.matchMedia && window.matchMedia('(prefers-color-scheme: light)').matches) {
    return 'light';
  }
  return 'dark';
}

function applyTheme(theme) {
  document.body.setAttribute('data-theme', theme);
  localStorage.setItem(THEME_KEY, theme);
  if (themeToggle) {
    themeToggle.textContent = theme === 'dark' ? 'Switch to light mode' : 'Switch to dark mode';
  }
  drawHistoryChart();
}

function toggleTheme() {
  const current = document.body.getAttribute('data-theme') || 'dark';
  applyTheme(current === 'dark' ? 'light' : 'dark');
}

function renderEnergy(energy) {
  if (!energy || !energy.available) {
    energyBox.className = 'message neutral';
    energyBox.textContent = 'CodeCarbon metrics unavailable. Ensure codecarbon is installed in the active Python environment.';
    renderList(energyPhases, [], (x) => x);
    return;
  }

  const total = energy.total || { energy_kwh: 0, emissions_kg: 0 };
  energyBox.className = 'message good';
  energyBox.textContent = `Total energy: ${Number(total.energy_kwh).toFixed(8)} kWh | Total emissions: ${Number(total.emissions_kg).toFixed(8)} kgCO2eq`;

  renderList(energyPhases, energy.phases, (p) =>
    `${p.phase} -> ${Number(p.energy_kwh).toFixed(8)} kWh, ${Number(p.emissions_kg).toFixed(8)} kgCO2eq`
  );
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

function formatTimestamp(ts) {
  const d = new Date(ts);
  return d.toLocaleString();
}

function addHistoryEntry(entry) {
  runHistory.unshift(entry);
  if (runHistory.length > 20) {
    runHistory.pop();
  }
  renderHistory();
}

function renderHistory() {
  renderList(historyList, runHistory, (r) => {
    return `${formatTimestamp(r.timestamp)} | ${r.filename} | ${r.status.toUpperCase()} | violations=${r.violations} | energy=${r.energyKwh.toFixed(8)} kWh`;
  });
  drawHistoryChart();
}

function drawHistoryChart() {
  const canvas = historyChart;
  if (!canvas) return;
  const ctx = canvas.getContext('2d');
  if (!ctx) return;

  const width = canvas.width;
  const height = canvas.height;
  const styles = getComputedStyle(document.body);
  const axisColor = styles.getPropertyValue('--line').trim() || '#314139';
  const labelColor = styles.getPropertyValue('--muted').trim() || '#95a79a';
  const barColor = styles.getPropertyValue('--accent').trim() || '#f4b860';
  ctx.clearRect(0, 0, width, height);

  ctx.strokeStyle = axisColor;
  ctx.lineWidth = 1;
  ctx.beginPath();
  ctx.moveTo(30, 12);
  ctx.lineTo(30, height - 24);
  ctx.lineTo(width - 8, height - 24);
  ctx.stroke();

  if (runHistory.length === 0) {
    ctx.fillStyle = labelColor;
    ctx.font = '12px IBM Plex Mono';
    ctx.fillText('No runs yet', 40, 30);
    return;
  }

  const values = runHistory.slice(0, 10).map((r) => r.energyKwh);
  const max = Math.max(...values, 0.000001);
  const chartW = width - 46;
  const chartH = height - 42;
  const barW = chartW / values.length;

  values.forEach((v, i) => {
    const x = 32 + i * barW + 3;
    const h = (v / max) * (chartH - 8);
    const y = height - 24 - h;
    ctx.fillStyle = barColor;
    ctx.fillRect(x, y, Math.max(4, barW - 6), h);
  });

  ctx.fillStyle = labelColor;
  ctx.font = '11px IBM Plex Mono';
  ctx.fillText(`max ${max.toExponential(2)} kWh`, 36, 24);
}

function downloadBlob(content, filename, type) {
  const blob = new Blob([content], { type });
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = filename;
  document.body.appendChild(a);
  a.click();
  a.remove();
  URL.revokeObjectURL(url);
}

function exportJsonReport() {
  const payload = {
    generatedAt: new Date().toISOString(),
    lastRun: lastCompilePayload,
    history: runHistory,
  };
  downloadBlob(JSON.stringify(payload, null, 2), 'secure-compiler-report.json', 'application/json');
}

function exportMarkdownReport() {
  const lines = [];
  lines.push('# Secure OTA Compiler Run Report');
  lines.push('');
  lines.push(`Generated: ${new Date().toISOString()}`);
  lines.push('');

  if (lastCompilePayload) {
    lines.push('## Latest Run');
    lines.push('');
    lines.push(`- Status: ${lastCompilePayload.status}`);
    lines.push(`- File: ${lastCompilePayload.filename}`);
    lines.push(`- Violations: ${lastCompilePayload.violations}`);
    lines.push(`- Energy (kWh): ${lastCompilePayload.energyKwh.toFixed(8)}`);
    lines.push(`- Emissions (kgCO2eq): ${lastCompilePayload.emissionsKg.toFixed(8)}`);
    lines.push('');
  }

  lines.push('## Run History');
  lines.push('');
  lines.push('| Time | File | Status | Violations | Energy (kWh) |');
  lines.push('|---|---|---|---:|---:|');
  for (const r of runHistory) {
    lines.push(`| ${formatTimestamp(r.timestamp)} | ${r.filename} | ${r.status.toUpperCase()} | ${r.violations} | ${r.energyKwh.toFixed(8)} |`);
  }

  downloadBlob(lines.join('\n'), 'secure-compiler-report.md', 'text/markdown');
}

async function runCompile() {
  compileBtn.disabled = true;
  compileBtn.textContent = 'Compiling...';

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
    renderEnergy(data.energy);
    rawOutput.textContent = data.rawOutput || '(no output)';

    const totalEnergy = data.energy?.total?.energy_kwh || 0;
    const totalEmissions = data.energy?.total?.emissions_kg || 0;
    const historyEntry = {
      timestamp: Date.now(),
      filename: filenameInput.value.trim() || 'firmware_demo.c',
      status: data.ok ? 'pass' : 'fail',
      violations: (data.violations || []).length,
      energyKwh: Number(totalEnergy),
      emissionsKg: Number(totalEmissions),
    };
    lastCompilePayload = historyEntry;
    addHistoryEntry(historyEntry);
  } catch (err) {
    setBadge('failed');
    setMessage(`Unexpected error: ${err.message}`, 'bad');
  } finally {
    compileBtn.disabled = false;
    compileBtn.textContent = 'Run secure-clang';
  }
}

loadSampleBtn.addEventListener('click', loadSample);
compileBtn.addEventListener('click', runCompile);
exportJsonBtn.addEventListener('click', exportJsonReport);
exportMdBtn.addEventListener('click', exportMarkdownReport);
if (themeToggle) {
  themeToggle.addEventListener('click', toggleTheme);
}

(async function init() {
  applyTheme(getInitialTheme());
  await fetchSamples();
  renderDemoButtons();
  await loadSample();
  renderHistory();
})();
