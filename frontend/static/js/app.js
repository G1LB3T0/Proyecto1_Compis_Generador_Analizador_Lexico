// Controlador del IDE SQL: ejecución, resultados, errores, árboles y archivos.
// Reutiliza del Proyecto 2 la gestión de archivos, el visor gráfico de árboles
// (ahora sirve para el parse tree y para el AST), diagnósticos, tokens y el
// reporte de la batería de pruebas.
const editor = document.getElementById('source-editor');
const filenameInput = document.getElementById('filename');
const statusBox = document.getElementById('status');
const SVG_NAMESPACE = 'http://www.w3.org/2000/svg';
let latestResult = null;
let selectedFile = 'metadata.txt';

document.addEventListener('DOMContentLoaded', async () => {
  bindEvents();
  await refreshFiles();
  const select = document.getElementById('example-select');
  if (select.value) await loadExample(select.value);
  updateCursor();
  await refreshDatabase();
});

// Eventos principales del editor y administración de archivos .sql.
function bindEvents() {
  document.getElementById('load-example').addEventListener('click', () => {
    const name = document.getElementById('example-select').value;
    if (name) loadExample(name);
  });
  document.getElementById('new-file').addEventListener('click', newFile);
  document.getElementById('local-file').addEventListener('change', loadLocalFile);
  document.getElementById('save-file').addEventListener('click', saveFile);
  document.getElementById('download-file').addEventListener('click', downloadFile);
  document.getElementById('run-button').addEventListener('click', () => runSql('execute'));
  document.getElementById('analyze-button').addEventListener('click', () => runSql('analyze'));
  document.getElementById('reset-button').addEventListener('click', resetDatabase);
  document.getElementById('tests-button').addEventListener('click', runTests);
  filenameInput.addEventListener('input', updateTabName);
  editor.addEventListener('keyup', updateCursor);
  editor.addEventListener('click', updateCursor);
  editor.addEventListener('input', () => setStatus('Cambios sin ejecutar.'));
  editor.addEventListener('keydown', event => {
    if (event.key === 'Tab') {
      event.preventDefault();
      const start = editor.selectionStart;
      editor.setRangeText('  ', start, editor.selectionEnd, 'end');
      updateCursor();
    }
    if ((event.ctrlKey || event.metaKey) && event.key === 'Enter') runSql('execute');
  });
  document.querySelectorAll('.tab').forEach(tab => {
    tab.addEventListener('click', () => selectPanel(tab.dataset.panel));
  });
}

function setStatus(message, kind = '') {
  statusBox.textContent = message;
  statusBox.className = `status ${kind}`.trim();
}

function updateTabName() {
  document.getElementById('tab-name').textContent = normalizedFilename();
}

function normalizedFilename() {
  const raw = filenameInput.value.trim() || 'script.sql';
  return raw.endsWith('.sql') ? raw : `${raw}.sql`;
}

function updateCursor() {
  const before = editor.value.slice(0, editor.selectionStart);
  const lines = before.split('\n');
  document.getElementById('cursor-position').textContent =
    `Ln ${lines.length}, Col ${lines.at(-1).length + 1}`;
}

async function refreshFiles(selected = '') {
  const response = await fetch('/api/files');
  const data = await response.json();
  const select = document.getElementById('example-select');
  select.replaceChildren();
  for (const name of data.files || []) {
    const option = document.createElement('option');
    option.value = name;
    option.textContent = name;
    option.selected = name === selected;
    select.append(option);
  }
}

async function loadExample(name) {
  setStatus(`Cargando ${name}…`);
  const response = await fetch(`/api/file/${encodeURIComponent(name)}`);
  const data = await response.json();
  if (!data.ok) return setStatus(data.error, 'error');
  filenameInput.value = data.filename;
  editor.value = data.content;
  updateTabName();
  updateCursor();
  setStatus(`${data.filename} cargado.`, 'success');
}

function newFile() {
  filenameInput.value = 'script.sql';
  editor.value = '';
  updateTabName();
  editor.focus();
  setStatus('Nuevo archivo .sql.');
}

function loadLocalFile(event) {
  const file = event.target.files[0];
  if (!file) return;
  if (!file.name.endsWith('.sql')) return setStatus('Selecciona un archivo .sql.', 'error');
  const reader = new FileReader();
  reader.onload = () => {
    filenameInput.value = file.name;
    editor.value = reader.result;
    updateTabName();
    updateCursor();
    setStatus(`${file.name} cargado desde tu equipo.`, 'success');
  };
  reader.readAsText(file);
  event.target.value = '';
}

