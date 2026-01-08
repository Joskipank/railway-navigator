#include <vector>
#include <algorithm>
#include <cstring>

#include "graph.hpp"
#include "dfs_bus.cpp"
#include "dfs_metro.cpp"
#include "dfs_railway.cpp"

using namespace std;


// Объединённый DFS через существующие DFS по типам
void dfs_combined(const Graph& g, int v, vector<int>& used, vector<int>& component)
{
    if (used[v])
        return;

    used[v] = true;
    component.push_back(v);

    // DFS по автобусам
    dfs_bus(g, v, used);

    // DFS по метро
    dfs_metro(g, v, used);

    // DFS по железной дороге
    dfs_rail(g, v, used);

    for (int i = 0; i < g.n; ++i)
    {
        if (used[i] &&
            find(component.begin(), component.end(), i) == component.end())
        {
            dfs_combined(g, i, used, component);
        }
    }
}

// Поиск компонент связности всего графа
vector<vector<int>> find_components(const Graph& g)
{
    vector<int> used(g.n, false);
    vector<vector<int>> components;

    for (int v = 0; v < g.n; ++v)
    {
        if (!used[v])
        {
            vector<int> component;
            dfs_combined(g, v, used, component);
            components.push_back(component);
        }
    }

    return components;
}
