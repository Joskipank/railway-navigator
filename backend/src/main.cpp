#include "algorithms.hpp"
#include "parser.hpp"
#include "validator.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

const char* mode_label(int mode) {
    switch (mode) {
        case MODE_METRO:
            return "metro";
        case MODE_BUS:
            return "bus";
        case MODE_RAIL:
            return "rail";
        default:
            return "unknown";
    }
}

std::string format_path(const Route& route, int start) {
    if (!route.reachable) {
        return "unreachable";
    }

    if (route.steps.empty()) {
        return std::to_string(start);
    }

    std::ostringstream out;
    for (std::size_t i = 0; i < route.steps.size(); ++i) {
        const Step& step = route.steps[i];
        out << step.from << "-[" << mode_label(step.mode) << "]->" << step.to;
        if (i + 1 < route.steps.size()) {
            out << ' ';
        }
    }
    return out.str();
}

void print_route_formatted(const Route& route, int start) {
    std::cout << "Destination: " << route.target << " | ";

    if (!route.reachable) {
        std::cout << "Time: INF | Transfers: INF | Metric: INF | Path: unreachable\n";
        return;
    }

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Time: " << route.time
              << " | Transfers: " << route.transfers
              << " | Metric: " << route.metric
              << " | Path: " << format_path(route, start) << '\n';
}

std::vector<std::vector<int>> get_isolated_components(const Graph& g, TransportType type) {
    std::vector<std::vector<int>> components = get_connected_components(g, type);
    if (!components.empty()) {
        components.erase(components.begin());
    }
    return components;
}

void print_isolated_zones(const Graph& g, TransportType type, const std::string& label) {
    std::cout << "ISOLATED ZONES (" << label << ")\n";
    const std::vector<std::vector<int>> zones = get_isolated_components(g, type);
    if (zones.empty()) {
        std::cout << "None\n";
        return;
    }
    for (std::size_t i = 0; i < zones.size(); ++i) {
        const auto& zone = zones[i];
        std::cout << (i + 1) << ". " << zone.size() << " stations: ";
        for (std::size_t j = 0; j < zone.size(); ++j) {
            std::cout << zone[j];
            if (j + 1 < zone.size()) {
                std::cout << ' ';
            }
        }
        std::cout << '\n';
    }
}

} // namespace

int main() {
    InputData data;
    std::string error;

    if (!parse_all(std::cin, data, error)) {
        std::cerr << error << "\n";
        return 1;
    }

    if (!validate_all(data, error)) {
        std::cerr << error << "\n";
        return 1;
    }

    print_isolated_zones(data.g, TransportType::Metro, "metro");
    std::cout << '\n';
    print_isolated_zones(data.g, TransportType::Bus, "bus");
    std::cout << '\n';
    print_isolated_zones(data.g, TransportType::Rail, "rail");
    std::cout << '\n';
    print_isolated_zones(data.g, TransportType::All, "all");
    std::cout << '\n';

    for (std::size_t i = 0; i < data.requests.size(); ++i) {
        const Request& rq = data.requests[i];
        const std::vector<Route> routes = solve_request(data.g, data.model, rq);

        std::cout << "REQUEST " << (i + 1) << " (start " << rq.start << ", k " << rq.k << ")\n";
        if (routes.empty()) {
            std::cout << "No targets\n";
        } else {
            for (const Route& route : routes) {
                print_route_formatted(route, rq.start);
            }
        }

        if (i + 1 < data.requests.size()) {
            std::cout << '\n';
        }
    }

    return 0;
}
