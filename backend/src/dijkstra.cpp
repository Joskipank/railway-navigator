#include "graph.hpp"
#include <vector>
#include <queue>
#include <array>
#include <limits>
#include <algorithm>

using namespace std;

const double INF = 1e18;

//статутс для дейкстеры
struct State {
    int v;        // станция
    int mode;     // последний вид транспорта
    double dist;  // текущее минимальное время
};

struct Cmp { // компаратор для min-heap
    bool operator()(const State& a, const State& b) const {
        return a.dist > b.dist;
    }
};


//дейкстера

void dijkstra(
    const Graph& g,
    int start,
    const array<double,3>& sensitivity,
    const array<array<double,3>,3>& transfer_penalty,
    const vector<double>& station_penalty,
    vector<vector<double>>& dist,
    vector<vector<pair<int,int>>>& parent
){
    dist.assign(g.n + 1, vector<double>(4, INF));
    parent.assign(g.n + 1, vector<pair<int,int>>(4, {-1, -1}));

    priority_queue<State, vector<State>, Cmp> pq;

    dist[start][3] = 0.0;// старт без транспорта
    pq.push({start, 3, 0.0});

    while (!pq.empty()) {
        State cur = pq.top();
        pq.pop();

        if (cur.dist > dist[cur.v][cur.mode]) continue;

        for (const Edge& e : g.adj[cur.v]) {
            int next = e.to;
            int new_mode = e.mode;

            double w = edge_time(e, sensitivity);// фактическое время ребра

            double penalty = 0.0;// штраф за пересадку
            if (cur.mode != 3 && cur.mode != new_mode) {
                penalty = transfer_penalty[cur.mode][new_mode]
                        + station_penalty[cur.v];
            }

            double nd = cur.dist + w + penalty;

            if (nd < dist[next][new_mode]) {
                dist[next][new_mode] = nd;
                parent[next][new_mode] = {cur.v, cur.mode};
                pq.push({next, new_mode, nd});
            }
        }
    }
}

// востановление пути
vector<int> restore_path(
    int target,
    int best_mode,
    const vector<vector<pair<int,int>>>& parent
){
    vector<int> path;
    int v = target;
    int m = best_mode;

    while (v != -1) {
        path.push_back(v);
        auto p = parent[v][m];
        v = p.first;
        m = p.second;
    }

    std::reverse(path.begin(), path.end());
    return path;
}
