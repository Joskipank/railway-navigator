#ifndef ALGORITHM_HPP
#define ALGORITHM_HPP

#include <vector>
#include <array>
#include <cstdint>

#include "graph.hpp"
#include "parser.hpp"   // ModelParams, Request

// -------------------- DFS: компоненты связности --------------------
// mode_filter:
//   -1  => использовать все ребра (объединенный граф)
//   0/1/2 => использовать только ребра данного вида транспорта
std::vector<std::vector<int>> get_components(const Graph& g, int mode_filter);

// Возвращает "изолированные зоны": все компоненты кроме крупнейшей,
// отсортированные по невозрастанию размера, а внутри станции по возрастанию
std::vector<std::vector<int>> get_isolated_zones(const Graph& g, int mode_filter);


// -------------------- Маршруты / Дейкстра --------------------
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

// Результат Дейкстры "по состояниям": (vertex, last_mode)
struct DijkstraStateResult {
    // dist_time[v][m], dist_tr[v][m]
    std::vector<std::array<double, 3>> dist_time;
    std::vector<std::array<int, 3>> dist_transfers;

    // предки для восстановления маршрута к конкретному (v,m)
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
