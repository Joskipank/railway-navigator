#include <vector>
#include "graph.hpp"

using namespace std;

static const int MAXN = 100000;

bool used[MAXN];

// DFS для метро
void dfs_metro(const Graph& g, int v, vector<int>& component)
{
    used[v] = true;
    component.push_back(v);

    for (const Edge& e : g.adj[v])
    {
        if (e.mode != MODE_METRO)
            continue;

        int to = e.to;
        if (!used[to])
            dfs_metro(g, to, component);
    }
}

// поиск компонент метро
vector<vector<int>> find_metro_components(const Graph& g)
{
    int n = g.n;

    for (int i = 0; i < n; ++i)
        used[i] = false;

    vector<vector<int>> components;

    for (int v = 0; v < n; ++v)
    {
        if (used[v])
            continue;

        vector<int> component;
        dfs_metro(g, v, component);

        if (!component.empty())
            components.push_back(component);
    }

    return components;
}

// Поиск изолированных зон метро
vector<int> find_isolated_metro_zones(const Graph& g)
{
    int n = g.n;

    for (int i = 0; i < n; ++i)
        used[i] = false;

    vector<int> isolated;

    for (int v = 0; v < n; ++v)
    {
        if (used[v])
            continue;

        vector<int> component;
        dfs_metro(g, v, component);

        bool hasMetroEdge = false;

        for (int u : component)
        {
            for (const Edge& e : g.adj[u])
            {
                if (e.mode == MODE_METRO)
                {
                    hasMetroEdge = true;
                    break;
                }
            }
            if (hasMetroEdge)
                break;
        }

        if (!hasMetroEdge)
        {
            for (int u : component)
                isolated.push_back(u);
        }
    }

    return isolated;
}
