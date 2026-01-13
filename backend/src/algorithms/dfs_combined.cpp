#include "models/graph.hpp"

#include <algorithm>
#include <cstdio>
#include <vector>

namespace {

bool by_size_desc(const std::vector<int>& a, const std::vector<int>& b) {
    if (a.size() != b.size()) {
        return a.size() > b.size();
    }
    return a < b;
}

void print_zone_summary(const std::vector<std::vector<int>>& comps, int total_vertices) {
    const int zones = static_cast<int>(comps.size());
    if (zones == 0) {
        std::printf("No isolated zones\n");
        return;
    }

    std::printf("\nISOLATED ZONES (sorted by size):\n");
    std::printf("Total zones: %d\n", zones);
    std::printf("-----------------------------\n");

    for (int i = 0; i < zones; ++i) {
        const auto& comp = comps[static_cast<std::size_t>(i)];
        std::printf("%d. %d stations: ", i + 1, static_cast<int>(comp.size()));

        for (int j = 0; j < static_cast<int>(comp.size()); ++j) {
            std::printf("%d", comp[static_cast<std::size_t>(j)]);
            if (j + 1 < static_cast<int>(comp.size())) {
                std::printf(", ");
            }
        }
        std::printf("\n");
    }

    int total = 0;
    for (const auto& comp : comps) {
        total += static_cast<int>(comp.size());
    }

    std::printf("\nStats:\n");
    std::printf("- Covered stations: %d of %d\n", total, total_vertices);
    std::printf("- Largest zone: %d stations\n", static_cast<int>(comps.front().size()));
    std::printf("- Smallest zone: %d stations\n", static_cast<int>(comps.back().size()));
}

void print_zone_list(const std::vector<std::vector<int>>& comps) {
    std::printf("\nZones (by size):\n");
    for (std::size_t i = 0; i < comps.size(); ++i) {
        const auto& comp = comps[i];
        std::printf("%zu. %d st.: [", i + 1, static_cast<int>(comp.size()));
        for (std::size_t j = 0; j < comp.size(); ++j) {
            std::printf("%d", comp[j]);
            if (j + 1 < comp.size()) {
                std::printf(" ");
            }
        }
        std::printf("]\n");
    }
}

} // namespace

std::vector<std::vector<int>> find_components(const Graph& g) {
    auto comps = g.getConnectedComponents(TransportType::All);
    std::sort(comps.begin(), comps.end(), by_size_desc);
    if (!comps.empty()) {
        comps.erase(comps.begin()); // исключаем крупнейшую компоненту
    }
    return comps;
}

void print_sorted_zones(const Graph& g) {
    auto comps = find_components(g);
    print_zone_summary(comps, g.n);
}

void show_zones(const Graph& g) {
    auto comps = find_components(g);
    print_zone_list(comps);
}
