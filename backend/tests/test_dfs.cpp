#include "algorithms.hpp"

#include <cassert>
#include <vector>

int main() {
    Graph g;
    graph_init(g, 6);

    // Metro: 1-2-3 and 4-5 (components plus isolated 6).
    graph_add_undirected(g, 1, 2, MODE_METRO, 1.0, 0.0);
    graph_add_undirected(g, 2, 3, MODE_METRO, 1.0, 0.0);
    graph_add_undirected(g, 4, 5, MODE_METRO, 1.0, 0.0);

    // Rail: connect all stations into one component.
    graph_add_undirected(g, 1, 2, MODE_RAIL, 1.0, 0.0);
    graph_add_undirected(g, 2, 3, MODE_RAIL, 1.0, 0.0);
    graph_add_undirected(g, 3, 4, MODE_RAIL, 1.0, 0.0);
    graph_add_undirected(g, 4, 5, MODE_RAIL, 1.0, 0.0);
    graph_add_undirected(g, 5, 6, MODE_RAIL, 1.0, 0.0);

    // Bus: connect 1-5, leave 6 isolated.
    graph_add_undirected(g, 1, 2, MODE_BUS, 1.0, 0.0);
    graph_add_undirected(g, 2, 3, MODE_BUS, 1.0, 0.0);
    graph_add_undirected(g, 3, 4, MODE_BUS, 1.0, 0.0);
    graph_add_undirected(g, 4, 5, MODE_BUS, 1.0, 0.0);

    const auto metro = get_connected_components(g, TransportType::Metro);
    assert(metro.size() == 3);
    assert((metro[0] == std::vector<int>{1, 2, 3}));
    assert((metro[1] == std::vector<int>{4, 5}));
    assert((metro[2] == std::vector<int>{6}));

    const auto rail = get_connected_components(g, TransportType::Rail);
    assert(rail.size() == 1);
    assert(rail[0].size() == 6);
    assert(rail[0].front() == 1);
    assert(rail[0].back() == 6);

    const auto bus_isolated = get_isolated_zones(g, TransportType::Bus);
    assert((bus_isolated == std::vector<int>{6}));

    return 0;
}