async function saveFile() {
  const filename = normalizedFilename();
  const response = await fetch('/api/save', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({filename, source: editor.value}),
  });
  const data = await response.json();
  if (!data.ok) return setStatus(data.error, 'error');
  filenameInput.value = data.filename;
  updateTabName();
  await refreshFiles(data.filename);
  setStatus(`${data.filename} guardado en ejemplos.`, 'success');
}

function downloadFile() {
  const blob = new Blob([editor.value], {type: 'text/plain;charset=utf-8'});
  const link = document.createElement('a');
  link.href = URL.createObjectURL(blob);
  link.download = normalizedFilename();
  link.click();
  URL.revokeObjectURL(link.href);
}

// Envía el script al CLI C++ (vía Flask) y reparte el resultado en paneles.
async function runSql(mode) {
  const buttons = [document.getElementById('run-button'), document.getElementById('analyze-button')];
  buttons.forEach(button => { button.disabled = true; });
  setStatus(mode === 'execute' ? 'Interpretando el script…' : 'Analizando sin ejecutar…');
  try {
    const response = await fetch('/api/run', {
      method: 'POST',
      headers: {'Content-Type': 'application/json'},
      body: JSON.stringify({filename: normalizedFilename(), source: editor.value, mode}),
    });
    const data = await response.json();
    if (data.error) throw new Error(data.error);
    latestResult = data;
    renderResult(data);
    const errors = (data.diagnostics || []).length;
    if (data.ok) {
      setStatus(mode === 'execute' ? 'Script ejecutado y guardado en archivos.' : 'Análisis completo: el script es válido.', 'success');
    } else {
      setStatus(`Se encontraron ${errors} error(es).`, 'error');
    }
  } catch (error) {
    setStatus(error.message || 'No se pudo ejecutar el script.', 'error');
  } finally {
    buttons.forEach(button => { button.disabled = false; });
  }
}

function renderResult(data) {
  renderSummary(data);
  renderStatements(data);
  renderDiagnostics(data.diagnostics || []);
  renderTokens(data.tokens || []);
  renderTree('tree-panel', data.tree, {
    empty: 'No hay árbol sintáctico disponible.',
    label: 'Árbol sintáctico concreto generado por ANTLR',
    legend: [['rule', 'Regla de la gramática'], ['token', 'Token']],
  });
  renderTree('ast-panel', data.ast, {
    empty: 'El AST se construye solo cuando el script no tiene errores léxicos ni sintácticos.',
    label: 'Árbol de sintaxis abstracta',
    legend: [['rule', 'Operación / cláusula'], ['token', 'Valor (tabla, columna, literal)']],
  });
  if (data.database) renderDatabase(data.database);
  document.getElementById('summary').classList.remove('hidden');
  const hasErrors = (data.diagnostics || []).length > 0;
  selectPanel(hasErrors ? 'diagnostics-panel' : 'results-panel');
}

function renderSummary(data) {
  const phases = data.phases || {};
  const analyzeOnly = data.mode === 'analyze';
  const executed = (data.statements || [])
    .filter(item => item.status === 'ok' || item.status === 'analizada').length;
  let execution = phases.execution ? 'Completa' : (phases.execution_skipped ? 'Omitida' : 'Con errores');
  if (analyzeOnly) execution = 'Solo análisis';
  const cards = [
    ['Léxico', phases.lexical ? 'Correcto' : 'Con errores', phases.lexical],
    ['Sintaxis', phases.syntax ? 'Correcta' : (phases.lexical ? 'Con errores' : 'Omitida'), phases.syntax],
    ['Semántica', phases.semantic ? 'Correcta' : (phases.semantic_skipped ? 'Omitida' : 'Con errores'), phases.semantic],
    ['Ejecución', execution, phases.execution || (analyzeOnly && phases.semantic)],
    ['Sentencias', `${executed}/${(data.statements || []).length}`, true],
    ['Tokens', (data.tokens || []).length, true],
  ];
  const summary = document.getElementById('summary');
  summary.replaceChildren();
  for (const [label, value, good] of cards) {
    const card = document.createElement('div');
    card.className = `summary-card ${good ? 'good' : 'bad'}`;
    const caption = document.createElement('span');
    caption.textContent = label;
    const number = document.createElement('strong');
    number.textContent = value;
    card.append(caption, number);
    summary.append(card);
  }
}

// ── Resultados de cada sentencia ──

function renderStatements(data) {
  const statements = data.statements || [];
  document.getElementById('statement-count').textContent = statements.length;
  const panel = document.getElementById('results-panel');
  panel.replaceChildren();
  const reason = data.phases?.execution_reason;
  if (reason) {
    const note = semanticNote(data.mode === 'analyze' ? 'Modo análisis' : 'Ejecución', reason);
    if (!data.ok) note.classList.add('warning');
    panel.append(note);
  }
  if (!statements.length) {
    panel.append(emptyMessage((data.diagnostics || []).length
      ? 'Corrige los errores léxicos o sintácticos para ver las sentencias.'
      : 'El script no contiene sentencias.'));
    return;
  }
  for (const statement of statements) panel.append(statementCard(statement));
}

