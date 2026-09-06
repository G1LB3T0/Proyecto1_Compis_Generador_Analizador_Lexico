const editor = document.getElementById('source-editor');
const filenameInput = document.getElementById('filename');
const statusBox = document.getElementById('status');
let latestResult = null;
const SVG_NAMESPACE = 'http://www.w3.org/2000/svg';
const treeGraph = {
  root: null,
  collapsed: new Set(),
  zoom: 1,
  viewport: null,
  stage: null,
  zoomLabel: null,
  width: 0,
  height: 0,
};

document.addEventListener('DOMContentLoaded', async () => {
  bindEvents();
  await refreshFiles();
  const select = document.getElementById('example-select');
  if (select.value) await loadExample(select.value);
  updateCursor();
});

function bindEvents() {
  document.getElementById('load-example').addEventListener('click', () => {
    const name = document.getElementById('example-select').value;
    if (name) loadExample(name);
  });
  document.getElementById('new-file').addEventListener('click', newFile);
  document.getElementById('local-file').addEventListener('change', loadLocalFile);
  document.getElementById('save-file').addEventListener('click', saveFile);
  document.getElementById('download-file').addEventListener('click', downloadFile);
  document.getElementById('analyze-button').addEventListener('click', analyze);
  document.getElementById('tests-button').addEventListener('click', runTests);
  filenameInput.addEventListener('input', updateTabName);
  editor.addEventListener('keyup', updateCursor);
  editor.addEventListener('click', updateCursor);
  editor.addEventListener('input', () => setStatus('Cambios sin analizar.'));
  editor.addEventListener('keydown', event => {
    if (event.key === 'Tab') {
      event.preventDefault();
      const start = editor.selectionStart;
      editor.setRangeText('  ', start, editor.selectionEnd, 'end');
      updateCursor();
    }
    if ((event.ctrlKey || event.metaKey) && event.key === 'Enter') analyze();
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
  const raw = filenameInput.value.trim() || 'program.cps';
  return raw.endsWith('.cps') ? raw : `${raw}.cps`;
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
  filenameInput.value = 'program.cps';
  editor.value = '';
  latestResult = null;
  document.getElementById('summary').classList.add('hidden');
  document.getElementById('results').classList.add('hidden');
  updateTabName();
  editor.focus();
  setStatus('Nuevo archivo .cps.');
}

function loadLocalFile(event) {
  const file = event.target.files[0];
  if (!file) return;
  if (!file.name.endsWith('.cps')) return setStatus('Selecciona un archivo .cps.', 'error');
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

async function analyze() {
  const button = document.getElementById('analyze-button');
  button.disabled = true;
  setStatus('ANTLR y el Visitor C++ están analizando…');
  try {
    const response = await fetch('/api/analyze', {
      method: 'POST',
      headers: {'Content-Type': 'application/json'},
      body: JSON.stringify({filename: normalizedFilename(), source: editor.value}),
    });
    const data = await response.json();
    if (data.error) throw new Error(data.error);
    latestResult = data;
    renderResult(data);
    setStatus(
      data.ok ? 'Análisis completado sin errores.' : `Análisis completado con ${data.diagnostics.length} diagnóstico(s).`,
      data.ok ? 'success' : 'error',
    );
  } catch (error) {
    setStatus(error.message || 'No se pudo ejecutar el análisis.', 'error');
  } finally {
    button.disabled = false;
  }
}

function renderResult(data) {
  renderSummary(data);
  renderDiagnostics(data.diagnostics || []);
  renderTree(data.tree);
  renderSymbols(data.semantic || {});
  renderTypeSystem(data.semantic || {});
  renderScopeManagement(data.semantic || {});
  renderTokens(data.tokens || []);
  renderClasses(data.semantic?.classes || [], data.semantic?.symbols || []);
  document.getElementById('summary').classList.remove('hidden');
  document.getElementById('results').classList.remove('hidden');
  selectPanel('diagnostics-panel');
}

function renderSummary(data) {
  const summary = data.summary;
  const cards = [
    ['Sintaxis', data.syntax_ok ? 'Correcta' : 'Con errores', data.syntax_ok],
    ['Semántica', data.semantic_ok ? 'Correcta' : (data.semantic?.skipped ? 'Omitida' : 'Con errores'), data.semantic_ok],
    ['Tokens', summary.tokens, true],
    ['Símbolos', summary.symbols, true],
    ['Ámbitos', summary.scopes, true],
  ];
  const container = document.getElementById('summary');
  container.replaceChildren(...cards.map(([label, value, good]) => {
    const card = document.createElement('div');
    card.className = `summary-card ${good ? 'good' : 'bad'}`;
    const caption = document.createElement('span');
    caption.textContent = label;
    const strong = document.createElement('strong');
    strong.textContent = value;
    card.append(caption, strong);
    return card;
  }));
}

function renderDiagnostics(diagnostics) {
  document.getElementById('diagnostic-count').textContent = diagnostics.length;
  const panel = document.getElementById('diagnostics-panel');
  panel.replaceChildren();
  if (!diagnostics.length) {
    const banner = document.createElement('div');
    banner.className = 'success-banner';
    banner.textContent = '✓ No se encontraron errores léxicos, sintácticos ni semánticos.';
    panel.append(banner);
    return;
  }
  for (const item of diagnostics) {
    const row = document.createElement('div');
    row.className = 'diagnostic';
    row.addEventListener('click', () => jumpTo(item.line, item.column));
    const code = document.createElement('span');
    code.className = 'diagnostic-code';
    code.textContent = item.code;
    const message = document.createElement('div');
    message.className = 'diagnostic-message';
    const category = document.createElement('strong');
    category.textContent = item.category;
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
  const lines = editor.value.split('\n');
  let offset = 0;
  for (let index = 0; index < Math.max(0, line - 1) && index < lines.length; index++) {
    offset += lines[index].length + 1;
  }
  offset += Math.max(0, column - 1);
  editor.focus();
  editor.setSelectionRange(offset, offset);
  editor.scrollIntoView({behavior: 'smooth', block: 'center'});
  updateCursor();
}

function renderTree(tree) {
  const panel = document.getElementById('tree-panel');
  panel.replaceChildren();
  if (!tree) return panel.append(emptyMessage('No hay árbol sintáctico disponible.'));

  treeGraph.root = indexTree(tree);
  treeGraph.collapsed = new Set();
  treeGraph.zoom = 1;
  setTreeOverview(treeGraph.root);

  const toolbar = document.createElement('div');
  toolbar.className = 'tree-toolbar';

  const actions = document.createElement('div');
  actions.className = 'tree-actions';
  actions.append(
    treeButton('Vista general', () => {
      treeGraph.collapsed.clear();
      setTreeOverview(treeGraph.root);
      drawTreeGraph();
    }),
    treeButton('Expandir todo', () => {
      treeGraph.collapsed.clear();
      drawTreeGraph();
    }),
    treeButton('Contraer', () => {
      treeGraph.collapsed.clear();
      treeGraph.collapsed.add(treeGraph.root.id);
      drawTreeGraph();
    }),
  );

  const help = document.createElement('div');
  help.className = 'tree-help';
  help.append(
    treeLegend('rule', 'Regla'),
    treeLegend('token', 'Token'),
  );
  const hint = document.createElement('span');
  hint.className = 'tree-hint';
  hint.textContent = 'Clic en un nodo para explorar';
  help.append(hint);

  const zoom = document.createElement('div');
  zoom.className = 'tree-zoom';
  const zoomOut = treeButton('−', () => changeTreeZoom(-0.1), 'Alejar');
  const zoomIn = treeButton('+', () => changeTreeZoom(0.1), 'Acercar');
  treeGraph.zoomLabel = document.createElement('span');
  treeGraph.zoomLabel.textContent = '100%';
  zoom.append(zoomOut, treeGraph.zoomLabel, zoomIn);

  toolbar.append(actions, help, zoom);

  treeGraph.viewport = document.createElement('div');
  treeGraph.viewport.className = 'tree-viewport';
  treeGraph.stage = document.createElement('div');
  treeGraph.stage.className = 'tree-stage';
  treeGraph.viewport.append(treeGraph.stage);
  panel.append(toolbar, treeGraph.viewport);
  drawTreeGraph();
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

function setTreeOverview(node) {
  if (node.children.length && node.depth >= 2) treeGraph.collapsed.add(node.id);
  node.children.forEach(setTreeOverview);
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

function changeTreeZoom(delta) {
  treeGraph.zoom = Math.min(1.8, Math.max(0.5, treeGraph.zoom + delta));
  applyTreeZoom();
}

function applyTreeZoom() {
  if (!treeGraph.stage) return;
  treeGraph.stage.style.width = `${treeGraph.width * treeGraph.zoom}px`;
  treeGraph.stage.style.height = `${treeGraph.height * treeGraph.zoom}px`;
  const svg = treeGraph.stage.querySelector('svg');
  if (svg) {
    svg.style.width = `${treeGraph.width * treeGraph.zoom}px`;
    svg.style.height = `${treeGraph.height * treeGraph.zoom}px`;
  }
  if (treeGraph.zoomLabel)
    treeGraph.zoomLabel.textContent = `${Math.round(treeGraph.zoom * 100)}%`;
}

function visibleChildren(node) {
  return treeGraph.collapsed.has(node.id) ? [] : node.children;
}

function treeNodeWidth(node) {
  const label = node.kind === 'token' ? node.text : node.name;
  return Math.min(190, Math.max(116, 44 + String(label).length * 7));
}

function measureTree(node, metrics) {
  const children = visibleChildren(node);
  const ownWidth = treeNodeWidth(node) + 28;
  if (!children.length) {
    metrics.set(node.id, {subtreeWidth: ownWidth, nodeWidth: treeNodeWidth(node)});
    return ownWidth;
  }
  const childrenWidth = children.reduce((total, child, index) =>
    total + measureTree(child, metrics) + (index ? 22 : 0), 0);
  const subtreeWidth = Math.max(ownWidth, childrenWidth);
  metrics.set(node.id, {subtreeWidth, nodeWidth: treeNodeWidth(node)});
  return subtreeWidth;
}

function positionTree(node, left, depth, metrics, positions) {
  const metric = metrics.get(node.id);
  const children = visibleChildren(node);
  let center = left + metric.subtreeWidth / 2;
  if (children.length) {
    const childrenWidth = children.reduce((total, child, index) =>
      total + metrics.get(child.id).subtreeWidth + (index ? 22 : 0), 0);
    let childLeft = left + (metric.subtreeWidth - childrenWidth) / 2;
    const childCenters = [];
    for (const child of children) {
      positionTree(child, childLeft, depth + 1, metrics, positions);
      childCenters.push(positions.get(child.id).x);
      childLeft += metrics.get(child.id).subtreeWidth + 22;
    }
    center = (childCenters[0] + childCenters.at(-1)) / 2;
  }
  positions.set(node.id, {
    x: center,
    y: 38 + depth * 106,
    width: metric.nodeWidth,
    depth,
  });
}

function drawTreeGraph() {
  if (!treeGraph.root || !treeGraph.stage) return;
  const metrics = new Map();
  const positions = new Map();
  const measuredWidth = measureTree(treeGraph.root, metrics);
  treeGraph.width = Math.max(720, measuredWidth + 56);
  positionTree(treeGraph.root, (treeGraph.width - measuredWidth) / 2, 0,
    metrics, positions);
  const maxDepth = Math.max(...Array.from(positions.values(), item => item.depth));
  treeGraph.height = Math.max(250, 116 + maxDepth * 106);

  const svg = svgElement('svg', {
    class: 'syntax-tree-svg',
    viewBox: `0 0 ${treeGraph.width} ${treeGraph.height}`,
    role: 'img',
    'aria-label': 'Árbol sintáctico gráfico de Compiscript',
  });
  const edges = svgElement('g', {class: 'tree-edges'});
  const nodes = svgElement('g', {class: 'tree-nodes'});
  svg.append(edges, nodes);
  appendTreeSvg(treeGraph.root, positions, edges, nodes);
  treeGraph.stage.replaceChildren(svg);
  applyTreeZoom();
}

function appendTreeSvg(node, positions, edges, nodes) {
  const position = positions.get(node.id);
  const children = visibleChildren(node);
  for (const child of children) {
    const childPosition = positions.get(child.id);
    const path = svgElement('path', {
      d: `M ${position.x} ${position.y + 28} C ${position.x} ${position.y + 66}, ${childPosition.x} ${childPosition.y - 38}, ${childPosition.x} ${childPosition.y - 28}`,
    });
    edges.append(path);
    appendTreeSvg(child, positions, edges, nodes);
  }

  const expandable = node.children.length > 0;
  const collapsed = treeGraph.collapsed.has(node.id);
  const group = svgElement('g', {
    class: `tree-graph-node ${node.kind}${expandable ? ' expandable' : ''}`,
    transform: `translate(${position.x}, ${position.y})`,
    tabindex: expandable ? '0' : '-1',
    role: expandable ? 'button' : 'img',
    'aria-label': `${node.kind === 'token' ? 'Token' : 'Regla'} ${node.name}`,
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
    ? `${node.category || node.name}: ${node.text} · ANTLR ${node.name} (${node.line}:${node.column})`
    : `${node.name} (${node.line}:${node.column})`;
  shape.append(title);

  const mainText = svgElement('text', {class: 'tree-node-title', y: '-4'});
  mainText.textContent = shortened(node.kind === 'token' ? node.text : node.name, 22);
  const detail = svgElement('text', {class: 'tree-node-detail', y: '14'});
  detail.textContent = node.kind === 'token'
    ? shortened(node.category || node.name, 22)
    : `${node.line}:${node.column} · ${node.children.length} hijo${node.children.length === 1 ? '' : 's'}`;
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
    const toggle = () => toggleTreeNode(node);
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

function toggleTreeNode(node) {
  if (treeGraph.collapsed.has(node.id)) {
    let branch = node;
    treeGraph.collapsed.delete(branch.id);
    while (branch.children.length === 1 && branch.children[0].kind === 'rule') {
      branch = branch.children[0];
      treeGraph.collapsed.delete(branch.id);
    }
  } else {
    treeGraph.collapsed.add(node.id);
  }
  drawTreeGraph();
}

function shortened(value, limit) {
  const text = String(value);
  return text.length > limit ? `${text.slice(0, limit - 1)}…` : text;
}

function svgElement(name, attributes = {}) {
  const element = document.createElementNS(SVG_NAMESPACE, name);
  for (const [key, value] of Object.entries(attributes))
    element.setAttribute(key, value);
  return element;
}

function renderSymbols(semantic) {
  const panel = document.getElementById('symbols-panel');
  panel.replaceChildren();
  const scopes = semantic.scopes || [];
  const symbols = new Map((semantic.symbols || []).map(symbol => [symbol.id, symbol]));
  if (!scopes.length) return panel.append(emptyMessage(semantic.reason || 'No se construyó la tabla de símbolos.'));
  const scopeMap = new Map(scopes.map(scope => [scope.id, scope]));
  panel.append(createScope(scopeMap.get(0), scopeMap, symbols));
}

function createScope(scope, scopeMap, symbols) {
  const wrapper = document.createElement('div');
  wrapper.className = 'scope';
  const header = document.createElement('div');
  header.className = 'scope-header';
  const kind = document.createElement('span');
  kind.className = 'scope-kind';
  kind.textContent = scope.kind;
  const name = document.createElement('span');
  name.className = 'scope-name';
  name.textContent = scope.name;
  const parent = document.createElement('span');
  parent.className = 'scope-parent';
  parent.textContent = `scope #${scope.id} · padre ${scope.parent_id}`;
  header.append(kind, name, parent);
  wrapper.append(header);
  if (scope.symbol_ids.length) wrapper.append(symbolTable(scope.symbol_ids.map(id => symbols.get(id)).filter(Boolean), symbols));
  for (const childId of scope.children) {
    const child = document.createElement('div');
    child.className = 'scope-children';
    child.append(createScope(scopeMap.get(childId), scopeMap, symbols));
    wrapper.append(child);
  }
  return wrapper;
}

function symbolTable(items, symbols) {
  const table = makeTable(['Nombre', 'Clase', 'Tipo / firma', 'Estado', 'Capturas', 'Ubicación']);
  const body = table.tBodies[0];
  for (const symbol of items) {
    const signature = symbol.kind === 'function' || symbol.kind === 'method'
      ? `(${symbol.parameters.join(', ')}) → ${symbol.return_type}` : symbol.type;
    const captures = symbol.captures.map(id => symbols.get(id)?.name || `#${id}`);
    addRow(body, [
      codeCell(symbol.name),
      symbolRole(symbol.kind),
      codeCell(signature),
      symbol.mutable ? (symbol.initialized ? 'mutable · inicializado' : 'mutable · no inicializado') : 'solo lectura',
      captures.length ? captures.join(', ') : '—',
      `${symbol.line}:${symbol.column}`,
    ]);
  }
  return table;
}

function semanticNote(title, text) {
  const note = document.createElement('div');
  note.className = 'semantic-note';
  const heading = document.createElement('strong');
  heading.textContent = title;
  const explanation = document.createElement('p');
  explanation.textContent = text;
  note.append(heading, explanation);
  return note;
}

function subsectionTitle(title, description) {
  const heading = document.createElement('div');
  heading.className = 'semantic-section-title';
  const name = document.createElement('h3');
  name.textContent = title;
  const detail = document.createElement('p');
  detail.textContent = description;
  heading.append(name, detail);
  return heading;
}

function symbolType(symbol) {
  if (symbol.kind === 'function' || symbol.kind === 'method')
    return `(${(symbol.parameters || []).join(', ')}) → ${symbol.return_type}`;
  return symbol.type || 'unknown';
}

function symbolRole(kind) {
  return ({
    variable: 'Variable',
    constant: 'Constante',
    parameter: 'Parámetro',
    function: 'Función',
    method: 'Método',
    field: 'Atributo',
    class: 'Clase',
  })[kind] || kind;
}

function typeDescription(type) {
  if (type.endsWith('[]')) return ['Lista', `Colección cuyos elementos son ${type.slice(0, -2)}.`, 'Se asigna a otra lista con elementos compatibles.'];
  const descriptions = {
    integer: ['Primitivo numérico', 'Números enteros; admite aritmética y comparaciones.', 'Puede usarse donde se espera integer o promoverse a float.'],
    float: ['Primitivo numérico', 'Números con decimales; admite aritmética y comparaciones.', 'Acepta valores float y también integer por promoción.'],
    string: ['Primitivo textual', 'Cadenas de texto; permite concatenación con +.', 'Solo es compatible con string; también puede recibir null.'],
    boolean: ['Primitivo lógico', 'Valores true o false usados por condiciones y operadores lógicos.', 'Solo es compatible con boolean.'],
    null: ['Valor especial', 'Representa ausencia de un valor.', 'Puede asignarse a string, listas y objetos.'],
    void: ['Ausencia de retorno', 'Indica que una función no devuelve un valor.', 'No puede utilizarse como tipo de una variable.'],
    unknown: ['Tipo pendiente', 'El tipo no pudo determinarse con seguridad.', 'Evita diagnósticos en cascada después de otro error.'],
  };
  return descriptions[type] || ['Clase del programa', `Objeto perteneciente a la clase ${type}.`, 'Es compatible con su propia clase y con clases base según la herencia.'];
}

function renderTypeSystem(semantic) {
  const panel = document.getElementById('types-panel');
  panel.replaceChildren();
  if (semantic.skipped) {
    panel.append(emptyMessage(semantic.reason || 'La sintaxis debe ser válida para determinar los tipos.'));
    return;
  }

  panel.append(semanticNote(
    '¿Cuándo se construye esta información?',
    'Se actualiza cada vez que analizas un archivo .cps. El Visitor C++ determina los tipos de declaraciones, parámetros, retornos, listas y objetos mientras recorre el árbol sintáctico.',
  ));

  const symbols = semantic.symbols || [];
  const scopes = new Map((semantic.scopes || []).map(scope => [scope.id, scope]));
  const occurrences = new Map();
  const registerType = type => {
    if (!type || ['function', 'class'].includes(type)) return;
    occurrences.set(type, (occurrences.get(type) || 0) + 1);
  };
  symbols.forEach(symbol => {
    registerType(symbol.type);
    (symbol.parameters || []).forEach(registerType);
    if (symbol.kind === 'function' || symbol.kind === 'method') registerType(symbol.return_type);
  });

  const coreTypes = ['integer', 'float', 'string', 'boolean', 'null', 'void'];
  const discoveredTypes = [...occurrences.keys()].filter(type => !coreTypes.includes(type)).sort();
  const allTypes = [...coreTypes, ...discoveredTypes];
  panel.append(subsectionTitle(
    'Catálogo de tipos activo',
    'Incluye los tipos base del lenguaje y los tipos adicionales encontrados en el programa analizado.',
  ));
  const catalog = makeTable(['Tipo', 'Categoría', 'Significado sencillo', 'Compatibilidad principal', 'Usos en este .cps']);
  allTypes.forEach(type => {
    const [category, meaning, compatibility] = typeDescription(type);
    addRow(catalog.tBodies[0], [codeCell(type), category, meaning, compatibility, occurrences.get(type) || 0]);
  });
  panel.append(catalog);

  panel.append(subsectionTitle(
    'Tipos determinados en el programa',
    'Esta tabla usa los símbolos producidos realmente por el análisis actual; una firma muestra parámetros → retorno.',
  ));
  if (!symbols.length) {
    panel.append(emptyMessage('El programa no contiene declaraciones con tipos para mostrar.'));
    return;
  }
  const resolved = makeTable(['Identificador', 'Rol', 'Tipo o firma determinada', 'Entorno', 'Línea']);
  symbols.forEach(symbol => addRow(resolved.tBodies[0], [
    codeCell(symbol.name),
    symbolRole(symbol.kind),
    codeCell(symbolType(symbol)),
    scopes.get(symbol.scope_id)?.name || `scope #${symbol.scope_id}`,
    symbol.line,
  ]));
  panel.append(resolved);
}

function scopeKind(kind) {
  return ({global: 'Global', function: 'Función', class: 'Clase', block: 'Bloque'})[kind] || kind;
}

function renderScopeManagement(semantic) {
  const panel = document.getElementById('scopes-panel');
  panel.replaceChildren();
  const scopes = semantic.scopes || [];
  const symbols = new Map((semantic.symbols || []).map(symbol => [symbol.id, symbol]));
  if (!scopes.length) {
    panel.append(emptyMessage(semantic.reason || 'No se crearon ámbitos porque el análisis semántico no pudo ejecutarse.'));
    return;
  }

  panel.append(semanticNote(
    '¿Qué demuestra esta tabla?',
    'Cada fila es un entorno creado realmente por el Visitor C++. Para resolver un nombre se revisa primero el entorno actual y, si no aparece, se continúa con su padre hasta llegar al ámbito global.',
  ));
  const table = makeTable(['ID', 'Entorno', 'Tipo', 'Padre', 'Propietario', 'Símbolos declarados', 'Línea']);
  scopes.forEach(scope => {
    const owner = symbols.get(scope.owner_symbol_id);
    const names = (scope.symbol_ids || []).map(id => symbols.get(id)?.name).filter(Boolean);
    addRow(table.tBodies[0], [
      codeCell(`#${scope.id}`),
      scope.name,
      scopeKind(scope.kind),
      scope.parent_id < 0 ? '— (raíz)' : `#${scope.parent_id}`,
      owner?.name || '—',
      names.length ? names.join(', ') : '—',
      scope.line,
    ]);
  });
  panel.append(table);

  const legend = document.createElement('div');
  legend.className = 'scope-explanation-grid';
  legend.append(
    semanticNote('Global', 'Es el entorno raíz. Sus nombres pueden consultarse desde los entornos descendientes.'),
    semanticNote('Función', 'Guarda parámetros y variables locales. Al terminar la función, esos nombres dejan de estar disponibles.'),
    semanticNote('Clase', 'Agrupa atributos y métodos pertenecientes a una clase.'),
    semanticNote('Bloque', 'Se crea para llaves, ciclos y catch; permite controlar visibilidad y sombreado de nombres.'),
  );
  panel.append(legend);
}

function renderTokens(tokens) {
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

function renderClasses(classes, symbols) {
  const panel = document.getElementById('classes-panel');
  panel.replaceChildren();
  if (!classes.length) return panel.append(emptyMessage('El programa no declara clases.'));
  const byId = new Map(symbols.map(symbol => [symbol.id, symbol]));
  const grid = document.createElement('div');
  grid.className = 'class-grid';
  for (const item of classes) {
    const card = document.createElement('article');
    card.className = 'class-card';
    const title = document.createElement('h3');
    title.textContent = item.name;
    const base = document.createElement('p');
    base.textContent = item.base ? `Hereda de ${item.base}` : 'Sin clase base';
    card.append(title, base, tagList('Campos', Object.values(item.fields).map(id => byId.get(id)?.name)), tagList('Métodos', Object.values(item.methods).map(id => byId.get(id)?.name)));
    grid.append(card);
  }
  panel.append(grid);
}

function tagList(label, values) {
  const section = document.createElement('div');
  const caption = document.createElement('span');
  caption.className = 'muted';
  caption.textContent = `${label}: `;
  section.append(caption);
  if (!values.filter(Boolean).length) section.append('—');
  for (const value of values.filter(Boolean)) {
    const tag = document.createElement('span');
    tag.className = 'tag';
    tag.textContent = value;
    section.append(tag);
  }
  return section;
}

function makeTable(headers) {
  const table = document.createElement('table');
  table.className = 'data-table';
  const head = table.createTHead().insertRow();
  headers.forEach(text => {
    const cell = document.createElement('th');
    cell.textContent = text;
    head.append(cell);
  });
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
  const empty = document.createElement('div');
  empty.className = 'empty-result';
  empty.textContent = text;
  return empty;
}

function selectPanel(panelId) {
  document.querySelectorAll('.tab').forEach(tab => tab.classList.toggle('active', tab.dataset.panel === panelId));
  document.querySelectorAll('.result-panel').forEach(panel => panel.classList.toggle('active', panel.id === panelId));
}

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
  title.textContent = data.ok
    ? 'Batería semántica aprobada'
    : 'La batería encontró fallos';
  const explanation = document.createElement('p');
  explanation.textContent = 'Un “error esperado” aprueba únicamente si el analizador produce el diagnóstico correcto; no basta con que el programa falle.';
  titleGroup.append(title, explanation);

  const actions = document.createElement('div');
  actions.className = 'test-report-actions';
  const expandButton = document.createElement('button');
  expandButton.type = 'button';
  expandButton.className = 'small secondary';
  expandButton.textContent = 'Expandir todo';
  const collapseButton = document.createElement('button');
  collapseButton.type = 'button';
  collapseButton.className = 'small secondary';
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
      const explanation = testDetail('¿Qué está pasando?', test.explanation);
      explanation.classList.add('test-explanation');
      body.append(
        testDetail('Código Compiscript probado', test.source, true),
        explanation,
        testDetail('Resultado esperado', test.expected),
        testDetail('Resultado obtenido', test.actual),
      );
      if ((test.codes || []).length) {
        const diagnostics = document.createElement('div');
        diagnostics.className = 'test-detail test-diagnostics';
        const diagnosticsTitle = document.createElement('strong');
        diagnosticsTitle.textContent = 'Diagnósticos producidos';
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
  } catch (error) {
    resultBox.textContent = error.message;
    resultBox.classList.add('fail');
    setStatus(error.message, 'error');
  } finally {
    button.disabled = false;
  }
}
