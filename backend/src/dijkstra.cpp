#include "algorithms.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

namespace {

constexpr int kModeCount = 4;
constexpr int kNoMode = 3;
constexpr double kInf = std::numeric_limits<double>::infinity();
constexpr int kInfTransfers = std::numeric_limits<int>::max() / 4;

// Состояние графа расширенных вершин: (v, last_mode).
struct State {
    int v;        // вершина
    int mode;     // последний вид транспорта (0..2) или kNoMode
    double time;  // текущая длина пути
    int transfers; // число пересадок
};

// Компаратор для min-heap по (time, transfers).
struct MinKey {
    bool operator()(const State& a, const State& b) const {
        if (a.time != b.time) {
            return a.time > b.time;
        }
        return a.transfers > b.transfers;
    }
};

using TimeTable = std::vector<std::array<double, kModeCount>>;
using TransferTable = std::vector<std::array<int, kModeCount>>;
using ParentTable = std::vector<std::array<int, kModeCount>>;

struct InternalResult {
    TimeTable time;
    TransferTable transfers;
    ParentTable parent_v;
    ParentTable parent_mode;
    ParentTable parent_edge_mode;
};

bool is_better(double t_new, int tr_new, double t_old, int tr_old) {
    return (t_new < t_old) || (t_new == t_old && tr_new < tr_old);
}

InternalResult run_dijkstra_states(
    const Graph& g,
    int start,
    const std::array<double, 3>& sensitivity,
    const std::array<std::array<double, 3>, 3>& transfer_penalty,
    const std::vector<double>& station_penalty
) {
    InternalResult res;
    res.time.assign(g.n + 1, std::array<double, kModeCount>{kInf, kInf, kInf, kInf});
    res.transfers.assign(g.n + 1, std::array<int, kModeCount>{kInfTransfers, kInfTransfers, kInfTransfers, kInfTransfers});
    res.parent_v.assign(g.n + 1, std::array<int, kModeCount>{-1, -1, -1, -1});
    res.parent_mode.assign(g.n + 1, std::array<int, kModeCount>{-1, -1, -1, -1});
    res.parent_edge_mode.assign(g.n + 1, std::array<int, kModeCount>{-1, -1, -1, -1});

    if (!valid_vertex(g, start)) {
        return res;
    }

    res.time[start][kNoMode] = 0.0;
    res.transfers[start][kNoMode] = 0;

    std::priority_queue<State, std::vector<State>, MinKey> q;
    q.push({start, kNoMode, 0.0, 0});

    while (!q.empty()) {
        const State u = q.top();
        q.pop();

        if (u.time > res.time[u.v][u.mode]) {
            continue;
        }
        if (u.time == res.time[u.v][u.mode] && u.transfers > res.transfers[u.v][u.mode]) {
            continue;
        }

        for (const Edge& e : g.adj[u.v]) {
            const int v = e.to;
            const int mode_v = e.mode;

            double w = edge_time(e, sensitivity);
            int add_transfer = 0;
            if (u.mode != kNoMode && u.mode != mode_v) {
                w += transfer_penalty[u.mode][mode_v] + station_penalty[u.v];
                add_transfer = 1;
            }

            const double new_time = res.time[u.v][u.mode] + w;
            const int new_transfers = res.transfers[u.v][u.mode] + add_transfer;

            if (is_better(new_time, new_transfers, res.time[v][mode_v], res.transfers[v][mode_v])) {
                res.time[v][mode_v] = new_time;
                res.transfers[v][mode_v] = new_transfers;
                res.parent_v[v][mode_v] = u.v;
                res.parent_mode[v][mode_v] = u.mode;
                res.parent_edge_mode[v][mode_v] = mode_v;
                q.push({v, mode_v, new_time, new_transfers});
            }
        }
    }

    return res;
}

bool route_less(const Route& a, const Route& b) {
    if (a.metric != b.metric) {
        return a.metric < b.metric;
    }
    if (a.time != b.time) {
        return a.time < b.time;
    }
    if (a.transfers != b.transfers) {
        return a.transfers < b.transfers;
    }
    return a.target < b.target;
}

} // namespace

// DIJKSTRA-STATE(G, s): алгоритм Дейкстры на графе состояний (v, last_mode).
// Инвариант: после извлечения (u, mode) из очереди приоритетов d[u][mode]
// является длиной кратчайшего пути в графе состояний (все веса неотрицательны).
void dijkstra(
    const Graph& g,
    int start,
    const std::array<double, 3>& sensitivity,
    const std::array<std::array<double, 3>, 3>& transfer_penalty,
    const std::vector<double>& station_penalty,
    std::vector<std::vector<double>>& dist,
    std::vector<std::vector<std::pair<int, int>>>& parent
) {
    const InternalResult res = run_dijkstra_states(g, start, sensitivity, transfer_penalty, station_penalty);

    dist.assign(g.n + 1, std::vector<double>(kModeCount, kInf));
    parent.assign(g.n + 1, std::vector<std::pair<int, int>>(kModeCount, {-1, -1}));

    for (int v = 0; v <= g.n; ++v) {
        for (int m = 0; m < kModeCount; ++m) {
            dist[v][m] = res.time[v][m];
            parent[v][m] = {res.parent_v[v][m], res.parent_mode[v][m]};
        }
    }
}