function statementCard(statement) {
  const card = document.createElement('article');
  card.className = `statement-card ${statement.status}`;

  const header = document.createElement('header');
  header.className = 'statement-header';
  const index = document.createElement('span');
  index.className = 'statement-index';
  index.textContent = `#${statement.index}`;
  const kind = document.createElement('strong');
  kind.textContent = statement.kind;
  const status = document.createElement('span');
  status.className = `statement-status ${statement.status}`;
  status.textContent = statusLabel(statement.status);
  const location = document.createElement('button');
  location.type = 'button';
  location.className = 'statement-location';
  location.textContent = `Ln ${statement.line}`;
  location.addEventListener('click', () => jumpTo(statement.line, statement.column));
  header.append(index, kind, status, location);

  const sql = document.createElement('pre');
  sql.className = 'statement-sql';
  sql.textContent = statement.sql;

  const message = document.createElement('p');
  message.className = 'statement-message';
  message.textContent = statement.message;
  card.append(header, sql, message);

  if ((statement.columns || []).length) card.append(resultTable(statement));

  const extras = [];
  if ((statement.plan || []).length) extras.push(['Plan de ejecución', statement.plan]);
  if ((statement.files || []).length) extras.push(['Archivos escritos', statement.files]);
  for (const [title, items] of extras) {
    const details = document.createElement('details');
    details.className = 'statement-extra';
    details.open = title === 'Plan de ejecución';
    const summary = document.createElement('summary');
    summary.textContent = `${title} (${items.length})`;
    const list = document.createElement('ul');
    for (const item of items) {
      const entry = document.createElement('li');
      entry.textContent = item;
      if (/Index (Scan|Nested)/.test(item)) entry.classList.add('uses-index');
      list.append(entry);
    }
    details.append(summary, list);
    card.append(details);
  }
  return card;
}

function statusLabel(status) {
  return {ok: 'Ejecutada', error: 'Error', omitida: 'Omitida', analizada: 'Válida'}[status] || status;
}

function resultTable(statement) {
  const wrapper = document.createElement('div');
  wrapper.className = 'result-table-wrapper';
  const table = makeTable(statement.columns.map((name, index) =>
    `${name} · ${statement.column_types[index] || ''}`));
  for (const row of statement.rows) {
    const tr = document.createElement('tr');
    for (const value of row) {
      const td = document.createElement('td');
      if (value === null) {
        td.textContent = 'NULL';
        td.className = 'null-cell';
      } else {
        td.textContent = value;
      }
      tr.append(td);
    }
    table.tBodies[0].append(tr);
  }
  if (!statement.rows.length) {
    const tr = document.createElement('tr');
    const td = document.createElement('td');
    td.colSpan = statement.columns.length;
    td.className = 'muted';
    td.textContent = 'La consulta no devolvió registros.';
    tr.append(td);
    table.tBodies[0].append(tr);
  }
  wrapper.append(table);
  return wrapper;
}

// ── Diagnósticos ──

function renderDiagnostics(diagnostics) {
  document.getElementById('diagnostic-count').textContent = diagnostics.length;
  const panel = document.getElementById('diagnostics-panel');
  panel.replaceChildren();
  if (!diagnostics.length) {
    const banner = document.createElement('div');
    banner.className = 'success-banner';
    banner.textContent = '✓ No se encontraron errores léxicos, sintácticos, semánticos ni de ejecución.';
    panel.append(banner);
    return;
  }
  for (const item of diagnostics) {
    const row = document.createElement('div');
    row.className = `diagnostic ${item.category === 'ejecución' ? 'runtime' : ''}`;
    row.addEventListener('click', () => jumpTo(item.line, item.column));
    const code = document.createElement('span');
    code.className = 'diagnostic-code';
    code.textContent = item.code;
    const message = document.createElement('div');
    message.className = 'diagnostic-message';
    const category = document.createElement('strong');
    category.textContent = item.statement > 0
      ? `${item.category} · sentencia ${item.statement}` : item.category;
    const text = document.createElement('span');
    text.textContent = item.message;
    message.append(category, text);
    const location = document.createElement('span');
    location.className = 'diagnostic-location';
    location.textContent = `${item.line}:${item.column}`;
    row.append(code, message, location);
    panel.append(row);
  }
}

