#pragma once

#include "Types.h"
#include <iosfwd>
#include <string>
#include <unordered_map>
#include <vector>

struct Graph {
    std::vector<std::vector<int>> edges;
    std::vector<int> scores;
    std::vector<std::string> names, sequences;
    std::unordered_map<std::string, int> id_of;

    int add_vertex(const std::string &name, int default_score,
                   const std::string &sequence);

    int get_id(const std::string &name) const;

    void add_edge(int u, int v);

    Graph get_induced_subgraph(std::vector<int> const &vertices) const;

    std::pair<std::vector<std::string>, std::vector<Graph>>
    split_by_articulations(std::string const &s, std::string const &t) const;
};

struct NetworkEdge {
    int target;
    int64_t weight;
    bool free;
    bool back;
    int64_t reweighted_weight = -1;
    int capacity = 1;
};

std::vector<int> get_reachable(int start,
                               const std::vector<std::vector<int>> &edges);

std::vector<std::vector<NetworkEdge>> create_network(const Graph &graph,
                                                     Alpha alpha);
