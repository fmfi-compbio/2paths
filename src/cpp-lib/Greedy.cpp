#include "Greedy.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr int kDefaultIterations = 100;

std::vector<std::vector<int>> build_predecessors(const Graph &g) {
    std::vector<std::vector<int>> predecessors(g.edges.size());
    for (int u = 0; u < (int)g.edges.size(); ++u) {
        for (int v : g.edges[u]) {
            if (v < 0 || v >= (int)g.edges.size()) {
                throw std::runtime_error("greedy: edge endpoint out of range");
            }
            predecessors[v].push_back(u);
        }
    }
    return predecessors;
}

std::vector<int>
topo_order_or_throw(const Graph &g, const std::vector<std::vector<int>> &pred) {
    std::vector<int> indeg(pred.size(), 0);
    for (int v = 0; v < (int)pred.size(); ++v) {
        indeg[v] = (int)pred[v].size();
    }

    std::vector<int> ready;
    ready.reserve(pred.size());
    for (int v = 0; v < (int)indeg.size(); ++v) {
        if (indeg[v] == 0) {
            ready.push_back(v);
        }
    }

    std::vector<int> topo;
    topo.reserve(g.edges.size());
    while (!ready.empty()) {
        const int u = ready.back();
        ready.pop_back();
        topo.push_back(u);
        for (int v : g.edges[u]) {
            --indeg[v];
            if (indeg[v] == 0) {
                ready.push_back(v);
            }
        }
    }

    if ((int)topo.size() != (int)g.edges.size()) {
        throw std::runtime_error("greedy: algorithm expects an acyclic graph");
    }
    return topo;
}

long double path_score(const std::vector<int> &path,
                       const std::vector<long double> &score_map) {
    long double score = 0.0L;
    for (int u : path) {
        score += score_map[u];
    }
    return score;
}

std::pair<std::vector<int>, long double> find_longest_path(
    const Graph &g, const std::vector<std::vector<int>> &predecessors,
    const std::vector<int> &topo_order,
    const std::vector<long double> &score_map, int source, int target) {
    const long double neg_inf = -std::numeric_limits<long double>::infinity();
    std::vector<long double> dp(g.edges.size(), neg_inf);
    std::vector<int> parent(g.edges.size(), -1);

    dp[source] = 0.0L;

    for (int v : topo_order) {
        if (v == source) {
            continue;
        }
        for (int u : predecessors[v]) {
            if (dp[u] == neg_inf) {
                continue;
            }
            const long double candidate = dp[u] + score_map[v];
            if (dp[v] < candidate) {
                dp[v] = candidate;
                parent[v] = u;
            }
        }
    }

    if (dp[target] == neg_inf) {
        throw std::runtime_error("greedy: no source-to-target path exists");
    }

    std::vector<int> path;
    for (int v = target; v != -1; v = parent[v]) {
        path.push_back(v);
        if (v == source) {
            break;
        }
    }
    if (path.empty() || path.back() != source) {
        throw std::runtime_error("greedy: failed to reconstruct path");
    }
    std::reverse(path.begin(), path.end());
    return {path, dp[target]};
}

void update_weights(std::vector<long double> &score_map, Alpha alpha,
                    const std::vector<int> &path) {
    const long double factor = static_cast<long double>(alpha.numerator) /
                               static_cast<long double>(alpha.denominator);
    for (int u : path) {
        if (score_map[u] < 0) {
            score_map[u] = (double)2 - factor;
        } else {
            score_map[u] *= factor;
        }
    }
}

std::vector<std::string> ids_to_names(const Graph &g,
                                      const std::vector<int> &path) {
    std::vector<std::string> names;
    names.reserve(path.size());
    for (int u : path) {
        if (u < 0 || u >= (int)g.names.size()) {
            throw std::runtime_error(
                "greedy: vertex id out of range during name conversion");
        }
        names.push_back(g.names[u]);
    }
    return names;
}

double exact_result_score(const Graph &g, const std::vector<int> &path1,
                          const std::vector<int> &path2, Alpha alpha) {
    std::vector<int> used(g.edges.size(), 0);
    int64_t scaled_sum = 0;

    for (int u : path1) {
        scaled_sum += (int64_t)g.scores[u] * alpha.denominator;
        used[u] = 1;
    }
    for (int u : path2) {
        if (used[u]) {
            if (g.scores[u] < 0) {
                scaled_sum += (int64_t)g.scores[u] *
                              (2 * alpha.denominator - alpha.numerator);
            } else {
                scaled_sum += (int64_t)g.scores[u] * alpha.numerator;
            }
        } else {
            scaled_sum += (int64_t)g.scores[u] * alpha.denominator;
        }
    }

    return static_cast<double>(scaled_sum) /
           static_cast<double>(alpha.denominator);
}

} // namespace

Greedy::Greedy(const Graph &graph, std::string source, std::string target,
               Alpha alpha)
    : Solver(graph, std::move(source), std::move(target), alpha) {
    N = (int)graph.edges.size();
}

TwoPathsResult Greedy::solve() {
    const int s = graph.get_id(source);
    const int t = graph.get_id(target);

    const auto predecessors = build_predecessors(graph);
    const auto topo = topo_order_or_throw(graph, predecessors);

    std::vector<long double> score_map(graph.scores.begin(),
                                       graph.scores.end());

    auto [path0, score0] =
        find_longest_path(graph, predecessors, topo, score_map, s, t);

    std::vector<std::vector<int>> paths = {path0};
    std::vector<long double> scores = {score0};

    long double best_score = -std::numeric_limits<long double>::infinity();
    std::vector<int> best_path1 = path0;
    std::vector<int> best_path2 = path0;

    for (int i = 0; i < kDefaultIterations; i++) {
        update_weights(score_map, alpha, paths.back());

        auto [next_path, next_score] =
            find_longest_path(graph, predecessors, topo, score_map, s, t);
        const long double candidate_sum = next_score + scores.back();

        if (candidate_sum > best_score) {
            best_score = candidate_sum;
            best_path1 = paths.back();
            best_path2 = next_path;
        } else {
            break;
        }

        paths.push_back(next_path);
        scores.push_back(path_score(next_path, score_map));
    }

    return {exact_result_score(graph, best_path1, best_path2, alpha),
            ids_to_names(graph, best_path1), ids_to_names(graph, best_path2)};
}