function jumpTo(line, column) {
  if (!line) return;
  const lines = editor.value.split('\n');
  let offset = 0;
  for (let index = 0; index < line - 1 && index < lines.length; index += 1)
    offset += lines[index].length + 1;
  offset += Math.max(0, column - 1);
  editor.focus();
  editor.setSelectionRange(offset, offset);
  updateCursor();
}

// ── Tokens ──

function renderTokens(tokens) {
  document.getElementById('token-count').textContent = tokens.length;
  const panel = document.getElementById('tokens-panel');
  panel.replaceChildren();
  if (!tokens.length) return panel.append(emptyMessage('No se reconocieron tokens.'));
  const table = makeTable(['#', 'Categoría', 'Token ANTLR', 'Lexema', 'Línea', 'Columna']);
  tokens.forEach((token, index) => addRow(table.tBodies[0], [
    index + 1, token.category || 'Símbolo', codeCell(token.type),
    codeCell(token.text), token.line, token.column,
  ]));
  panel.append(table);
}

// ── Visor gráfico de árboles (parse tree y AST) ──

const treeViews = {};

function renderTree(panelId, tree, options) {
  const panel = document.getElementById(panelId);
  panel.replaceChildren();
  if (!tree) {
    delete treeViews[panelId];
    return panel.append(emptyMessage(options.empty));
  }
  const view = {
    root: indexTree(tree),
    collapsed: new Set(),
    zoom: 1,
    label: options.label,
    viewport: null,
    stage: null,
    zoomLabel: null,
    width: 0,
    height: 0,
  };
  treeViews[panelId] = view;
  setTreeOverview(view, view.root);

  const toolbar = document.createElement('div');
  toolbar.className = 'tree-toolbar';
  const actions = document.createElement('div');
  actions.className = 'tree-actions';
  actions.append(
    treeButton('Vista general', () => {
      view.collapsed.clear();
      setTreeOverview(view, view.root);
      drawTreeGraph(view);
    }),
    treeButton('Expandir todo', () => {
      view.collapsed.clear();
      drawTreeGraph(view);
    }),
    treeButton('Contraer', () => {
      view.collapsed.clear();
      view.collapsed.add(view.root.id);
      drawTreeGraph(view);
    }),
  );
  const help = document.createElement('div');
  help.className = 'tree-help';
  for (const [kind, label] of options.legend) help.append(treeLegend(kind, label));
  const hint = document.createElement('span');
  hint.className = 'tree-hint';
  hint.textContent = 'Clic en un nodo para explorar';
  help.append(hint);
  const zoom = document.createElement('div');
  zoom.className = 'tree-zoom';
  view.zoomLabel = document.createElement('span');
  view.zoomLabel.textContent = '100%';
  zoom.append(
    treeButton('−', () => changeTreeZoom(view, -0.1), 'Alejar'),
    view.zoomLabel,
    treeButton('+', () => changeTreeZoom(view, 0.1), 'Acercar'),
  );
  toolbar.append(actions, help, zoom);

  view.viewport = document.createElement('div');
  view.viewport.className = 'tree-viewport';
  view.stage = document.createElement('div');
  view.stage.className = 'tree-stage';
  view.viewport.append(view.stage);
  panel.append(toolbar, view.viewport);
  drawTreeGraph(view);
}

function indexTree(node, id = '0', depth = 0) {
  return {
    ...node,
    id,
    depth,
    children: (node.children || []).map((child, index) =>
      indexTree(child, `${id}.${index}`, depth + 1)),
  };
}

function setTreeOverview(view, node) {
  if (node.children.length && node.depth >= 3) view.collapsed.add(node.id);
  node.children.forEach(child => setTreeOverview(view, child));
}

function treeButton(label, handler, title = label) {
  const button = document.createElement('button');
  button.type = 'button';
  button.className = 'tree-control';
  button.textContent = label;
  button.title = title;
  button.addEventListener('click', handler);
  return button;
}

function treeLegend(kind, label) {
  const item = document.createElement('span');
  item.className = 'tree-legend';
  const dot = document.createElement('i');
  dot.className = kind;
  item.append(dot, label);
  return item;
}

function changeTreeZoom(view, delta) {
  view.zoom = Math.min(1.8, Math.max(0.4, view.zoom + delta));
  applyTreeZoom(view);
}

function applyTreeZoom(view) {
  if (!view.stage) return;
  view.stage.style.width = `${view.width * view.zoom}px`;
  view.stage.style.height = `${view.height * view.zoom}px`;
  const svg = view.stage.querySelector('svg');
  if (svg) {
    svg.style.width = `${view.width * view.zoom}px`;
    svg.style.height = `${view.height * view.zoom}px`;
  }
  if (view.zoomLabel) view.zoomLabel.textContent = `${Math.round(view.zoom * 100)}%`;
}

