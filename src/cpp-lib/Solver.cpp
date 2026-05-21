#include "Solver.h"
#include <iostream>
#include <stdexcept>
#include <unordered_set>

Solver::Solver(const Graph &graph, std::string source, std::string target,
               Alpha alpha)
    : graph(graph), source(source), target(target), alpha(alpha) {
    int s = graph.get_id(source), t = graph.get_id(target);
    if (s < 0 || t < 0 || s >= (int)graph.edges.size() ||
        t >= (int)graph.edges.size()) {
        throw std::runtime_error("source/target out of range");
    }
}

TwoPathsResult Solver::solve() {
    throw std::runtime_error("solve() is not implemented for base Solver");
}

bool Solver::resultValid(TwoPathsResult result) const {
    bool valid = true;
    valid &= pathIsValid(result.path1);
    valid &= pathIsValid(result.path2);

    std::unordered_set<std::string> p1_set(result.path1.begin(),
                                           result.path1.end());
    int64_t res = 0;
    for (std::string u : result.path1)
        res +=
            alpha.get_first_pass_score_numerator(graph.scores[graph.get_id(u)]);
    for (std::string u : result.path2) {
        if (p1_set.find(u) == p1_set.end()) {
            res += alpha.get_first_pass_score_numerator(
                graph.scores[graph.get_id(u)]);
        } else {
            res += alpha.get_second_pass_score_numerator(
                graph.scores[graph.get_id(u)]);
        }
    }
    double score = (double)res / (double)alpha.denominator;
    valid &= fabs(score - result.score) < 1e-6;
    return valid;
}

bool Solver::pathIsValid(const std::vector<std::string> &path) const {
    if (path.empty())
        return false;
    std::vector<int> p;
    for (std::string x : path)
        p.push_back(graph.get_id(x));

    for (int u : p) {
        if (u < 0 || u >= (int)graph.edges.size())
            return false;
    }
    for (int i = 0; i + 1 < (int)p.size(); i++) {
        int u = p[i];
        int v = p[i + 1];
        bool ok = false;
        for (int w : graph.edges[u]) {
            if (w == v) {
                ok = true;
                break;
            }
        }
        if (!ok) {
            std::cerr << "There is no edge between " << graph.names[u]
                      << " and " << graph.names[v] << ". Path Invalid."
                      << std::endl;
            return false;
        }
    }
    return true;
}

std::string
Solver::getCorrespondingSequence(const std::vector<std::string> &path) const {
    std::vector<std::string> sequences;
    for (std::string name : path) {
        int u = graph.get_id(name);
        sequences.push_back(graph.sequences[u]);
    }
    std::string joined;
    for (const auto &segment : sequences) {
        joined += segment;
    }
    return joined;
}

std::vector<int> getFreqTable(const Graph &graph,
                              std::vector<std::string> &path1,
                              std::vector<std::string> &path2) {
    std::vector<int> table(graph.names.size(), 0);
    for (std::string u : path1) {
        table[graph.get_id(u)]++;
    }
    for (std::string u : path2) {
        table[graph.get_id(u)]++;
    }
    return table;
}
