#include <vector>
#include "graph.hpp"

using namespace std;

void dfs_bus(const Graph& g, int v, vector<int>& used, vector<int>& component) {
    used[v] = true;
    component.push_back(v);

    for (const Edge& e : g.adj[v]) {
        // Пропускаем все типы транспорта, кроме автобуса
        if (e.mode != MODE_BUS) {
            continue;
        }

        int to = e.to;
        if (!used[to]) {
            dfs_bus(g, to, used, component);
        }
    }
}

/**
 * Определение компонент связности автобусной сети.
 * Возвращает вектор векторов, где каждый внутренний вектор — это отдельная компонента.
 */
vector<vector<int>> find_bus_components(const Graph& g) {
    vector<int> used(g.n + 1, false);
    vector<vector<int>> components;

    for (int v = 1; v <= g.n; ++v) {
        // Если вершина еще не посещена, значит найдена новая компонента
        if (!used[v]) {
            vector<int> current_component;
            dfs_bus(g, v, used, current_component);
            components.push_back(current_component);
        }
    }

    return components;
}

/**
 * Подсчет количества изолированных автобусных зон (компонент связности).
 */
int count_bus_components(const Graph& g) {
    vector<int> used(g.n + 1, false);
    int count = 0;

    for (int v = 1; v <= g.n; ++v) {
        if (!used[v]) {
            count++;
            vector<int> dummy; // Нам не важен состав, только факт наличия
            dfs_bus(g, v, used, dummy);
        }
    }

    return count;
}

