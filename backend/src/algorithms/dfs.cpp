#include "algorithms.hpp"

#include <algorithm>
#include <stack>

namespace {

bool is_indexed_type(TransportType type) {
    const int value = static_cast<int>(type);
    return value >= 0 && value <= 2;
}

std::size_t type_index(TransportType type) {
    return static_cast<std::size_t>(static_cast<int>(type));
}

} // namespace

void Graph::dfsIterative(
    int start,
    TransportType type,
    std::vector<bool>& visited,
    std::vector<int>& component
) const {
    if (start < 1 || start > n) {
        return;
    }
    if (visited[start]) {
        return;
    }

    std::stack<int> stack;
    stack.push(start);
    visited[start] = true;

    while (!stack.empty()) {
        const int v = stack.top();
        stack.pop();

        component.push_back(v);

        if (v < 1 || v > n) {
            continue;
        }

        if (is_indexed_type(type)) {
            const std::size_t idx = type_index(type);
            for (int to : adjacency[idx][v]) {
                if (to < 1 || to > n) {
                    continue;
                }
                if (!visited[to]) {
                    visited[to] = true;
                    stack.push(to);
                }
            }
        } else {
            for (const Edge& e : adj[v]) {
                const int to = e.to;
                if (to < 1 || to > n) {
                    continue;
                }
                if (!visited[to]) {
                    visited[to] = true;
                    stack.push(to);
                }
            }
        }
    }
}

std::vector<std::vector<int>> Graph::getConnectedComponents(TransportType type) const {
    std::vector<bool> visited(static_cast<std::size_t>(n) + 1, false);
    std::vector<std::vector<int>> components;
    components.reserve(static_cast<std::size_t>(n));

    for (int v = 1; v <= n; ++v) {
        if (visited[v]) {
            continue;
        }
        std::vector<int> component;
        dfsIterative(v, type, visited, component);
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

    for (const auto& component : components) {
        if (component.size() == 1) {
            isolated.push_back(component.front());
        }
    }

    std::sort(isolated.begin(), isolated.end());
    return isolated;
}

std::vector<std::vector<int>> get_connected_components(const Graph& g, TransportType type) {
    return g.getConnectedComponents(type);
}

std::vector<int> get_isolated_zones(const Graph& g, TransportType type) {
    return g.getIsolatedZones(type);
}
