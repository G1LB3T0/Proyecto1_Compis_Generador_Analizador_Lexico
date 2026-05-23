// ── Estado global ─────────────────────────────────────────────
let lastBuild  = null;
let editorOpen = false;
let dirty      = { yal: false, yalp: false };
let activeTab  = 'yal';

// ── Init ──────────────────────────────────────────────────────
document.addEventListener('DOMContentLoaded', () => {
  loadFiles();
  setupRadios();
});

function setStatus(msg, color) {
  const el = document.getElementById('status');
  el.textContent = msg;
  el.style.color = color || 'var(--subtext)';
}

// ── Archivos ──────────────────────────────────────────────────
async function loadFiles() {
  try {
    const res  = await fetch('/api/files');
    const data = await res.json();
    populate('sel-yal',  data.yal);
    populate('sel-yalp', data.yalp);
  } catch { setStatus('Error cargando archivos', 'var(--red)'); }
}

function populate(id, list) {
  const sel = document.getElementById(id);
  sel.innerHTML = list.map(f => `<option value="${f}">${f}</option>`).join('');
}

// Cuando cambia un selector, refrescar el editor si está abierto
function onFileChange() {
  if (editorOpen) loadEditorFiles();
}

// ── Editor integrado ──────────────────────────────────────────
async function toggleEditor() {
  const panel = document.getElementById('editor-panel');
  const btn   = document.getElementById('btn-editor');
  editorOpen  = !editorOpen;
  panel.style.display = editorOpen ? 'block' : 'none';
  btn.textContent = editorOpen ? '✕ Cerrar Editor' : '✏️ Abrir Editor';
  if (editorOpen) await loadEditorFiles();
}

async function loadEditorFiles() {
  const yal  = document.getElementById('sel-yal').value;
  const yalp = document.getElementById('sel-yalp').value;
  if (yal)  { await fetchIntoEditor('yal',  yal);  updateTabLabel('yal',  yal);  }
  if (yalp) { await fetchIntoEditor('yalp', yalp); updateTabLabel('yalp', yalp); }
  dirty = { yal: false, yalp: false };
  clearDirtyMark('yal'); clearDirtyMark('yalp');
}

async function fetchIntoEditor(tab, filename) {
  try {
    const res  = await fetch(`/api/file/${filename}`);
    const data = await res.json();
    if (data.ok) document.getElementById(`textarea-${tab}`).value = data.content;
  } catch (e) { console.error('Error leyendo archivo:', e); }
}

function updateTabLabel(tab, filename) {
  document.getElementById(`tab-${tab}-label`).textContent = filename;
}

function switchTab(tab) {
  activeTab = tab;
  document.querySelectorAll('.editor-tab').forEach(t => t.classList.remove('active'));
  document.querySelector(`.editor-tab[data-tab="${tab}"]`).classList.add('active');
  document.getElementById('editor-yal').style.display  = tab === 'yal'  ? 'flex' : 'none';
  document.getElementById('editor-yalp').style.display = tab === 'yalp' ? 'flex' : 'none';
}

function markDirty(tab) {
  dirty[tab] = true;
  document.querySelector(`.editor-tab[data-tab="${tab}"]`).classList.add('dirty');
}

function clearDirtyMark(tab) {
  dirty[tab] = false;
  document.querySelector(`.editor-tab[data-tab="${tab}"]`).classList.remove('dirty');
}

async function saveCurrentFile() {
  const tab      = activeTab;
  const filename = tab === 'yal'
    ? document.getElementById('sel-yal').value
    : document.getElementById('sel-yalp').value;
  const content  = document.getElementById(`textarea-${tab}`).value;

  if (!filename) { showSaveStatus('Selecciona un archivo primero', 'var(--yellow)'); return; }

  try {
    const res  = await fetch('/api/save', {
      method:  'POST',
      headers: { 'Content-Type': 'application/json' },
      body:    JSON.stringify({ filename, content })
    });
    const data = await res.json();
    if (data.ok) {
      showSaveStatus('✓ Guardado', 'var(--green)');
      clearDirtyMark(tab);
    } else {
      showSaveStatus('✗ ' + data.error, 'var(--red)');
    }
  } catch (e) { showSaveStatus('✗ Error de red', 'var(--red)'); }
}

function showSaveStatus(msg, color) {
  const el = document.getElementById('save-status');
  el.textContent = msg;
  el.style.color = color;
  clearTimeout(el._t);
  el._t = setTimeout(() => { el.textContent = ''; }, 2500);
}

