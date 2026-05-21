#pragma once

#include "Graph.h"
#include "Suurballe.h"
#include "Types.h"

struct Greedy : Solver {
    int N;
    Greedy(const Graph &graph, std::string source, std::string target,
           Alpha alpha);

    virtual TwoPathsResult solve() override;
};
