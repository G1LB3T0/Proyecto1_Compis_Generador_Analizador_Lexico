// ── Estado global ─────────────────────────────────────────────
let lastBuild  = null;
let editorOpen = false;
let dirty      = { yal: false, yalp: false };
let activeTab  = 'yal';

// ── SVG icons ─────────────────────────────────────────────────
const ICONS = {
  pencil:  `<svg class="btn-icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M11 4H4a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2v-7"/><path d="M18.5 2.5a2.121 2.121 0 0 1 3 3L12 15l-4 1 1-4 9.5-9.5z"/></svg>`,
  x:       `<svg class="btn-icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round"><line x1="18" y1="6" x2="6" y2="18"/><line x1="6" y1="6" x2="18" y2="18"/></svg>`,
  play:    `<svg class="btn-icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor" stroke="none"><polygon points="5 3 19 12 5 21 5 3"/></svg>`,
  zap:     `<svg class="btn-icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polygon points="13 2 3 14 12 14 11 22 21 10 12 10 13 2"/></svg>`,
  folder:  `<svg class="btn-icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M22 19a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h5l2 3h9a2 2 0 0 1 2 2z"/></svg>`,
  save:    `<svg class="btn-icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M19 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h11l5 5v11a2 2 0 0 1-2 2z"/><polyline points="17 21 17 13 7 13 7 21"/><polyline points="7 3 7 8 15 8"/></svg>`,
  chevron: `<svg class="chevron-icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round"><polyline points="6 9 12 15 18 9"/></svg>`,
  info:    `<svg class="inline-icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="10"/><line x1="12" y1="8" x2="12" y2="12"/><line x1="12" y1="16" x2="12.01" y2="16"/></svg>`,
  warning: `<svg class="inline-icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M10.29 3.86L1.82 18a2 2 0 0 0 1.71 3h16.94a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0z"/><line x1="12" y1="9" x2="12" y2="13"/><line x1="12" y1="17" x2="12.01" y2="17"/></svg>`,
};

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
  btn.innerHTML = editorOpen ? `${ICONS.x} Cerrar Editor` : `${ICONS.pencil} Abrir Editor`;
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
      showSaveStatus('Guardado', 'var(--green)');
      clearDirtyMark(tab);
    } else {
      showSaveStatus(data.error, 'var(--red)');
    }
  } catch (e) { showSaveStatus('Error de red', 'var(--red)'); }
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
    const content = e.target.result;
    // Si parece código multilinea (más de una línea), cargar como una sola cadena
    const lines = content.split('\n').map(l => l.trimEnd()).filter(Boolean);
    if (lines.length > 1) {
      document.getElementById('input-str').value = content;
      document.getElementById('strings-panel').style.display = 'none';
    } else {
      renderStringsList(lines);
    }
  };
  reader.readAsText(file);
  event.target.value = '';
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
    setStatus('Tabla construida correctamente', 'var(--green)');
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
    setStatus(data.result.accepted ? 'Cadena ACEPTADA' : 'Cadena RECHAZADA',
              data.result.accepted ? 'var(--green)' : 'var(--red)');
  } catch (e) {
    setStatus('Error: ' + e.message, 'var(--red)');
  }
}

// ── Render completo ───────────────────────────────────────────
function renderAll(data) {
  const main = document.getElementById('main');
  const method = data.table.method;
  main.innerHTML = '';
  main.appendChild(makeStep(renderDFAStep(data),     1, 'DFA + Tokens — Proyecto 1',  `${data.dfa.states} estados`));
  main.appendChild(makeStep(renderGrammarStep(data), 2, 'Gramática + FIRST / FOLLOW', `${data.grammar.productions.length} producciones`));

  if (method === 'slr' && data.lr0) {
    main.appendChild(makeStep(renderLR0Step(data),   3, 'Autómata LR(0)',                      `${data.lr0.count} estados`));
    main.appendChild(makeStep(renderTableStep(data), 4, 'Tabla de Análisis',                   'SLR(1)'));
  } else if (method === 'lalr' && data.lr1) {
    main.appendChild(makeStep(renderLR1Step(data),   3, 'Autómata LR(1) → fusionado LALR',     `${data.lr1.count} estados`));
    main.appendChild(makeStep(renderTableStep(data), 4, 'Tabla de Análisis',                   'LALR(1)'));
  } else {
    main.appendChild(makeStep(renderTableStep(data), 3, 'Tabla de Análisis — FIRST / FOLLOW',  'LL(1)'));
  }
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
      <span class="step-toggle open">${ICONS.chevron}</span>
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
    ${ll1 ? `<div class="conflict-banner">${ICONS.info} Gramática transformada para LL(1) — se eliminó recursión izquierda y se aplicó factorización.</div>` : ''}
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
          <span style="margin-left:auto;color:var(--overlay)">${ICONS.chevron}</span>
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
  const ch = hdr.querySelector('.chevron-icon');
  if (ch) ch.classList.toggle('open');
}

