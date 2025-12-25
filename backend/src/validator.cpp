#include "validator.hpp"

#include <cmath>      // std::isfinite
#include <limits>

static bool is_finite(double x) {
    return std::isfinite(x);
}

bool validate_graph(const Graph& g, std::string& error) {
    error.clear();

    if (g.n <= 0) { error = "validate_graph: N must be > 0"; return false; }
    if ((int)g.adj.size() != g.n + 1) { error = "validate_graph: adj size must be N+1"; return false; }

    // Проверим каждое ребро в списках смежности
    for (int u = 1; u <= g.n; ++u) {
        for (const Edge& e : g.adj[u]) {
            if (e.to < 1 || e.to > g.n) {
                error = "validate_graph: edge has invalid 'to' vertex";
                return false;
            }
            if (e.mode < 0 || e.mode > 2) {
                error = "validate_graph: edge has invalid mode (must be 0..2)";
                return false;
            }
            if (!is_finite(e.base_time) || e.base_time < 0.0) {
                error = "validate_graph: edge base_time must be finite and >= 0";
                return false;
            }
            if (!is_finite(e.load) || e.load < 0.0 || e.load > 1.0) {
                error = "validate_graph: edge load must be finite and in [0,1]";
                return false;
            }
            // id может быть любым >=0, но на всякий случай:
            if (e.id < 0) {
                error = "validate_graph: edge id must be >= 0";
                return false;
            }
        }
    }

    // g.m — число неориентированных ребер; не обязаны строго сверять,
    // но базово оно должно быть >=0
    if (g.m < 0) { error = "validate_graph: M must be >= 0"; return false; }

    return true;
}

bool validate_model(const Graph& g, const ModelParams& m, std::string& error) {
    error.clear();

    // sensitivity[3]
    for (int i = 0; i < 3; ++i) {
        if (!is_finite(m.sensitivity[i]) || m.sensitivity[i] < 0.0) {
            error = "validate_model: sensitivity must be finite and >= 0";
            return false;
        }
    }

    // transfer matrix 3x3
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            const double x = m.trans[i][j];
            if (!is_finite(x) || x < 0.0) {
                error = "validate_model: transfer matrix entries must be finite and >= 0";
                return false;
            }
        }
    }

    // station_transfer size and values
    if ((int)m.station_transfer.size() != g.n + 1) {
        error = "validate_model: station_transfer must have size N+1";
        return false;
    }
    for (int v = 1; v <= g.n; ++v) {
        const double x = m.station_transfer[v];
        if (!is_finite(x) || x < 0.0) {
            error = "validate_model: station_transfer[v] must be finite and >= 0";
            return false;
        }
    }

    return true;
}

bool validate_requests(const Graph& g, const std::vector<Request>& reqs, std::string& error) {
    error.clear();

    for (std::size_t qi = 0; qi < reqs.size(); ++qi) {
        const Request& r = reqs[qi];

        if (r.start < 1 || r.start > g.n) {
            error = "validate_requests: query has invalid start station";
            return false;
        }
        if (!is_finite(r.k) || r.k < 0.0) {
            error = "validate_requests: query coefficient k must be finite and >= 0";
            return false;
        }
        for (int t : r.targets) {
            if (t < 1 || t > g.n) {
                error = "validate_requests: query has invalid target station";
                return false;
            }
        }
    }

    return true;
}

bool validate_all(const InputData& data, std::string& error) {
    if (!validate_graph(data.g, error)) return false;
    if (!validate_model(data.g, data.model, error)) return false;
    if (!validate_requests(data.g, data.requests, error)) return false;
    return true;
}
