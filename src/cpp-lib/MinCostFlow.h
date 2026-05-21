#pragma once

#include "Graph.h"
#include "Suurballe.h"
#include "Types.h"

struct MinCostFlow : Solver {
    int N;
    bool split_on_articulations;
    std::vector<std::vector<NetworkEdge>> network;
    MinCostFlow(const Graph &graph, std::string source, std::string target,
                Alpha alpha, bool split_on_articulations);

    virtual TwoPathsResult solve() override;

    TwoPathsResult solve_by_splitting();
};
