#ifndef ALGORITHM_HPP
#define ALGORITHM_HPP

#include <vector>
#include <array>
#include <cstdint>

#include "models/graph.hpp"
#include "parser.hpp"   // ModelParams, Request

// -------------------- DFS: компоненты связности --------------------
// DFS с цветами, временем открытия/закрытия и деревом предков (CLRS, гл. 22.3).
// Сложность: O(V + E) для представления списками смежности.
std::vector<std::vector<int>> get_connected_components(const Graph& g, TransportType type);
std::vector<int> get_isolated_zones(const Graph& g, TransportType type);


// -------------------- Маршруты / Дейкстра --------------------
// Дейкстра для графа состояний (v, last_mode) с неотрицательными весами.
// Сложность: O((V + E) log V) при двоичной куче (CLRS, гл. 24.3).
struct Step {
    int from = 0;
    int to = 0;
    int mode = 0; // 0..2
};

struct Route {
    int target = 0;
    double time = 0.0;
    int transfers = 0;
    double metric = 0.0;              // time + k * transfers
    bool reachable = false;
    std::vector<Step> steps;          // последовательность переходов
};

// Результат Дейкстры по состояниям (v, last_mode).
struct DijkstraStateResult {
    // d_time[v][m], d_tr[v][m] — оценки расстояний/пересадок.
    std::vector<std::array<double, 3>> dist_time;
    std::vector<std::array<int, 3>> dist_transfers;

    // pi_v[v][m], pi_mode[v][m], pi_edge_mode[v][m] — дерево предков.
    std::vector<std::array<int, 3>> parent_v;
    std::vector<std::array<int, 3>> parent_mode;
    std::vector<std::array<int, 3>> parent_edge_mode; // mode ребра, по которому пришли
};

// Запускает Дейкстру от start.
// Важно: учитывать штрафы пересадки (матрица + локальная пересадка на станции),
// а первую посадку делать без штрафа.
DijkstraStateResult dijkstra_states(
    const Graph& g,
    const ModelParams& model,
    int start
);

// Восстановить лучший маршрут до target (с выбором лучшего среди last_mode=0..2,
// при равном времени — меньшие пересадки)
Route build_route_to_target(
    const DijkstraStateResult& dj,
    const ModelParams& model,
    int start,
    int target,
    double k
);

// Для одного запроса построить маршруты до всех целей
std::vector<Route> solve_request(
    const Graph& g,
    const ModelParams& model,
    const Request& rq
);


// -------------------- Быстрая сортировка (своя) --------------------
// Сортировка маршрутов по правилам:
// metric ↑, time ↑, transfers ↑, target ↑
void quicksort_routes(std::vector<Route>& a, int l, int r);

#endif // ALGORITHM_HPP
