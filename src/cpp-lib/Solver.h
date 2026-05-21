#pragma once

#include "Graph.h"
#include "Types.h"
#include <vector>

struct Solver {
    const Graph &graph;
    std::string source, target;
    Alpha alpha;
    Solver(const Graph &graph, std::string source, std::string target,
           Alpha alpha);
    virtual ~Solver() = default;
    virtual TwoPathsResult solve();

    bool pathIsValid(const std::vector<std::string> &path) const;
    std::string getCorrespondingSequence(const std::vector<std::string> &path) const;
    bool resultValid(TwoPathsResult result) const;
};

std::vector<int> getFreqTable(const Graph &graph,
                              std::vector<std::string> &path1,
                              std::vector<std::string> &path2);