// Восстановление пути по дереву предков pi.
std::vector<int> restore_path(
    int target,
    int best_mode,
    const std::vector<std::vector<std::pair<int, int>>>& pi
) {
    std::vector<int> path;
    int v = target;
    int m = best_mode;

    while (v != -1) {
        path.push_back(v);
        const auto p = pi[v][m];
        v = p.first;
        m = p.second;
    }

    std::reverse(path.begin(), path.end());
    return path;
}

DijkstraStateResult dijkstra_states(
    const Graph& g,
    const ModelParams& model,
    int start
) {
    const InternalResult res = run_dijkstra_states(
        g,
        start,
        model.sensitivity,
        model.trans,
        model.station_transfer
    );

    DijkstraStateResult out;
    out.dist_time.assign(g.n + 1, std::array<double, 3>{kInf, kInf, kInf});
    out.dist_transfers.assign(g.n + 1, std::array<int, 3>{kInfTransfers, kInfTransfers, kInfTransfers});
    out.parent_v.assign(g.n + 1, std::array<int, 3>{-1, -1, -1});
    out.parent_mode.assign(g.n + 1, std::array<int, 3>{-1, -1, -1});
    out.parent_edge_mode.assign(g.n + 1, std::array<int, 3>{-1, -1, -1});

    for (int v = 0; v <= g.n; ++v) {
        for (int m = 0; m < 3; ++m) {
            out.dist_time[v][m] = res.time[v][m];
            out.dist_transfers[v][m] = res.transfers[v][m];
            out.parent_v[v][m] = res.parent_v[v][m];
            out.parent_mode[v][m] = res.parent_mode[v][m];
            out.parent_edge_mode[v][m] = res.parent_edge_mode[v][m];
        }
    }

    return out;
}

Route build_route_to_target(
    const DijkstraStateResult& dj,
    const ModelParams& model,
    int start,
    int target,
    double k
) {
    static_cast<void>(model);

    Route route;
    route.target = target;

    if (start == target) {
        route.reachable = true;
        route.time = 0.0;
        route.transfers = 0;
        route.metric = 0.0;
        return route;
    }

    int best_mode = -1;
    double best_time = kInf;
    int best_transfers = kInfTransfers;

    if (target >= 0 && target < static_cast<int>(dj.dist_time.size())) {
        for (int m = 0; m < 3; ++m) {
            const double t = dj.dist_time[target][m];
            const int tr = dj.dist_transfers[target][m];
            if (is_better(t, tr, best_time, best_transfers)) {
                best_time = t;
                best_transfers = tr;
                best_mode = m;
            }
        }
    }

    if (best_mode == -1 || !std::isfinite(best_time)) {
        route.reachable = false;
        route.time = kInf;
        route.transfers = kInfTransfers;
        route.metric = kInf;
        return route;
    }

    route.reachable = true;
    route.time = best_time;
    route.transfers = best_transfers;
    route.metric = best_time + k * static_cast<double>(best_transfers);

    std::vector<Step> reverse_steps;
    int v = target;
    int m = best_mode;

    while (v != -1) {
        const int u = dj.parent_v[v][m];
        if (u == -1) {
            break;
        }
        reverse_steps.push_back(Step{u, v, dj.parent_edge_mode[v][m]});
        const int next_mode = dj.parent_mode[v][m];
        v = u;
        if (next_mode == kNoMode || next_mode < 0) {
            break;
        }
        m = next_mode;
    }

    std::reverse(reverse_steps.begin(), reverse_steps.end());
    route.steps = std::move(reverse_steps);
    return route;
}

std::vector<Route> solve_request(
    const Graph& g,
    const ModelParams& model,
    const Request& rq
) {
    const DijkstraStateResult dj = dijkstra_states(g, model, rq.start);

    std::vector<Route> routes;
    routes.reserve(rq.targets.size());
    for (int target : rq.targets) {
        routes.push_back(build_route_to_target(dj, model, rq.start, target, rq.k));
    }

    if (!routes.empty()) {
        quicksort_routes(routes, 0, static_cast<int>(routes.size()) - 1);
    }
    return routes;
}

void quicksort_routes(std::vector<Route>& a, int l, int r) {
    int i = l;
    int j = r;
    const Route pivot = a[l + (r - l) / 2];

    while (i <= j) {
        while (route_less(a[i], pivot)) {
            ++i;
        }
        while (route_less(pivot, a[j])) {
            --j;
        }
        if (i <= j) {
            std::swap(a[i], a[j]);
            ++i;
            --j;
        }
    }

    if (l < j) {
        quicksort_routes(a, l, j);
    }
    if (i < r) {
        quicksort_routes(a, i, r);
    }
}
