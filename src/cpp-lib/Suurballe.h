#pragma once

#include "Graph.h"
#include "Solver.h"
#include "Types.h"

#include <map>
#include <vector>

struct ShortestPathSuff {
    std::vector<int64_t> distances;
    std::map<int, int> sp_parents;
};

struct InternalResult {
    int64_t cost = 0;
    std::vector<int> path1, path2;
};

struct Suurballe : Solver {
    int N;
    Alpha alpha;
    std::vector<std::vector<NetworkEdge>> network;
    Suurballe(const Graph &graph, std::string source, std::string target,
              Alpha alpha);

    static std::vector<std::string>
    getOriginalPath(const std::vector<int> &path,
                    std::vector<std::string> names);

    virtual TwoPathsResult solve() override;

    ShortestPathSuff getDistsAcyclic(int start) const;

    ShortestPathSuff getDistsPositiveWeights(int source) const;

    void makeResidual(const std::vector<int> &path);

    void reweight(const std::vector<int64_t> &dists);

    InternalResult computeResult(int source, int sink);
};