function visibleChildren(view, node) {
  return view.collapsed.has(node.id) ? [] : node.children;
}

function treeNodeWidth(node) {
  const label = node.kind === 'token' ? node.text : node.name;
  return Math.min(200, Math.max(116, 44 + String(label).length * 7));
}

function measureTree(view, node, metrics) {
  const children = visibleChildren(view, node);
  const ownWidth = treeNodeWidth(node) + 28;
  if (!children.length) {
    metrics.set(node.id, {subtreeWidth: ownWidth, nodeWidth: treeNodeWidth(node)});
    return ownWidth;
  }
  const childrenWidth = children.reduce((total, child, index) =>
    total + measureTree(view, child, metrics) + (index ? 22 : 0), 0);
  const subtreeWidth = Math.max(ownWidth, childrenWidth);
  metrics.set(node.id, {subtreeWidth, nodeWidth: treeNodeWidth(node)});
  return subtreeWidth;
}

function positionTree(view, node, left, depth, metrics, positions) {
  const metric = metrics.get(node.id);
  const children = visibleChildren(view, node);
  let center = left + metric.subtreeWidth / 2;
  if (children.length) {
    const childrenWidth = children.reduce((total, child, index) =>
      total + metrics.get(child.id).subtreeWidth + (index ? 22 : 0), 0);
    let childLeft = left + (metric.subtreeWidth - childrenWidth) / 2;
    const childCenters = [];
    for (const child of children) {
      positionTree(view, child, childLeft, depth + 1, metrics, positions);
      childCenters.push(positions.get(child.id).x);
      childLeft += metrics.get(child.id).subtreeWidth + 22;
    }
    center = (childCenters[0] + childCenters.at(-1)) / 2;
  }
  positions.set(node.id, {x: center, y: 38 + depth * 106, width: metric.nodeWidth, depth});
}

function drawTreeGraph(view) {
  if (!view.root || !view.stage) return;
  const metrics = new Map();
  const positions = new Map();
  const measuredWidth = measureTree(view, view.root, metrics);
  view.width = Math.max(720, measuredWidth + 56);
  positionTree(view, view.root, (view.width - measuredWidth) / 2, 0, metrics, positions);
  const maxDepth = Math.max(...Array.from(positions.values(), item => item.depth));
  view.height = Math.max(250, 116 + maxDepth * 106);

  const svg = svgElement('svg', {
    class: 'syntax-tree-svg',
    viewBox: `0 0 ${view.width} ${view.height}`,
    role: 'img',
    'aria-label': view.label,
  });
  const edges = svgElement('g', {class: 'tree-edges'});
  const nodes = svgElement('g', {class: 'tree-nodes'});
  svg.append(edges, nodes);
  appendTreeSvg(view, view.root, positions, edges, nodes);
  view.stage.replaceChildren(svg);
  applyTreeZoom(view);
}

function appendTreeSvg(view, node, positions, edges, nodes) {
  const position = positions.get(node.id);
  for (const child of visibleChildren(view, node)) {
    const childPosition = positions.get(child.id);
    edges.append(svgElement('path', {
      d: `M ${position.x} ${position.y + 28} C ${position.x} ${position.y + 66}, ${childPosition.x} ${childPosition.y - 38}, ${childPosition.x} ${childPosition.y - 28}`,
    }));
    appendTreeSvg(view, child, positions, edges, nodes);
  }

  const expandable = node.children.length > 0;
  const collapsed = view.collapsed.has(node.id);
  const group = svgElement('g', {
    class: `tree-graph-node ${node.kind}${expandable ? ' expandable' : ''}`,
    transform: `translate(${position.x}, ${position.y})`,
    tabindex: expandable ? '0' : '-1',
    role: expandable ? 'button' : 'img',
    'aria-label': `${node.kind === 'token' ? 'Hoja' : 'Nodo'} ${node.name}`,
  });
  const shape = svgElement('rect', {
    x: -position.width / 2,
    y: -28,
    width: position.width,
    height: 56,
    rx: node.kind === 'token' ? 27 : 9,
  });
  const title = svgElement('title');
  title.textContent = node.kind === 'token'
    ? `${node.category || node.name}: ${node.text} (${node.line}:${node.column})`
    : `${node.name}${node.detail ? ` — ${node.detail}` : ''} (${node.line}:${node.column})`;
  shape.append(title);

  const mainText = svgElement('text', {class: 'tree-node-title', y: '-4'});
  mainText.textContent = shortened(node.kind === 'token' ? node.text : node.name, 24);
  const detail = svgElement('text', {class: 'tree-node-detail', y: '14'});
  detail.textContent = node.kind === 'token'
    ? shortened(node.category || node.name, 26)
    : shortened(node.detail || `${node.line}:${node.column} · ${node.children.length} hijo${node.children.length === 1 ? '' : 's'}`, 26);
  group.append(shape, mainText, detail);

  if (expandable) {
    const badge = svgElement('g', {
      class: 'tree-node-badge',
      transform: `translate(${position.width / 2 - 8}, -20)`,
    });
    badge.append(svgElement('circle', {r: '9'}));
    const badgeText = svgElement('text', {y: '4'});
    badgeText.textContent = collapsed ? '+' : '−';
    badge.append(badgeText);
    group.append(badge);
    const toggle = () => toggleTreeNode(view, node);
    group.addEventListener('click', toggle);
    group.addEventListener('keydown', event => {
      if (event.key === 'Enter' || event.key === ' ') {
        event.preventDefault();
        toggle();
      }
    });
  }
  nodes.append(group);
}