// ── Cargar cadenas desde archivo .txt ────────────────────────
function loadInputFile(event) {
  const file = event.target.files[0];
  if (!file) return;
  const reader = new FileReader();
  reader.onload = e => {
    const lines = e.target.result
      .split('\n')
      .map(l => l.trim())
      .filter(Boolean);
    renderStringsList(lines);
  };
  reader.readAsText(file);
  event.target.value = ''; // reset input
}

function renderStringsList(lines) {
  const panel = document.getElementById('strings-panel');
  const list  = document.getElementById('strings-list');
  panel.style.display = 'flex';
  list.innerHTML = lines.map(l =>
    `<div class="string-item" onclick="selectString(this,'${escHtml(l).replace(/'/g,"\\'")}')">
       ${escHtml(l)}
     </div>`
  ).join('');
}

function selectString(el, str) {
  document.querySelectorAll('.string-item').forEach(i => i.classList.remove('selected'));
  el.classList.add('selected');
  document.getElementById('input-str').value = str;
}

// ── Radios ────────────────────────────────────────────────────
function setupRadios() {
  document.querySelectorAll('.radio-option').forEach(opt => {
    opt.addEventListener('click', () => {
      document.querySelectorAll('.radio-option').forEach(o => o.classList.remove('active'));
      opt.classList.add('active');
      opt.querySelector('input').checked = true;
    });
  });
}

function getMethod() {
  return document.querySelector('.radio-option.active').dataset.method;
}

// ── Construir tabla ───────────────────────────────────────────
async function buildTable() {
  const yal    = document.getElementById('sel-yal').value;
  const yalp   = document.getElementById('sel-yalp').value;
  const method = getMethod();
  if (!yal || !yalp) { setStatus('Selecciona .yal y .yalp', 'var(--yellow)'); return; }

  document.getElementById('btn-build').disabled = true;
  setStatus('Construyendo...', 'var(--blue)');
  document.getElementById('main').innerHTML = renderSkeleton();

  try {
    const res  = await fetch('/api/run', {
      method: 'POST',
      headers: {'Content-Type':'application/json'},
      body: JSON.stringify({ yal, yalp, method })
    });
    const data = await res.json();
    if (!data.ok) { throw new Error(data.error); }

    lastBuild = { yal, yalp, method, data };
    renderAll(data);
    document.getElementById('parse-section').style.display = 'flex';
    setStatus('Tabla construida correctamente ✓', 'var(--green)');
  } catch (e) {
    document.getElementById('main').innerHTML =
      `<div class="pipeline-step"><div class="step-body" style="color:var(--red)">
        <strong>Error:</strong> ${e.message}</div></div>`;
    setStatus('Error al construir', 'var(--red)');
  } finally {
    document.getElementById('btn-build').disabled = false;
  }
}

// ── Parsear cadena ────────────────────────────────────────────
async function parseInput() {
  if (!lastBuild) return;
  const inp = document.getElementById('input-str').value;
  if (inp.trim() === '') { setStatus('Escribe una cadena', 'var(--yellow)'); return; }

  setStatus('Analizando...', 'var(--blue)');
  const { yal, yalp, method } = lastBuild;

  try {
    const res  = await fetch('/api/run', {
      method: 'POST',
      headers: {'Content-Type':'application/json'},
      body: JSON.stringify({ yal, yalp, method, input: inp })
    });
    const data = await res.json();
    if (!data.ok) throw new Error(data.error);

    // Actualizar resultado (PASO 5)
    removeStep('step-result');
    appendStep(renderResultStep(data.result, method.toUpperCase()));
    document.getElementById('step-result').scrollIntoView({ behavior: 'smooth' });
    setStatus(data.result.accepted ? 'Cadena ACEPTADA ✓' : 'Cadena RECHAZADA ✗',
              data.result.accepted ? 'var(--green)' : 'var(--red)');
  } catch (e) {
    setStatus('Error: ' + e.message, 'var(--red)');
  }
}

// ── Render completo ───────────────────────────────────────────
function renderAll(data) {
  const main = document.getElementById('main');
  main.innerHTML = '';
  main.appendChild(makeStep(renderDFAStep(data),      1, 'DFA + Tokens — Proyecto 1',  `${data.dfa.states} estados`));
  main.appendChild(makeStep(renderGrammarStep(data),  2, 'Gramática + FIRST / FOLLOW', `${data.grammar.productions.length} producciones`));
  main.appendChild(makeStep(renderLR0Step(data),      3, 'Autómata LR(0)',              `${data.lr0.count} estados`));
  main.appendChild(makeStep(renderTableStep(data),    4, 'Tabla de Análisis',           data.table.method.toUpperCase()));
}

