#include "algorithms.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

namespace {

ModelParams make_model(int n) {
    ModelParams model{};
    model.station_transfer.assign(static_cast<std::size_t>(n) + 1, 0.0);
    return model;
}

void expect_step(const Step& step, int from, int to, int mode) {
    assert(step.from == from);
    assert(step.to == to);
    assert(step.mode == mode);
}

} // namespace

int main() {
    std::cout << "start\n";

    {
        Graph g;
        graph_init(g, 1);
        const ModelParams model = make_model(1);

        const DijkstraStateResult dj = dijkstra_states(g, model, 1);
        const Route route = build_route_to_target(dj, model, 1, 1, 2.0);

        assert(route.reachable);
        assert(route.time == 0.0);
        assert(route.transfers == 0);
        assert(route.metric == 0.0);
        assert(route.steps.empty());
    }

    {
        Graph g;
        graph_init(g, 3);
        graph_add_undirected(g, 1, 2, MODE_METRO, 5.0, 0.0);
        graph_add_undirected(g, 2, 3, MODE_BUS, 5.0, 0.0);
        graph_add_undirected(g, 1, 3, MODE_RAIL, 10.0, 0.0);

        const ModelParams model = make_model(3);
        const DijkstraStateResult dj = dijkstra_states(g, model, 1);
        const Route route = build_route_to_target(dj, model, 1, 3, 2.0);

        assert(route.reachable);
        assert(route.time == 10.0);
        assert(route.transfers == 0);
        assert(route.metric == 10.0);
        assert(route.steps.size() == 1);
        expect_step(route.steps[0], 1, 3, MODE_RAIL);
    }

    {
        Graph g;
        graph_init(g, 3);
        graph_add_undirected(g, 1, 2, MODE_METRO, 2.0, 0.0);
        graph_add_undirected(g, 2, 3, MODE_BUS, 2.0, 0.0);
        graph_add_undirected(g, 1, 3, MODE_BUS, 10.0, 0.0);

        ModelParams model = make_model(3);
        model.trans[MODE_METRO][MODE_BUS] = 3.0;
        model.station_transfer[2] = 1.0;

        const DijkstraStateResult dj = dijkstra_states(g, model, 1);
        const Route route = build_route_to_target(dj, model, 1, 3, 5.0);

        assert(route.reachable);
        assert(route.time == 8.0);
        assert(route.transfers == 1);
        assert(route.metric == 13.0);
        assert(route.steps.size() == 2);
        expect_step(route.steps[0], 1, 2, MODE_METRO);
        expect_step(route.steps[1], 2, 3, MODE_BUS);
    }

    {
        Graph g;
        graph_init(g, 3);
        graph_add_undirected(g, 1, 2, MODE_METRO, 1.0, 0.0);

        const ModelParams model = make_model(3);
        const DijkstraStateResult dj = dijkstra_states(g, model, 1);
        const Route route = build_route_to_target(dj, model, 1, 3, 1.0);

        assert(!route.reachable);
        assert(!std::isfinite(route.time));
        assert(route.steps.empty());
    }

    {
        Graph g;
        graph_init(g, 2);
        graph_add_undirected(g, 1, 2, MODE_METRO, 10.0, 1.0);

        ModelParams model = make_model(2);
        model.sensitivity[MODE_METRO] = 0.5;

        const DijkstraStateResult dj = dijkstra_states(g, model, 1);
        const Route route = build_route_to_target(dj, model, 1, 2, 0.0);

        assert(route.reachable);
        assert(route.time == 15.0);
        assert(route.transfers == 0);
        assert(route.metric == 15.0);
        assert(route.steps.size() == 1);
        expect_step(route.steps[0], 1, 2, MODE_METRO);
    }

    return 0;
}