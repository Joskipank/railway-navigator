// backend/include/graph.hpp
#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>
#include <array>
#include <stdexcept>

/*
----------------------------------------------------------------------
ПРЕДСТАВЛЕНИЕ ГРАФА (в терминах CLRS)

Граф G = (V, E) — неориентированный мультиграф. Вершины V соответствуют
станциям; далее используется термин "вершина". Рёбра E моделируют
переходы (метро/автобус/жд). Между одной парой вершин допускаются
параллельные рёбра.

Хранение: списки смежности Adj[u] для каждой вершины u.
Каждое неориентированное ребро {u, v} хранится как два ориентированных
рёбра (u, v) в Adj[u] и (v, u) в Adj[v], как в CLRS.
----------------------------------------------------------------------
*/

// Виды транспорта: 0 метро, 1 автобус, 2 железная дорога
static const int MODE_METRO = 0;
static const int MODE_BUS   = 1;
static const int MODE_RAIL  = 2;

enum class TransportType : int {
    Metro = MODE_METRO,
    Bus = MODE_BUS,
    Rail = MODE_RAIL,
    All = -1
};

struct Edge {
    int to;            // конечная вершина v (ребро (u, v) хранится в Adj[u])
    int mode;          // тип ребра (0..2)
    double base_time;  // вес ребра w(u, v) без учета загрузки/пересадки
    double load;       // коэффициент загрузки [0..1]
    int id;            // идентификатор неориентированного ребра (общий для обеих сторон)
};

struct Graph {
    int n; // |V| — число вершин
    int m; // |E| — число неориентированных ребер
    std::vector<std::vector<Edge>> adj; // Adj[u], u = 1..n
    std::array<std::vector<std::vector<int>>, 3> adjacency; // Adj_mode[u], u = 1..n

    std::vector<std::vector<int>> getConnectedComponents(TransportType type) const;
    std::vector<int> getIsolatedZones(TransportType type) const;
};

/*
----------------------------------------------------------------------
ОПЕРАЦИИ (как "процедуры" в стиле книги)
----------------------------------------------------------------------
*/

// GRAPH-INIT(G, n)
inline void graph_init(Graph& g, int n) {
    if (n < 0) throw std::invalid_argument("graph_init: n must be >= 0");
    g.n = n;
    g.m = 0;
    g.adj.assign(static_cast<std::size_t>(n) + 1, std::vector<Edge>{});
    for (auto& by_mode : g.adjacency) {
        by_mode.assign(static_cast<std::size_t>(n) + 1, std::vector<int>{});
    }
}

// VALID-VERTEX(G, v)
inline bool valid_vertex(const Graph& g, int v) {
    return (1 <= v && v <= g.n);
}

// GRAPH-ADD-UNDIRECTED(G, u, v, mode, base_time, load)
inline void graph_add_undirected(Graph& g, int u, int v, int mode, double base_time, double load) {
    if (!valid_vertex(g, u) || !valid_vertex(g, v))
        throw std::out_of_range("graph_add_undirected: vertex out of range");
    if (mode < 0 || mode > 2)
        throw std::invalid_argument("graph_add_undirected: mode must be 0..2");
    if (base_time < 0.0)
        throw std::invalid_argument("graph_add_undirected: base_time must be >= 0");
    if (load < 0.0 || load > 1.0)
        throw std::invalid_argument("graph_add_undirected: load must be in [0,1]");

    const int id = g.m++; // новый id неориентированного ребра

    // добавить обе стороны (u->v и v->u)
    g.adj[u].push_back(Edge{v, mode, base_time, load, id});
    g.adj[v].push_back(Edge{u, mode, base_time, load, id});
    g.adjacency[static_cast<std::size_t>(mode)][u].push_back(v);
    g.adjacency[static_cast<std::size_t>(mode)][v].push_back(u);
}

/*
Вес ребра w(u, v) с учетом загрузки:
time = base_time * (1 + load * sensitivity[mode]).
*/
inline double edge_time(const Edge& e, const std::array<double, 3>& sensitivity) {
    return e.base_time * (1.0 + e.load * sensitivity[static_cast<std::size_t>(e.mode)]);
}

#endif // GRAPH_HPP
