#ifndef VALIDATOR_HPP
#define VALIDATOR_HPP

#include <string>
#include "parser.hpp"   // InputData, ModelParams, Request
#include "graph.hpp"    // Graph, Edge

// VALIDATE-ALL(data)
// true если всё ок, иначе false и error заполнен.
bool validate_all(const InputData& data, std::string& error);

// Если хочешь — можно вызывать по частям:
bool validate_graph(const Graph& g, std::string& error);
bool validate_model(const Graph& g, const ModelParams& m, std::string& error);
bool validate_requests(const Graph& g, const std::vector<Request>& reqs, std::string& error);

#endif // VALIDATOR_HPP
