#include "MinCostFlow.h"

#include <cassert>
#include <cstdlib>
#include <ranges>
#include <stdexcept>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Types.h"
#include "ortools/graph/min_cost_flow.h"
using namespace std;

using operations_research::SimpleMinCostFlow;

std::vector<int>
extract_path(std::vector<std::vector<pair<int, int>>> &outgoing, int start,
             const std::unordered_set<int> &ends, int original_vertex_count) {
    if (start < 0 || start >= (int)outgoing.size()) {
        throw std::runtime_error(
            "mincostflow: path extraction start/end out of range");
    }

    std::vector<int> path;
    path.reserve(64);
    path.push_back(start / 4);

    int u = start;
    while (ends.find(u) == ends.end()) {
        int next = -1;
        for (int i = 0; i < outgoing[u].size(); i++) {
            auto [v, cnt] = outgoing[u][i];
            if (cnt != 0) {
                outgoing[u][i].second--;
                next = v;
                break;
            }
        }
        if (next == -1) {
            throw std::runtime_error(
                "mincostflow: failed to extract path from flow (dead end)");
        }
        u = next;

        const int id = u / 4;
        if (id < 0 || id >= original_vertex_count) {
            throw std::runtime_error(
                "mincostflow: extracted node maps out of original range");
        }
        if (path.back() != id) {
            path.push_back(id);
        }
    }
    return path;
}

MinCostFlow::MinCostFlow(const Graph &graph, std::string source,
                         std::string target, Alpha alpha,
                         bool split_on_articulations)
    : Solver(graph, std::move(source), std::move(target), alpha) {
    network = create_network(graph, alpha);
    N = (int)network.size();
    this->split_on_articulations = split_on_articulations;
}

static std::vector<std::string> ids_to_names(const Graph &g,
                                             const std::vector<int> &path) {
    std::vector<std::string> names;
    names.reserve(path.size());
    for (int u : path) {
        if (u < 0 || u >= (int)g.names.size()) {
            throw std::runtime_error(
                "mincostflow: vertex id out of range during name conversion");
        }
        names.push_back(g.names[u]);
    }
    return names;
}

TwoPathsResult MinCostFlow::solve_by_splitting() {
    auto [artics, super_path] = graph.split_by_articulations(source, target);
    assert(artics.size() == super_path.size() - 1);

    TwoPathsResult result;
    if (super_path.size() == 1) {
        split_on_articulations = false;
        result = solve();
    } else {
        result =
            MinCostFlow(super_path[0], source, artics[0], alpha, false).solve();
        for (int i = 0; i < artics.size(); i++) {
            string t = (i + 1 == artics.size()) ? target : artics[i + 1];
            TwoPathsResult cur_res =
                MinCostFlow(super_path[i + 1], artics[i], t, alpha, false)
                    .solve();
            assert(cur_res.path1.front() == artics[i]);
            assert(cur_res.path2.front() == artics[i]);
            cur_res.path1.erase(cur_res.path1.begin());
            cur_res.path2.erase(cur_res.path2.begin());
            int score = graph.scores[graph.get_id(artics[i])];
            cur_res.score -= alpha.get_both_passes_score_numerator(
                                 graph.scores[graph.get_id(artics[i])]) /
                             (double)alpha.denominator;
            result.path1.insert(result.path1.end(), cur_res.path1.begin(),
                                cur_res.path1.end());
            result.path2.insert(result.path2.end(), cur_res.path2.begin(),
                                cur_res.path2.end());
            result.score += cur_res.score;
        }
    }
    return result;
}

TwoPathsResult MinCostFlow::solve() {
    if (split_on_articulations) {
        return solve_by_splitting();
    }
    SimpleMinCostFlow mcf;
    vector<int> from, to, costs;
    for (int u = 0; u < N; u++) {
        for (NetworkEdge e : network[u]) {
            if (!e.back) {
                mcf.AddArcWithCapacityAndUnitCost(u, e.target, e.capacity,
                                                  e.weight);
                from.push_back(u);
                to.push_back(e.target);
                costs.push_back(e.weight);
            }
        }
    }
    mcf.SetPriceScaling(false);
    for (int u = 0; u < N; u++) {
        mcf.SetNodeSupply(u, 0);
    }
    int s1 = 4 * graph.get_id(source);
    int s2 = 4 * graph.get_id(source) + 1;
    int t1 = 4 * graph.get_id(target) + 2;
    int t2 = 4 * graph.get_id(target) + 3;
    mcf.SetNodeSupply(s1, 1);
    mcf.SetNodeSupply(s2, 1);
    mcf.SetNodeSupply(t1, -1);
    mcf.SetNodeSupply(t2, -1);


    const int status = mcf.Solve();

    if (status != SimpleMinCostFlow::OPTIMAL) {
        throw std::runtime_error(
            "mincostflow: Solve() did not find an optimal solution (status=" +
            std::to_string(status) + ")");
    }
    std::vector<std::vector<pair<int, int>>> outgoing(N);
    for (int a = 0; a < mcf.NumArcs(); ++a) {
        if (mcf.Flow(a) <= 0)
            continue;
        const int tail = (int)mcf.Tail(a);
        const int head = (int)mcf.Head(a);
        if (tail < 0 || tail >= N || head < 0 || head >= N) {
            throw std::runtime_error(
                "mincostflow: internal error (arc endpoint out of range)");
        }
        outgoing[tail].push_back({head, (int)mcf.Flow(a)});
    }

    std::unordered_set<int> end_nodes = {t1, t2};
    auto path1 = extract_path(outgoing, s1, end_nodes, N);
    end_nodes.erase(path1.back());
    auto path2 = extract_path(outgoing, s2, end_nodes, N);

    double opt_score = (double)(-mcf.OptimalCost()) / (double)alpha.denominator;
    TwoPathsResult result{opt_score, ids_to_names(graph, path1),
                          ids_to_names(graph, path2)};
    if (!resultValid(result)) {
        throw std::runtime_error(
            "mincostflow: extracted paths do not match optimal flow");
    }
    return result;
}
