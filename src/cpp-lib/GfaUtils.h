#include "Graph.h"

#include <charconv>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

struct GfaReadOptions {
    std::unordered_set<std::string> expected_walk_sample_ids;
    bool match_all_walks = false;
    int default_score = 0;
    bool require_positive_orientation = true;
    bool parse_expected_paths_from_W = true;
};

struct GfaReadResult {
    Graph graph;
    std::vector<std::string> path1, path2;
    std::vector<std::vector<std::string>> paths;
};

GfaReadResult read_gfa(std::istream &in, const GfaReadOptions &opt,
                       bool more_than_2_walks);

void output_new_gfa(std::istream &in, Graph &graph,
                    std::vector<std::string> &path1,
                    std::vector<std::string> &path2);