function toggleTreeNode(view, node) {
  if (view.collapsed.has(node.id)) view.collapsed.delete(node.id);
  else view.collapsed.add(node.id);
  drawTreeGraph(view);
}

function shortened(value, limit) {
  const text = String(value ?? '');
  return text.length > limit ? `${text.slice(0, limit - 1)}…` : text;
}

function svgElement(name, attributes = {}) {
  const element = document.createElementNS(SVG_NAMESPACE, name);
  for (const [key, value] of Object.entries(attributes)) element.setAttribute(key, value);
  return element;
}

// ── Base de datos: catálogo e inspección de archivos ──

async function refreshDatabase() {
  try {
    const response = await fetch('/api/database');
    const data = await response.json();
    if (data.error) throw new Error(data.error);
    renderDatabase(data);
  } catch (error) {
    const panel = document.getElementById('database-panel');
    panel.replaceChildren(emptyMessage(error.message));
  }
}

async function resetDatabase() {
  if (!window.confirm('¿Eliminar todas las tablas, registros e índices de database/?')) return;
  const response = await fetch('/api/database/reset', {method: 'POST'});
  const data = await response.json();
  if (data.error) return setStatus(data.error, 'error');
  renderDatabase(data);
  selectPanel('database-panel');
  setStatus('Base de datos reiniciada: database/ quedó vacía.', 'success');
}

function renderDatabase(database) {
  const tables = database.tables || [];
  document.getElementById('table-count').textContent = tables.length;
  const panel = document.getElementById('database-panel');
  panel.replaceChildren();
  panel.append(semanticNote('Almacenamiento',
    `Todo vive en ${database.path}: metadata.txt guarda esquemas, tipos, restricciones e índices; ` +
    'cada tabla es un .csv (la columna __rowid identifica cada registro) y cada índice un .idx ordenado.'));

  const layout = document.createElement('div');
  layout.className = 'database-layout';
  const catalog = document.createElement('div');
  catalog.className = 'database-catalog';
  if (!tables.length) catalog.append(emptyMessage('La base de datos está vacía. Ejecuta un CREATE TABLE.'));
  for (const table of tables) catalog.append(tableCard(table));

  const files = document.createElement('div');
  files.className = 'database-files';
  const fileTitle = subsectionTitle('Archivos en disco', 'Selecciona un archivo para ver su contenido real.');
  const tree = document.createElement('div');
  tree.className = 'file-tree';
  const viewer = document.createElement('pre');
  viewer.className = 'file-viewer';
  const entries = database.files || [];
  if (!entries.some(file => file.path === selectedFile)) selectedFile = entries[0]?.path || '';
  const root = document.createElement('div');
  root.className = 'file-tree-root';
  root.textContent = 'database/';
  tree.append(root);
  const show = file => {
    selectedFile = file.path;
    viewer.textContent = file.content + (file.truncated ? '\n… (archivo truncado)' : '');
    tree.querySelectorAll('button').forEach(button =>
      button.classList.toggle('active', button.dataset.path === file.path));
  };
  for (const file of entries) {
    const button = document.createElement('button');
    button.type = 'button';
    button.dataset.path = file.path;
    button.className = file.path.startsWith('indexes/') ? 'nested' : '';
    const name = document.createElement('span');
    name.textContent = file.path;
    const size = document.createElement('small');
    size.textContent = `${file.bytes} B`;
    button.append(name, size);
    button.addEventListener('click', () => show(file));
    tree.append(button);
  }
  files.append(fileTitle, tree, viewer);
  const current = entries.find(file => file.path === selectedFile);
  if (current) show(current);
  else viewer.textContent = 'Aún no hay archivos.';

  layout.append(catalog, files);
  panel.append(layout);
}

