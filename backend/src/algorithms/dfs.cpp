#include "algorithms.hpp"

#include <algorithm>
#include <cstdint>

namespace {

enum class Color : std::uint8_t { White, Gray, Black };

// Цвета и времена открытия/закрытия соответствуют DFS в CLRS и задают дерево предков,
// а также позволяют классифицировать рёбра при необходимости.
struct DfsState {
    std::vector<Color> color;
    std::vector<int> parent;
    std::vector<int> discovery;
    std::vector<int> finish;
    int time = 0;
};

bool is_indexed_type(TransportType type) {
    const int value = static_cast<int>(type);
    return value >= 0 && value <= 2;
}

std::size_t type_index(TransportType type) {
    return static_cast<std::size_t>(static_cast<int>(type));
}

// DFS-VISIT(u): на входе u белая; на выходе u черная и все достижимые из u
// вершины в соответствующем подграфе также черные (CLRS 22.3).
void dfs_visit(
    const Graph& g,
    int u,
    TransportType type,
    DfsState& state,
    std::vector<int>& component
) {
    if (!valid_vertex(g, u)) {
        return;
    }

    state.time += 1;
    state.discovery[u] = state.time;
    state.color[u] = Color::Gray;
    component.push_back(u);

    if (is_indexed_type(type)) {
        const std::size_t idx = type_index(type);
        for (int v : g.adjacency[idx][u]) {
            if (!valid_vertex(g, v)) {
                continue;
            }
            if (state.color[v] == Color::White) {
                state.parent[v] = u;
                dfs_visit(g, v, type, state, component);
            }
        }
    } else {
        for (const Edge& e : g.adj[u]) {
            const int v = e.to;
            if (!valid_vertex(g, v)) {
                continue;
            }
            if (state.color[v] == Color::White) {
                state.parent[v] = u;
                dfs_visit(g, v, type, state, component);
            }
        }
    }

    state.color[u] = Color::Black;
    state.time += 1;
    state.finish[u] = state.time;
}

} // namespace

std::vector<std::vector<int>> Graph::getConnectedComponents(TransportType type) const {
    // DFS формирует лес предков; каждая его вершина соответствует компоненте связности.
    DfsState state;
    state.color.assign(static_cast<std::size_t>(n) + 1, Color::White);
    state.parent.assign(static_cast<std::size_t>(n) + 1, -1);
    state.discovery.assign(static_cast<std::size_t>(n) + 1, 0);
    state.finish.assign(static_cast<std::size_t>(n) + 1, 0);

    std::vector<std::vector<int>> components;
    components.reserve(static_cast<std::size_t>(n));

    for (int u = 1; u <= n; ++u) {
        if (state.color[u] != Color::White) {
            continue;
        }
        std::vector<int> component;
        dfs_visit(*this, u, type, state, component);
        if (!component.empty()) {
            std::sort(component.begin(), component.end());
            components.push_back(std::move(component));
        }
    }

    std::sort(
        components.begin(),
        components.end(),
        [](const std::vector<int>& a, const std::vector<int>& b) {
            if (a.size() != b.size()) {
                return a.size() > b.size();
            }
            return a < b;
        }
    );

    return components;
}

std::vector<int> Graph::getIsolatedZones(TransportType type) const {
    std::vector<int> isolated;
    const std::vector<std::vector<int>> components = getConnectedComponents(type);

    if (components.size() <= 1) {
        return isolated;
    }

    for (std::size_t i = 1; i < components.size(); ++i) {
        const auto& component = components[i];
        isolated.insert(isolated.end(), component.begin(), component.end());
    }

    return isolated;
}

std::vector<std::vector<int>> get_connected_components(const Graph& g, TransportType type) {
    return g.getConnectedComponents(type);
}

std::vector<int> get_isolated_zones(const Graph& g, TransportType type) {
    return g.getIsolatedZones(type);
}