// ── PASO 3 (LALR): Autómata LR(1) fusionado ───────────────────
function renderLR1Step(data) {
  const states = data.lr1.states.map(s => {
    const items = s.items.map(it => {
      // Separar la parte del ítem de los lookaheads {a, b}
      const match = it.match(/^(.+?)(\s+\{.+\})$/);
      const core  = match ? escHtml(match[1]) : escHtml(it);
      const la    = match ? `<span class="lr1-la">${escHtml(match[2])}</span>` : '';
      return `<div class="lr0-item">${core}${la}</div>`;
    }).join('');
    const trans = Object.entries(s.transitions).map(([sym, dst]) =>
      `<span class="lr0-trans-chip">${sym} <em>→</em> I${dst}</span>`).join('');
    return `
      <div class="lr0-state">
        <div class="lr0-state-hdr" onclick="toggleLR0(this)">
          <span class="lr0-id">I${s.id}</span>
          <span style="font-size:.78rem;color:var(--subtext)">${s.items.length} ítem(s) LR(1)</span>
          <span style="margin-left:auto;color:var(--overlay)">${ICONS.chevron}</span>
        </div>
        <div class="lr0-items">
          ${items}
          ${trans ? `<div class="lr0-trans"><span>GOTO:</span>${trans}</div>` : ''}
        </div>
      </div>`;
  }).join('');
  return `
    <div class="conflict-banner" style="border-color:var(--teal);color:var(--teal);background:rgba(148,226,213,.06);">
      ${ICONS.info} Los ítems LR(1) incluyen lookaheads <span style="font-family:monospace">{a, b}</span>. Estados con el mismo núcleo LR(0) fueron fusionados para formar los estados LALR.
    </div>
    <div class="lr0-states">${states}</div>`;
}

// ── PASO 4: Tabla de análisis ─────────────────────────────────
function renderTableStep(data) {
  const t = data.table;
  const conflicts = t.has_conflicts
    ? `<div class="conflict-banner">${ICONS.warning} La gramática tiene conflictos en esta tabla.</div>` : '';

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
      <span class="step-toggle open">${ICONS.chevron}</span>
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
  const verdict = ok ? 'ACCEPT' : 'REJECT';
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

  const treeHtml = ok && result.tree ? renderTree(result.tree) : '';

  const div = document.createElement('div');
  div.id = 'step-result';
  div.className = 'pipeline-step';
  div.innerHTML = `
    <div class="step-header" onclick="toggleStep(this)">
      <div class="step-num" style="background:${ok ? 'var(--green)' : 'var(--red)'};color:var(--mantle)">5</div>
      <div class="step-title">Resultado del análisis</div>
      <span class="step-badge" style="color:${ok ? 'var(--green)' : 'var(--red)'};">${method} — ${verdict}</span>
      <span class="step-toggle open">${ICONS.chevron}</span>
    </div>
    <div class="step-body">
      <div class="result-box ${cls}">
        <div class="result-verdict">${verdict}</div>
        <div style="font-size:.85rem;color:var(--subtext);margin-top:6px;">Método: ${method}</div>
        ${errHtml}
      </div>
      ${traceHtml}
      ${treeHtml}
    </div>`;
  return div.outerHTML;
}

// ── Árbol de derivación (Canvas) ──────────────────────────────

const TREE_NODE_W  = 72;
const TREE_NODE_H  = 28;
const TREE_GAP_X   = 14;
const TREE_GAP_Y   = 52;

function treeLayout(node) {
  // Asigna x (leaves primero, luego centra padres) y devuelve ancho del subárbol
  if (!node.children || node.children.length === 0) {
    node._w = TREE_NODE_W + TREE_GAP_X;
    node._x = 0;
    return node._w;
  }
  let total = 0;
  for (const c of node.children) total += treeLayout(c);
  node._w = total;
  // centrar este nodo sobre sus hijos
  const first = node.children[0];
  const last  = node.children[node.children.length - 1];
  node._x = (first._x + last._x) / 2;
  return total;
}

function treeAssignX(node, offsetX) {
  if (!node.children || node.children.length === 0) {
    node._x = offsetX + TREE_NODE_W / 2;
    node._offsetX = offsetX;
    return offsetX + TREE_NODE_W + TREE_GAP_X;
  }
  let cur = offsetX;
  for (const c of node.children) cur = treeAssignX(c, cur);
  const first = node.children[0];
  const last  = node.children[node.children.length - 1];
  node._x = (first._x + last._x) / 2;
  return cur;
}