function appendStep(html) {
  document.getElementById('main').insertAdjacentHTML('beforeend', html);
}

function removeStep(id) {
  const el = document.getElementById(id);
  if (el) el.remove();
}

// ── Skeleton loader ───────────────────────────────────────────
function renderSkeleton() {
  return Array(3).fill(0).map((_, i) =>
    `<div class="pipeline-step" style="animation-delay:${i*80}ms">
      <div class="step-header">
        <div class="step-num" style="background:var(--surface1)">${i+1}</div>
        <div style="height:14px;width:180px;background:var(--surface0);border-radius:4px"></div>
      </div>
      <div class="step-body" style="height:80px;background:linear-gradient(90deg,var(--surface0) 25%,var(--bg) 50%,var(--surface0) 75%);background-size:400% 100%;animation:shimmer 1.2s infinite"></div>
    </div>`
  ).join('') + '<style>@keyframes shimmer{0%{background-position:100% 0}100%{background-position:-100% 0}}</style>';
}

// ── makeStep (collapsible wrapper) ───────────────────────────
function makeStep(innerHtml, num, title, badge) {
  const div = document.createElement('div');
  div.className = 'pipeline-step';
  div.innerHTML = `
    <div class="step-header" onclick="toggleStep(this)">
      <div class="step-num">${num}</div>
      <div class="step-title">${title}</div>
      <span class="step-badge">${badge}</span>
      <span class="step-toggle open">▾</span>
    </div>
    <div class="step-body">${innerHtml}</div>`;
  return div;
}

function toggleStep(hdr) {
  const body   = hdr.nextElementSibling;
  const toggle = hdr.querySelector('.step-toggle');
  const open   = body.style.display !== 'none';
  body.style.display = open ? 'none' : '';
  toggle.classList.toggle('open', !open);
}

// ── PASO 1: DFA ───────────────────────────────────────────────
function renderDFAStep(data) {
  const rows = data.dfa.tokens.map((t, i) => {
    const pat = (data.dfa.patterns && data.dfa.patterns[i]) || '';
    return `<div class="tok-row">
      <span class="tok-idx">[${i}]</span>
      <span class="tok-kv">&lt;<span class="tok-kv-type">${escHtml(t)}</span>,&nbsp;<span class="tok-kv-lex">"${escHtml(pat)}"</span>&gt;</span>
    </div>`;
  }).join('');

  return `
    <div style="display:grid;grid-template-columns:1fr 1fr;gap:12px;">
      <div class="card">
        <div class="card-title">Resumen</div>
        <div style="display:flex;flex-direction:column;gap:6px;font-size:.85rem;">
          <div>Estados DFA <strong style="color:var(--blue);float:right">${data.dfa.states}</strong></div>
          <div>Reglas .yal <strong style="color:var(--mauve);float:right">${data.dfa.tokens.length}</strong></div>
        </div>
      </div>
      <div class="card">
        <div class="card-title">Tokens</div>
        <div class="tok-list" style="margin-top:8px;">${rows}</div>
      </div>
    </div>`;
}

// ── PASO 2: Gramática + FIRST/FOLLOW ─────────────────────────
function renderGrammarStep(data) {
  const g   = data.table.method === 'll1' && data.grammar_ll1 ? data.grammar_ll1 : data.grammar;
  const ff  = data.table.method === 'll1' && data.ff_ll1      ? data.ff_ll1      : data.ff;
  const ll1 = data.table.method === 'll1' && data.grammar_ll1;

  const prods = g.productions.map(p =>
    `<div class="prod-row">
       <span class="prod-idx">[${p.id}]</span>
       <span class="prod-head">${p.head}</span>
       <span class="prod-arrow"> → </span>
       <span class="prod-body">${p.body}</span>
     </div>`).join('');

  const ffRows = g.non_terminals.map(nt => {
    const fst = (ff.first[nt]  || []).join(', ') || '—';
    const flw = (ff.follow[nt] || []).join(', ') || '—';
    return `<tr>
      <td class="ff-nt">${nt}</td>
      <td class="ff-set">{${fst}}</td>
      <td class="ff-set">{${flw}}</td>
    </tr>`;
  }).join('');

  return `
    ${ll1 ? '<div class="conflict-banner">ℹ Gramática transformada para LL(1) — se eliminó recursión izquierda y se aplicó factorización.</div>' : ''}
    <div class="two-col">
      <div class="card">
        <div class="card-title">Producciones</div>
        <div class="prod-list">${prods}</div>
      </div>
      <div class="card">
        <div class="card-title">FIRST y FOLLOW</div>
        <table class="ff-table">
          <thead><tr><th>NT</th><th>FIRST</th><th>FOLLOW</th></tr></thead>
          <tbody>${ffRows}</tbody>
        </table>
      </div>
    </div>`;
}

