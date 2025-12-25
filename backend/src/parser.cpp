#include "parser.hpp"

#include <sstream>
#include <limits>

// Маленькие “корменовские” процедуры чтения токенов
static bool read_int(std::istream& in, int& x) {
    return static_cast<bool>(in >> x);
}

static bool read_double(std::istream& in, double& x) {
    return static_cast<bool>(in >> x);
}

static std::string make_err(const std::string& msg) {
    return msg;
}

/*
ОЖИДАЕМЫЙ ФОРМАТ (можно поменять здесь, если у вас иначе):
N M
sens0 sens1 sens2
trans00 trans01 trans02
trans10 trans11 trans12
trans20 trans21 trans22
station_transfer[1..N]
M lines:
  u v mode base_time load
Q
Q queries:
  start T k  (then T targets)
*/
bool parse_all(std::istream& in, InputData& data, std::string& error) {
    error.clear();

    int N = 0, M = 0;
    if (!read_int(in, N) || !read_int(in, M)) {
        error = make_err("parse: cannot read N M");
        return false;
    }
    if (N <= 0 || M < 0) {
        error = make_err("parse: invalid N or M");
        return false;
    }

    // init graph
    graph_init(data.g, N);

    // sensitivity[3]
    for (int i = 0; i < 3; ++i) {
        double s = 0.0;
        if (!read_double(in, s)) {
            error = make_err("parse: cannot read sensitivity[3]");
            return false;
        }
        data.model.sensitivity[i] = s;
    }

    // trans[3][3]
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            double t = 0.0;
            if (!read_double(in, t)) {
                error = make_err("parse: cannot read transfer matrix 3x3");
                return false;
            }
            data.model.trans[i][j] = t;
        }
    }

    // station_transfer[1..N]
    data.model.station_transfer.assign(static_cast<std::size_t>(N) + 1, 0.0);
    for (int v = 1; v <= N; ++v) {
        double lt = 0.0;
        if (!read_double(in, lt)) {
            error = make_err("parse: cannot read station_transfer[1..N]");
            return false;
        }
        data.model.station_transfer[v] = lt;
    }

    // edges
    for (int i = 0; i < M; ++i) {
        int u = 0, v = 0, mode = 0;
        double base_time = 0.0, load = 0.0;

        if (!read_int(in, u) || !read_int(in, v) || !read_int(in, mode) ||
            !read_double(in, base_time) || !read_double(in, load)) {
            error = make_err("parse: cannot read an edge line: u v mode base_time load");
            return false;
        }

        // graph_add_undirected проверит диапазоны (вершины, mode, load)
        try {
            graph_add_undirected(data.g, u, v, mode, base_time, load);
        } catch (const std::exception& e) {
            error = std::string("parse: invalid edge: ") + e.what();
            return false;
        }
    }

    // queries
    int Q = 0;
    if (!read_int(in, Q)) {
        error = make_err("parse: cannot read Q");
        return false;
    }
    if (Q < 0) {
        error = make_err("parse: invalid Q");
        return false;
    }

    data.requests.clear();
    data.requests.reserve(static_cast<std::size_t>(Q));

    for (int qi = 0; qi < Q; ++qi) {
        Request rq;
        int T = 0;

        if (!read_int(in, rq.start) || !read_int(in, T) || !read_double(in, rq.k)) {
            error = make_err("parse: cannot read query header: start T k");
            return false;
        }
        if (rq.start < 1 || rq.start > N || T < 0) {
            error = make_err("parse: invalid query header values");
            return false;
        }

        rq.targets.clear();
        rq.targets.reserve(static_cast<std::size_t>(T));

        for (int j = 0; j < T; ++j) {
            int t = 0;
            if (!read_int(in, t)) {
                error = make_err("parse: cannot read target in query");
                return false;
            }
            rq.targets.push_back(t);
        }

        data.requests.push_back(rq);
    }

    return true;
}
