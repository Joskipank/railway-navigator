(() => {
  const inputEl = document.getElementById("graph-input");
  const routeEl = document.getElementById("route-input");
  const buildBtn = document.getElementById("build-btn");
  const highlightBtn = document.getElementById("highlight-btn");
  const clearBtn = document.getElementById("clear-btn");
  const loadExampleBtn = document.getElementById("load-example");
  const svg = document.getElementById("graph");
  const statusEl = document.getElementById("status");

  const MODE_CLASSES = {
    0: "metro",
    1: "bus",
    2: "rail",
  };

  const MODE_FROM_TEXT = {
    metro: 0,
    bus: 1,
    rail: 2,
  };

  const HIGHLIGHT_DELAY = 700;
  const BACKEND_ENDPOINT = "/api/run";

  let graphState = null;
  let highlightTimer = null;

  function setStatus(message) {
    if (!statusEl) {
      return;
    }
    statusEl.textContent = message;
  }

  function canUpdateBuildStatus() {
    return statusEl && statusEl.textContent === "Граф построен";
  }

  function firstLine(text) {
    if (!text) {
      return "";
    }
    const line = String(text).split(/\r?\n/)[0];
    return line.trim();
  }

  function extractFirstRoute(output) {
    if (!output) {
      return "";
    }
    const pattern = /Path:\s*([^\r\n]+)/g;
    let match = null;
    while ((match = pattern.exec(output)) !== null) {
      const path = match[1].trim();
      if (path && path !== "unreachable") {
        return path;
      }
    }
    return "";
  }

  async function requestBackend(inputText) {
    const response = await fetch(BACKEND_ENDPOINT, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ input: inputText }),
    });

    let payload = {};
    try {
      payload = await response.json();
    } catch (error) {
      payload = {};
    }

    if (!response.ok) {
      const message = firstLine(payload.error || `HTTP ${response.status}`);
      throw new Error(message || "backend error");
    }

    return payload;
  }

  async function syncBackend(inputText) {
    if (!inputText || !inputText.trim()) {
      return;
    }

    try {
      const result = await requestBackend(inputText);
      if (!result.ok) {
        const message = firstLine(result.error || result.stderr || "backend error");
        if (message && canUpdateBuildStatus()) {
          setStatus(`Ошибка backend: ${message}`);
        }
        return;
      }

      const routeFromBackend = extractFirstRoute(result.stdout);
      if (routeFromBackend && routeEl && !routeEl.value.trim()) {
        routeEl.value = routeFromBackend;
        if (canUpdateBuildStatus()) {
          setStatus("Граф построен (маршрут из backend)");
        }
        return;
      }

      if (canUpdateBuildStatus()) {
        setStatus("Граф построен (backend ok)");
      }
    } catch (error) {
      console.warn("Backend unavailable", error);
    }
  }

  function getSvgSize() {
    const viewBox = svg.getAttribute("viewBox");
    if (viewBox) {
      const parts = viewBox.split(/\s+/).map((value) => Number(value));
      if (parts.length === 4 && parts.every((value) => Number.isFinite(value))) {
        return { width: parts[2], height: parts[3] };
      }
    }
    return { width: 900, height: 600 };
  }

  function clearSvg() {
    while (svg.firstChild) {
      svg.removeChild(svg.firstChild);
    }
  }

  function parseInput(text) {
    const tokens = text.trim().split(/\s+/).filter(Boolean);
    if (tokens.length === 0) {
      return { ok: false, error: "Введите входные данные" };
    }

    let idx = 0;
    const readNumber = () => {
      if (idx >= tokens.length) {
        return null;
      }
      const value = Number(tokens[idx]);
      idx += 1;
      return Number.isFinite(value) ? value : null;
    };

    const readInt = () => {
      const value = readNumber();
      if (value === null || !Number.isInteger(value)) {
        return null;
      }
      return value;
    };

    const n = readInt();
    const m = readInt();
    if (n === null || m === null || n <= 0 || m < 0) {
      return { ok: false, error: "Ошибка парсинга: некорректные N и M" };
    }

    for (let i = 0; i < 3; i += 1) {
      if (readNumber() === null) {
        return { ok: false, error: "Ошибка парсинга: sensitivity" };
      }
    }

    for (let i = 0; i < 9; i += 1) {
      if (readNumber() === null) {
        return { ok: false, error: "Ошибка парсинга: матрица 3x3" };
      }
    }

    for (let i = 0; i < n; i += 1) {
      if (readNumber() === null) {
        return { ok: false, error: "Ошибка парсинга: station_transfer" };
      }
    }

    const edges = [];
    for (let i = 0; i < m; i += 1) {
      const u = readInt();
      const v = readInt();
      const mode = readInt();
      const baseTime = readNumber();
      const load = readNumber();

      if (
        u === null ||
        v === null ||
        mode === null ||
        baseTime === null ||
        load === null
      ) {
        return { ok: false, error: "Ошибка парсинга: ребро" };
      }

      if (u < 1 || u > n || v < 1 || v > n || mode < 0 || mode > 2) {
        return { ok: false, error: "Ошибка парсинга: ребро вне диапазона" };
      }

      edges.push({ u, v, mode, baseTime, load });
    }

    return { ok: true, data: { n, m, edges } };
  }

  function layoutNodes(n, width, height) {
    const positions = new Map();
    const cx = width / 2;
    const cy = height / 2;
    const radius = Math.min(width, height) / 2 - 70;

    for (let i = 1; i <= n; i += 1) {
      const angle = -Math.PI / 2 + (2 * Math.PI * (i - 1)) / n;
      const x = cx + radius * Math.cos(angle);
      const y = cy + radius * Math.sin(angle);
      positions.set(i, { x, y });
    }

    return positions;
  }

  function makeEdgeKey(u, v, mode) {
    const a = Math.min(u, v);
    const b = Math.max(u, v);
    return `${a}-${b}-${mode}`;
  }

  function buildEdgePath(edge, index, total, positions) {
    const p1 = positions.get(edge.u);
    const p2 = positions.get(edge.v);
    if (!p1 || !p2) {
      return "";
    }

    if (edge.u === edge.v) {
      const base = 26;
      const loopRadius = base + index * 8;
      const x = p1.x;
      const y = p1.y;
      return `M ${x + loopRadius} ${y} A ${loopRadius} ${loopRadius} 0 1 1 ${
        x
      } ${y - loopRadius} A ${loopRadius} ${loopRadius} 0 1 1 ${
        x + loopRadius
      } ${y}`;
    }

    const dx = p2.x - p1.x;
    const dy = p2.y - p1.y;
    const mx = (p1.x + p2.x) / 2;
    const my = (p1.y + p2.y) / 2;

    const spacing = 18;
    const offset = total > 1 ? (index - (total - 1) / 2) * spacing : 0;
    const length = Math.hypot(dx, dy) || 1;
    const nx = -dy / length;
    const ny = dx / length;

    const cx = mx + nx * offset;
    const cy = my + ny * offset;

    return `M ${p1.x} ${p1.y} Q ${cx} ${cy} ${p2.x} ${p2.y}`;
  }

  function renderGraph(data) {
    clearSvg();

    const size = getSvgSize();
    const positions = layoutNodes(data.n, size.width, size.height);
    const edgeMap = new Map();

    const edgesGroup = document.createElementNS(
      "http://www.w3.org/2000/svg",
      "g"
    );
    edgesGroup.setAttribute("class", "edges");

    const grouped = new Map();
    data.edges.forEach((edge) => {
      const key = `${Math.min(edge.u, edge.v)}-${Math.max(edge.u, edge.v)}`;
      if (!grouped.has(key)) {
        grouped.set(key, []);
      }
      grouped.get(key).push(edge);
    });

    grouped.forEach((edges) => {
      edges.forEach((edge, index) => {
        const total = edges.length;
        const path = document.createElementNS(
          "http://www.w3.org/2000/svg",
          "path"
        );
        path.setAttribute("d", buildEdgePath(edge, index, total, positions));
        path.classList.add("edge");

        const modeClass = MODE_CLASSES[edge.mode] || "";
        if (modeClass) {
          path.classList.add(modeClass);
        }

        path.dataset.u = String(edge.u);
        path.dataset.v = String(edge.v);
        path.dataset.mode = String(edge.mode);

        edgesGroup.appendChild(path);

        const edgeKey = makeEdgeKey(edge.u, edge.v, edge.mode);
        if (!edgeMap.has(edgeKey)) {
          edgeMap.set(edgeKey, []);
        }
        edgeMap.get(edgeKey).push(path);
      });
    });

    svg.appendChild(edgesGroup);

    const nodesGroup = document.createElementNS(
      "http://www.w3.org/2000/svg",
      "g"
    );
    nodesGroup.setAttribute("class", "nodes");

    positions.forEach((pos, nodeId) => {
      const circle = document.createElementNS(
        "http://www.w3.org/2000/svg",
        "circle"
      );
      circle.setAttribute("cx", pos.x);
      circle.setAttribute("cy", pos.y);
      circle.setAttribute("r", 16);
      circle.classList.add("node");

      const label = document.createElementNS(
        "http://www.w3.org/2000/svg",
        "text"
      );
      label.setAttribute("x", pos.x);
      label.setAttribute("y", pos.y);
      label.textContent = String(nodeId);
      label.classList.add("node-label");

      nodesGroup.appendChild(circle);
      nodesGroup.appendChild(label);
    });

    svg.appendChild(nodesGroup);

    graphState = {
      n: data.n,
      edges: data.edges,
      edgeMap,
    };
  }

  function clearHighlight() {
    if (highlightTimer) {
      clearInterval(highlightTimer);
      highlightTimer = null;
    }

    if (!graphState || !graphState.edgeMap) {
      return;
    }

    graphState.edgeMap.forEach((elements) => {
      elements.forEach((element) => {
        element.classList.remove("highlighted");
      });
    });
  }

  function parseRoute(text) {
    const trimmed = text.trim();
    if (!trimmed) {
      return { ok: false, error: "Маршрут не распознан" };
    }

    const pattern = /(\d+)\s*-\[(metro|bus|rail)\]->\s*(\d+)/g;
    let match = null;
    let lastIndex = 0;
    const segments = [];

    while ((match = pattern.exec(trimmed)) !== null) {
      const gap = trimmed.slice(lastIndex, match.index).trim();
      if (gap) {
        return { ok: false, error: "Маршрут не распознан" };
      }
      const u = Number(match[1]);
      const modeText = match[2];
      const v = Number(match[3]);
      if (!Number.isInteger(u) || !Number.isInteger(v)) {
        return { ok: false, error: "Маршрут не распознан" };
      }
      segments.push({ u, v, mode: MODE_FROM_TEXT[modeText] });
      lastIndex = pattern.lastIndex;
    }

    if (trimmed.slice(lastIndex).trim()) {
      return { ok: false, error: "Маршрут не распознан" };
    }

    if (segments.length === 0) {
      return { ok: false, error: "Маршрут не распознан" };
    }

    return { ok: true, segments };
  }

  function highlightRoute(segments) {
    if (!graphState) {
      setStatus("Сначала постройте граф");
      return;
    }

    clearHighlight();
    let stepIndex = 0;

    const step = () => {
      if (stepIndex >= segments.length) {
        clearInterval(highlightTimer);
        highlightTimer = null;
        setStatus("Подсветка завершена");
        return;
      }

      const segment = segments[stepIndex];
      const edgeKey = makeEdgeKey(segment.u, segment.v, segment.mode);
      const elements = graphState.edgeMap.get(edgeKey);

      if (!elements || elements.length === 0) {
        clearInterval(highlightTimer);
        highlightTimer = null;
        setStatus("Маршрут не найден в графе");
        return;
      }

      elements.forEach((element) => {
        element.classList.add("highlighted");
      });

      stepIndex += 1;
    };

    step();
    highlightTimer = setInterval(step, HIGHLIGHT_DELAY);
  }

  function handleBuildGraph() {
    if (!inputEl) {
      return;
    }

    const parsed = parseInput(inputEl.value);
    if (!parsed.ok) {
      setStatus(parsed.error);
      return;
    }

    clearHighlight();
    renderGraph(parsed.data);
    setStatus("Граф построен");
    syncBackend(inputEl.value);
  }

  function handleHighlightRoute() {
    if (!graphState) {
      setStatus("Сначала постройте граф");
      return;
    }

    const parsed = parseRoute(routeEl ? routeEl.value : "");
    if (!parsed.ok) {
      setStatus(parsed.error);
      return;
    }

    highlightRoute(parsed.segments);
  }

  function handleClear() {
    clearHighlight();
    setStatus("Подсветка очищена");
  }

  function loadExample() {
    if (!inputEl || !routeEl) {
      return;
    }

    inputEl.value = `5 6
0.2 0.3 0.5
0 1 2
1 0 1
2 1 0
0 0 0 0 0
1 2 0 12 0.3
2 3 1 10 0.4
3 4 2 8 0.2
4 5 0 14 0.1
5 1 1 9 0.5
2 4 2 7 0.6`;

    routeEl.value = "1-[metro]->2 2-[bus]->3 3-[rail]->4";
    setStatus("Пример загружен");
  }

  if (buildBtn) {
    buildBtn.addEventListener("click", handleBuildGraph);
  }

  if (highlightBtn) {
    highlightBtn.addEventListener("click", handleHighlightRoute);
  }

  if (clearBtn) {
    clearBtn.addEventListener("click", handleClear);
  }

  if (loadExampleBtn) {
    loadExampleBtn.addEventListener("click", loadExample);
  }

  setStatus("Граф не построен");
})();
