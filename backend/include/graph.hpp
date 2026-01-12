// backend/include/graph.hpp
#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>
#include <array>
#include <stdexcept>

/*
----------------------------------------------------------------------
ПРЕДСТАВЛЕНИЕ ГРАФА (как в учебниках)

Транспортная сеть моделируется как НЕОРИЕНТИРОВАННЫЙ МУЛЬТИГРАФ:
- вершины: станции 1..N
- ребра: перемещения (метро/автобус/жд)
- между двумя станциями может быть несколько ребер (мульти-рёбра)

Хранение: СПИСКИ СМЕЖНОСТИ.
Для каждой вершины u хранится список Adj[u] ребер (u -> v).
Одно неориентированное ребро (u <-> v) хранится ДВАЖДЫ:
- в Adj[u] как (to=v)
- в Adj[v] как (to=u)

Это соответствует требованию "граф хранить в виде списков смежности;
поддерживать несколько рёбер между одной парой станций". :contentReference[oaicite:1]{index=1}
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
    int to;            // куда ведёт ребро
    int mode;          // 0..2
    double base_time;  // базовое время
    double load;       // загрузка [0..1]
    int id;            // идентификатор неориентированного ребра (один на обе стороны)
};

struct Graph {
    int n; // число вершин
    int m; // число НЕориентированных ребер
    std::vector<std::vector<Edge>> adj; // adj[1..n]
    std::array<std::vector<std::vector<int>>, 3> adjacency; // adjacency[mode][1..n]

    std::vector<std::vector<int>> getConnectedComponents(TransportType type) const;
    std::vector<int> getIsolatedZones(TransportType type) const;
    void dfsIterative(
        int start,
        TransportType type,
        std::vector<bool>& visited,
        std::vector<int>& component
    ) const;
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
Фактическое время на ребре:
time = base_time * (1 + load * sensitivity[mode])

Это ровно из условия. :contentReference[oaicite:2]{index=2}
*/
inline double edge_time(const Edge& e, const std::array<double, 3>& sensitivity) {
    return e.base_time * (1.0 + e.load * sensitivity[static_cast<std::size_t>(e.mode)]);
}

#endif // GRAPH_HPP