function treeAssignY(node, depth) {
  node._y = depth * TREE_GAP_Y + TREE_NODE_H / 2;
  if (node.children) for (const c of node.children) treeAssignY(c, depth + 1);
}

function treeMaxXY(node) {
  let mx = node._x + TREE_NODE_W / 2, my = node._y + TREE_NODE_H / 2;
  if (node.children) for (const c of node.children) {
    const [cx, cy] = treeMaxXY(c);
    mx = Math.max(mx, cx); my = Math.max(my, cy);
  }
  return [mx, my];
}

function drawTree(ctx, node, isNT) {
  const x = node._x, y = node._y;
  const hw = TREE_NODE_W / 2, hh = TREE_NODE_H / 2;

  // líneas a hijos
  if (node.children) {
    ctx.strokeStyle = '#45475a';
    ctx.lineWidth   = 1.5;
    for (const c of node.children) {
      ctx.beginPath();
      ctx.moveTo(x, y + hh);
      ctx.lineTo(c._x, c._y - hh);
      ctx.stroke();
      drawTree(ctx, c, isNT);
    }
  }

  const isEps  = node.label === 'ε';
  const isLeaf = !node.children || node.children.length === 0;

  // fondo del nodo
  ctx.beginPath();
  roundRect(ctx, x - hw, y - hh, TREE_NODE_W, TREE_NODE_H, 6);
  if (isEps) {
    ctx.fillStyle = 'rgba(108,112,134,0.18)';
  } else if (isLeaf) {
    ctx.fillStyle = 'rgba(166,227,161,0.13)';
  } else {
    ctx.fillStyle = 'rgba(137,180,250,0.13)';
  }
  ctx.fill();

  // borde
  ctx.strokeStyle = isEps ? '#6c7086' : isLeaf ? '#a6e3a1' : '#89b4fa';
  ctx.lineWidth   = 1.2;
  ctx.stroke();

  // texto label
  ctx.fillStyle  = isEps ? '#6c7086' : isLeaf ? '#a6e3a1' : '#89b4fa';
  ctx.font       = 'bold 11px "JetBrains Mono", monospace';
  ctx.textAlign  = 'center';
  ctx.textBaseline = 'middle';
  ctx.fillText(clip(node.label, 9), x, node.lexema ? y - 5 : y);

  // lexema (para terminales)
  if (node.lexema && node.lexema !== '' && node.lexema !== 'ε') {
    ctx.fillStyle    = '#fab387';
    ctx.font         = '9px "JetBrains Mono", monospace';
    ctx.fillText('"' + clip(node.lexema, 8) + '"', x, y + 7);
  }
}

function roundRect(ctx, x, y, w, h, r) {
  ctx.moveTo(x + r, y);
  ctx.lineTo(x + w - r, y);
  ctx.quadraticCurveTo(x + w, y, x + w, y + r);
  ctx.lineTo(x + w, y + h - r);
  ctx.quadraticCurveTo(x + w, y + h, x + w - r, y + h);
  ctx.lineTo(x + r, y + h);
  ctx.quadraticCurveTo(x, y + h, x, y + h - r);
  ctx.lineTo(x, y + r);
  ctx.quadraticCurveTo(x, y, x + r, y);
  ctx.closePath();
}

function clip(s, n) { return s.length > n ? s.slice(0, n - 1) + '…' : s; }

function renderTree(tree) {
  if (!tree) return '';
  // layout
  treeAssignX(tree, 16);
  treeAssignY(tree, 0);
  const [maxX, maxY] = treeMaxXY(tree);
  const W = maxX + 16;
  const H = maxY + TREE_NODE_H / 2 + 20;

  const id = 'tree-canvas-' + Date.now();
  // devolvemos el contenedor; el canvas se pinta después con requestAnimationFrame
  setTimeout(() => {
    const canvas = document.getElementById(id);
    if (!canvas) return;
    canvas.width  = W;
    canvas.height = H;
    const ctx = canvas.getContext('2d');
    ctx.clearRect(0, 0, W, H);
    drawTree(ctx, tree, true);
  }, 0);

  return `<div class="tree-wrap">
    <div style="font-size:.75rem;color:var(--overlay);margin-bottom:10px;">
      Nodos azules = no terminales &nbsp;·&nbsp; Nodos verdes = terminales &nbsp;·&nbsp; Texto naranja = lexema
    </div>
    <canvas id="${id}" class="tree-canvas"></canvas>
  </div>`;
}

// ── Utilidades ────────────────────────────────────────────────
function escHtml(str) {
  return String(str)
    .replace(/&/g,'&amp;').replace(/</g,'&lt;')
    .replace(/>/g,'&gt;').replace(/"/g,'&quot;');
}