function tableCard(table) {
  const card = document.createElement('div');
  card.className = 'table-card';
  const header = document.createElement('div');
  header.className = 'table-card-header';
  const name = document.createElement('h3');
  name.textContent = table.name;
  const meta = document.createElement('span');
  meta.textContent = `${table.rows} registro${table.rows === 1 ? '' : 's'} · ${table.file}`;
  header.append(name, meta);
  card.append(header);

  const columns = makeTable(['Columna', 'Tipo', 'Restricciones']);
  for (const column of table.columns) {
    const constraints = document.createElement('div');
    if (column.primary_key) constraints.append(tag('PRIMARY KEY'));
    if (column.not_null && !column.primary_key) constraints.append(tag('NOT NULL'));
    if (column.unique) constraints.append(tag('UNIQUE'));
    if (column.references) constraints.append(tag(`→ ${column.references}`));
    if (!constraints.childNodes.length) constraints.append(Object.assign(document.createElement('span'), {className: 'muted', textContent: '—'}));
    addRow(columns.tBodies[0], [codeCell(column.name), column.type, constraints]);
  }
  card.append(columns);

  if ((table.indexes || []).length) {
    const indexes = makeTable(['Índice', 'Columna', 'Tipo', 'Claves']);
    for (const index of table.indexes) {
      addRow(indexes.tBodies[0], [
        codeCell(index.name), index.column,
        `${index.unique ? 'UNIQUE' : 'normal'}${index.automatic ? ' · automático' : ''}`,
        `${index.keys} (${index.entries} entradas)`,
      ]);
    }
    card.append(indexes);
  }
  return card;
}

function tag(text) {
  const element = document.createElement('span');
  element.className = 'tag';
  element.textContent = text;
  return element;
}

// ── Utilidades de tablas y paneles ──

function makeTable(headers) {
  const table = document.createElement('table');
  table.className = 'data-table';
  const head = table.createTHead().insertRow();
  for (const header of headers) {
    const th = document.createElement('th');
    th.textContent = header;
    head.append(th);
  }
  table.createTBody();
  return table;
}

function addRow(body, values) {
  const row = body.insertRow();
  for (const value of values) {
    const cell = row.insertCell();
    if (value instanceof Node) cell.append(value);
    else cell.textContent = value;
  }
}

function codeCell(text) {
  const code = document.createElement('code');
  code.textContent = text;
  return code;
}

function emptyMessage(text) {
  const element = document.createElement('div');
  element.className = 'empty-result';
  element.textContent = text;
  return element;
}

function semanticNote(title, text) {
  const note = document.createElement('div');
  note.className = 'semantic-note';
  const heading = document.createElement('strong');
  heading.textContent = title;
  const body = document.createElement('p');
  body.textContent = text;
  note.append(heading, body);
  return note;
}

function subsectionTitle(title, description) {
  const wrapper = document.createElement('div');
  wrapper.className = 'semantic-section-title';
  const heading = document.createElement('h3');
  heading.textContent = title;
  const text = document.createElement('p');
  text.textContent = description;
  wrapper.append(heading, text);
  return wrapper;
}

function selectPanel(panelId) {
  document.querySelectorAll('.tab').forEach(tab => tab.classList.toggle('active', tab.dataset.panel === panelId));
  document.querySelectorAll('.result-panel').forEach(panel => panel.classList.toggle('active', panel.id === panelId));
}

// ── Batería de pruebas (formato de reporte reutilizado del Proyecto 2) ──

function testMetric(value, label) {
  const metric = document.createElement('div');
  metric.className = 'test-metric';
  const number = document.createElement('strong');
  number.textContent = value;
  const caption = document.createElement('span');
  caption.textContent = label;
  metric.append(number, caption);
  return metric;
}

function testDetail(label, value, code = false) {
  const section = document.createElement('div');
  section.className = 'test-detail';
  const title = document.createElement('strong');
  title.textContent = label;
  const content = document.createElement(code ? 'pre' : 'p');
  content.textContent = value || '—';
  section.append(title, content);
  return section;
}