// ── PASO 3: LR(0) ─────────────────────────────────────────────
function renderLR0Step(data) {
  const states = data.lr0.states.map(s => {
    const items = s.items.map(it =>
      `<div class="lr0-item">${escHtml(it)}</div>`).join('');
    const trans = Object.entries(s.transitions).map(([sym, dst]) =>
      `<span class="lr0-trans-chip">${sym} <em>→</em> I${dst}</span>`).join('');
    return `
      <div class="lr0-state">
        <div class="lr0-state-hdr" onclick="toggleLR0(this)">
          <span class="lr0-id">I${s.id}</span>
          <span style="font-size:.78rem;color:var(--subtext)">${s.items.length} ítem(s)</span>
          <span style="margin-left:auto;color:var(--overlay)">▾</span>
        </div>
        <div class="lr0-items">
          ${items}
          ${trans ? `<div class="lr0-trans"><span>GOTO:</span>${trans}</div>` : ''}
        </div>
      </div>`;
  }).join('');
  return `<div class="lr0-states">${states}</div>`;
}

function toggleLR0(hdr) {
  const body = hdr.nextElementSibling;
  body.classList.toggle('open');
  hdr.querySelector('span:last-child').textContent =
    body.classList.contains('open') ? '▴' : '▾';
}

// ── PASO 4: Tabla de análisis ─────────────────────────────────
function renderTableStep(data) {
  const t = data.table;
  const conflicts = t.has_conflicts
    ? `<div class="conflict-banner">⚠ La gramática tiene conflictos en esta tabla.</div>` : '';

  if (t.method === 'll1') return conflicts + renderLL1Table(t);
  return conflicts + renderLRTable(t);
}

function renderLRTable(t) {
  const aHdrs = t.action_headers;
  const gHdrs = t.goto_headers;

  let html = '<div class="table-wrap"><table class="parse-table"><thead><tr>';
  html += '<th class="cell-state">Est.</th>';
  aHdrs.forEach(h => html += `<th class="th-action">${h}</th>`);
  html += '<th class="th-sep"></th>';
  gHdrs.forEach(h => html += `<th class="th-goto">${h}</th>`);
  html += '</tr></thead><tbody>';

  t.rows.forEach(row => {
    html += `<tr><td class="cell-state">${row.state}</td>`;
    aHdrs.forEach(h => {
      const cell = row.action[h];
      if (!cell) { html += '<td></td>'; return; }
      const cls = `cell-${cell.type}`;
      html += `<td class="${cls}">${cell.label}</td>`;
    });
    html += '<td class="th-sep"></td>';
    gHdrs.forEach(h => {
      const v = row.goto[h];
      html += v !== undefined ? `<td class="cell-goto">${v}</td>` : '<td></td>';
    });
    html += '</tr>';
  });
  html += '</tbody></table></div>';
  return html;
}

function renderLL1Table(t) {
  const aHdrs = t.action_headers;
  let html = '<div class="table-wrap"><table class="parse-table"><thead><tr>';
  html += '<th class="cell-state">NT</th>';
  aHdrs.forEach(h => html += `<th class="th-action">${h}</th>`);
  html += '</tr></thead><tbody>';

  t.rows.forEach(row => {
    html += `<tr><td class="cell-state">${row.nt}</td>`;
    aHdrs.forEach(h => {
      const cell = row.action[h];
      if (!cell) { html += '<td></td>'; return; }
      html += `<td class="cell-predict" title="${cell.label}">${cell.label}</td>`;
    });
    html += '</tr>';
  });
  html += '</tbody></table></div>';
  return html;
}

