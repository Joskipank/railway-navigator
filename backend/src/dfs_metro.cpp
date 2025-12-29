#include <vector>
#include "graph.hpp"

using namespace std;

// DFS для метро
void dfs_metro(const Graph& g, int v, vector<bool>& used)
{
    used[v] = true; // пометка остановок

    for (const Edge& e : g.adj[v])
    {
        if (e.mode != MODE_METRO)
            continue;

        int to = e.to;

        if (!used[to])
            dfs_metro(g, to, used);
    }
}

// поиск компонент метро
int count_metro_components(const Graph& g)
{
    vector<bool> used(g.n + 1, false);
    int components = 0;

    for (int v = 1; v <= g.n; ++v)
    {
        if (!used[v])
        {
            components++;
            dfs_metro(g, v, used);
        }
    }

    return components;
}

// Поиск изолированных зон метро
vector<int> find_isolated_metro_zones(const Graph& g)
{
    vector<bool> used(g.n + 1, false);

    // отмечаем все вершины, достижимые по метро
    for (int v = 1; v <= g.n; ++v)
    {
        if (!used[v])
            dfs_metro(g, v, used);
    }

    // все изолированные вершины
    vector<int> isolated;
    for (int v = 1; v <= g.n; ++v)
    {
        if (!used[v])
            isolated.push_back(v);
    }

    return isolated;
}
//на созвоне лучше сказать что не так