function renderTestReport(resultBox, data) {
  const summary = data.summary || {};
  resultBox.replaceChildren();
  resultBox.className = `tests-result ${data.ok ? 'pass' : 'fail'}`;

  const heading = document.createElement('div');
  heading.className = 'test-report-heading';
  const titleGroup = document.createElement('div');
  const title = document.createElement('h3');
  title.textContent = data.ok ? 'Batería del intérprete SQL aprobada' : 'La batería encontró fallos';
  const explanation = document.createElement('p');
  explanation.textContent = 'Cada caso se ejecuta sobre una base de datos temporal y aislada. Un “error esperado” solo aprueba cuando el intérprete lo rechaza con el código de error correcto.';
  titleGroup.append(title, explanation);

  const actions = document.createElement('div');
  actions.className = 'test-report-actions';
  const expandButton = document.createElement('button');
  expandButton.type = 'button';
  expandButton.textContent = 'Expandir todo';
  const collapseButton = document.createElement('button');
  collapseButton.type = 'button';
  collapseButton.textContent = 'Contraer casos';
  actions.append(expandButton, collapseButton);
  heading.append(titleGroup, actions);
  resultBox.append(heading);

  const metrics = document.createElement('div');
  metrics.className = 'test-metrics';
  metrics.append(
    testMetric(`${summary.passed || 0}/${summary.total || 0}`, 'Pruebas aprobadas'),
    testMetric(summary.valid_cases || 0, 'Casos válidos'),
    testMetric(summary.expected_error_cases || 0, 'Errores esperados'),
    testMetric(summary.failed || 0, 'Fallos reales'),
  );
  resultBox.append(metrics);

  const categoryContainer = document.createElement('div');
  categoryContainer.className = 'test-categories';
  for (const category of data.categories || []) {
    const categoryDetails = document.createElement('details');
    categoryDetails.className = 'test-category';
    categoryDetails.open = true;
    const categorySummary = document.createElement('summary');
    const categoryName = document.createElement('span');
    categoryName.textContent = category.name;
    const categoryCount = document.createElement('span');
    categoryCount.className = category.passed === category.total ? 'test-count pass' : 'test-count fail';
    categoryCount.textContent = `${category.passed}/${category.total}`;
    categorySummary.append(categoryName, categoryCount);
    categoryDetails.append(categorySummary);

    const cases = document.createElement('div');
    cases.className = 'test-cases';
    for (const test of category.tests || []) {
      const testCase = document.createElement('details');
      testCase.className = `test-case ${test.passed ? 'pass' : 'fail'}`;
      if (!test.passed) testCase.open = true;
      const testSummary = document.createElement('summary');
      const state = document.createElement('span');
      state.className = 'test-state';
      state.textContent = test.passed ? '✓' : '✕';
      const description = document.createElement('span');
      description.className = 'test-description';
      const rule = document.createElement('strong');
      rule.textContent = test.rule;
      const name = document.createElement('span');
      name.textContent = test.name;
      description.append(rule, name);
      const kind = document.createElement('span');
      kind.className = `test-kind ${test.kind === 'Error esperado' ? 'negative' : 'positive'}`;
      kind.textContent = test.kind;
      testSummary.append(state, description, kind);
      testCase.append(testSummary);

      const body = document.createElement('div');
      body.className = 'test-case-body';
      const explanationDetail = testDetail('¿Qué está pasando?', test.explanation);
      explanationDetail.classList.add('test-explanation');
      body.append(
        testDetail('SQL probado', test.source, true),
        explanationDetail,
        testDetail('Resultado esperado', test.expected),
        testDetail('Resultado obtenido', test.actual),
      );
      if ((test.codes || []).length) {
        const diagnostics = document.createElement('div');
        diagnostics.className = 'test-detail test-diagnostics';
        const diagnosticsTitle = document.createElement('strong');
        diagnosticsTitle.textContent = 'Códigos producidos';
        diagnostics.append(diagnosticsTitle);
        for (const diagnostic of test.codes) {
          const badge = document.createElement('code');
          badge.textContent = diagnostic;
          diagnostics.append(badge);
        }
        body.append(diagnostics);
      }
      testCase.append(body);
      cases.append(testCase);
    }
    categoryDetails.append(cases);
    categoryContainer.append(categoryDetails);
  }
  resultBox.append(categoryContainer);

  expandButton.addEventListener('click', () => {
    resultBox.querySelectorAll('details').forEach(detail => { detail.open = true; });
  });
  collapseButton.addEventListener('click', () => {
    resultBox.querySelectorAll('.test-category').forEach(detail => { detail.open = true; });
    resultBox.querySelectorAll('.test-case').forEach(detail => { detail.open = false; });
  });
}

async function runTests() {
  const button = document.getElementById('tests-button');
  const resultBox = document.getElementById('tests-result');
  button.disabled = true;
  resultBox.className = 'tests-result';
  resultBox.textContent = 'Ejecutando batería C++…';
  try {
    const response = await fetch('/api/tests', {method: 'POST'});
    const data = await response.json();
    if (data.error) throw new Error(data.error);
    renderTestReport(resultBox, data);
    setStatus(data.ok ? 'Batería de pruebas aprobada.' : 'La batería encontró fallos.', data.ok ? 'success' : 'error');
    resultBox.scrollIntoView({behavior: 'smooth', block: 'start'});
  } catch (error) {
    resultBox.textContent = error.message;
    resultBox.classList.add('fail');
    setStatus(error.message, 'error');
  } finally {
    button.disabled = false;
  }
}