// ── PASO 5: Tokens ────────────────────────────────────────────
function renderTokensStep(tokens) {
  const rows = tokens.map((t, i) => {
    const isEof = t.tipo === '$';
    const kvSpan = isEof
      ? `<span class="tok-kv tok-kv-eof">&lt;$, <em>"$"</em>&gt;</span>`
      : `<span class="tok-kv">&lt;<span class="tok-kv-type">${escHtml(t.tipo)}</span>,&nbsp;<span class="tok-kv-lex">"${escHtml(t.lexema)}"</span>&gt;</span>`;
    const meta = isEof
      ? `<span class="tok-meta">EOF</span>`
      : `<span class="tok-meta">l:${t.l} c:${t.c}</span>`;
    return `<div class="tok-row ${isEof ? 'tok-row-eof' : ''}">
      <span class="tok-idx">[${i}]</span>
      ${kvSpan}
      ${meta}
    </div>`;
  }).join('');

  const div = document.createElement('div');
  div.id = 'step-tokens';
  div.className = 'pipeline-step';
  div.innerHTML = `
    <div class="step-header" onclick="toggleStep(this)">
      <div class="step-num" style="background:var(--teal);color:var(--mantle)">5</div>
      <div class="step-title">Tokens generados — Proyecto 1</div>
      <span class="step-badge">${tokens.length - 1} tokens + EOF</span>
      <span class="step-toggle open">▾</span>
    </div>
    <div class="step-body">
      <div style="font-size:.75rem;color:var(--overlay);margin-bottom:10px;">
        Salida del analizador léxico (DFA) sobre la cadena de entrada.
        Formato: <code style="color:var(--mauve)">&lt;TOKEN, "lexema"&gt;</code>
      </div>
      <div class="tok-list">${rows}</div>
    </div>`;
  return div.outerHTML;
}

// ── PASO 5: Resultado ─────────────────────────────────────────
function renderResultStep(result, method) {
  const ok      = result.accepted;
  const verdict = ok ? 'ACCEPT ✓' : 'REJECT ✗';
  const cls     = ok ? 'result-accept' : 'result-reject';

  const errHtml = result.errors && result.errors.length
    ? `<div class="error-list">${result.errors.map(e =>
        `<div class="error-item">${escHtml(e)}</div>`).join('')}</div>` : '';

  // ── Traza de análisis ──────────────────────────────────────
  let traceHtml = '';
  if (result.trace && result.trace.length > 0) {
    const isLR = result.trace.some(s => s.symbols && s.symbols.length > 0);
    const rows  = result.trace.map(s => {
      const actCls = s.action.startsWith('ERROR')  ? 'trace-act-error'
                   : s.action.startsWith('ACCEPT') ? 'trace-act-accept'
                   : s.action.startsWith('MATCH')  ? 'trace-act-match'
                   : '';
      const stackCell = isLR
        ? `<td class="trace-stack">[${escHtml(s.stack).replace(/ /g,',')}]</td>
           <td class="trace-sym">${escHtml(s.symbols)}</td>`
        : `<td class="trace-stack">${escHtml(s.stack)}</td>`;
      return `<tr>
        ${stackCell}
        <td class="trace-in">${escHtml(s.input)}</td>
        <td class="trace-act ${actCls}">${escHtml(s.action)}</td>
      </tr>`;
    }).join('');

    const hdrLR = isLR
      ? '<th class="th-trace-stack">Pila de estados</th><th class="th-trace-sym">Pila de símbolos</th>'
      : '<th class="th-trace-stack">Pila</th>';

    traceHtml = `
      <div style="margin-top:18px;">
        <div class="trace-title">Traza de análisis
          <span style="font-weight:400;color:var(--overlay);font-size:.75rem;margin-left:8px;">${result.trace.length} pasos</span>
        </div>
        <div class="table-wrap trace-wrap">
          <table class="parse-table trace-table">
            <thead><tr>${hdrLR}<th class="th-trace-in">Entrada restante</th><th class="th-trace-act">Acción</th></tr></thead>
            <tbody>${rows}</tbody>
          </table>
        </div>
      </div>`;
  }

  const div = document.createElement('div');
  div.id = 'step-result';
  div.className = 'pipeline-step';
  div.innerHTML = `
    <div class="step-header" onclick="toggleStep(this)">
      <div class="step-num" style="background:${ok ? 'var(--green)' : 'var(--red)'};color:var(--mantle)">5</div>
      <div class="step-title">Resultado del análisis</div>
      <span class="step-badge" style="color:${ok ? 'var(--green)' : 'var(--red)'};">${method} — ${verdict}</span>
      <span class="step-toggle open">▾</span>
    </div>
    <div class="step-body">
      <div class="result-box ${cls}">
        <div class="result-verdict">${verdict}</div>
        <div style="font-size:.85rem;color:var(--subtext);margin-top:6px;">Método: ${method}</div>
        ${errHtml}
      </div>
      ${traceHtml}
    </div>`;
  return div.outerHTML;
}

// ── Utilidades ────────────────────────────────────────────────
function escHtml(str) {
  return String(str)
    .replace(/&/g,'&amp;').replace(/</g,'&lt;')
    .replace(/>/g,'&gt;').replace(/"/g,'&quot;');
}
