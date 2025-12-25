#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <array>
#include <string>
#include <istream>

#include "graph.hpp"

struct ModelParams {
    std::array<double, 3> sensitivity;          // чувствительность по видам
    std::array<std::array<double, 3>, 3> trans; // матрица межвидовых штрафов 3x3
    std::vector<double> station_transfer;       // локальная пересадка на станции [1..N]
};

struct Request {
    int start = 0;
    std::vector<int> targets;
    double k = 0.0; // "цена" пересадки для метрики удобства
};

struct InputData {
    Graph g;
    ModelParams model;
    std::vector<Request> requests;
};

// PARSE-ALL(in, data)
// Возвращает true при успехе; false если не удалось прочитать (или формат не совпал).
bool parse_all(std::istream& in, InputData& data, std::string& error);

#endif // PARSER_HPP
