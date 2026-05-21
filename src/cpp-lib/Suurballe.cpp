#include "Suurballe.h"
#include "Types.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <map>
#include <queue>
#include <stdexcept>
#include <utility>
#include <vector>

using std::map;
using std::pair;
using std::priority_queue;
using std::queue;
using std::vector;

vector<int> getPath(const map<int, int> &sp_parents, int s, int t) {
    vector<int> path = {t};
    int u = t;
    while (u != s) {
        auto it = sp_parents.find(u);
        if (it == sp_parents.end()) {
            throw std::runtime_error("no path in shortest path parents");
        }
        int v = it->second;
        path.push_back(v);
        u = v;
    }
    std::reverse(path.begin(), path.end());
    return path;
}

Suurballe::Suurballe(const Graph &graph, std::string source, std::string target,
                     Alpha alpha)
    : Solver(graph, std::move(source), std::move(target), alpha), alpha(alpha) {
    network = create_network(graph, alpha);
    N = (int)network.size();
}

vector<std::string> Suurballe::getOriginalPath(const vector<int> &path,
                                               vector<std::string> names) {
    vector<int> res;
    res.reserve(path.size());
    res.push_back(path[0] / 4);
    for (int u : path) {
        int v = u / 4;
        if (res.back() != v)
            res.push_back(v);
    }
    vector<std::string> names_res;
    for (int x : res)
        names_res.push_back(names[x]);
    return names_res;
}

TwoPathsResult Suurballe::solve() {
    int s = 4 * graph.get_id(source) + 3;
    int t = 4 * graph.get_id(target);

    auto sps = getDistsAcyclic(s);
    auto path = getPath(sps.sp_parents, s, t);
    reweight(sps.distances);
    makeResidual(path);
    auto sps2 = getDistsPositiveWeights(s);
    auto path2 = getPath(sps2.sp_parents, s, t);
    makeResidual(path2);
    InternalResult internal = computeResult(s, t);

    auto getOriginalVertexScore = [&](int u) {
        u = 4 * (u / 4);
        int64_t score = 0;
        for (const NetworkEdge &e : this->network[u]) {
            if (!e.back) {
                score = e.weight;
            }
        }
        return score;
    };

    int64_t source_score = getOriginalVertexScore(s);
    internal.cost +=
        source_score + alpha.numerator * (source_score / alpha.denominator);
    int64_t target_score = getOriginalVertexScore(t);
    internal.cost +=
        target_score + alpha.numerator * (target_score / alpha.denominator);

    double opt_score = (double)(-internal.cost) / (double)alpha.denominator;
    return {opt_score, getOriginalPath(internal.path1, graph.names),
            getOriginalPath(internal.path2, graph.names)};
}

ShortestPathSuff Suurballe::getDistsAcyclic(int start) const {
    vector<bool> visited(N, false);
    queue<int> q;
    q.push(start);
    visited[start] = true;
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (const NetworkEdge &e : network[u]) {
            if (!visited[e.target] && e.free) {
                visited[e.target] = true;
                q.push(e.target);
            }
        }
    }

    vector<int> indegs(N, 0);
    for (int u = 0; u < N; u++) {
        for (const NetworkEdge &e : network[u]) {
            if (visited[e.target] && e.back)
                indegs[u]++;
        }
    }

    map<int, int> sp_parents;
    vector<int64_t> dists(N, 1);
    dists[start] = 0;
    vector<int> ready = {start};
    while (!ready.empty()) {
        int u = ready.back();
        ready.pop_back();
        if (!visited[u])
            continue;
        for (NetworkEdge const &e : network[u]) {
            if (!e.free)
                continue;
            int v = e.target;
            indegs[v]--;
            if (dists[v] == 1 || dists[u] + e.weight < dists[v]) {
                dists[v] = dists[u] + e.weight;
                sp_parents[v] = u;
            }
            if (indegs[v] == 0)
                ready.push_back(v);
        }
    }
    return {dists, sp_parents};
}

ShortestPathSuff Suurballe::getDistsPositiveWeights(int source) const {
    vector<int64_t> dists(N, -1);
    using P = pair<int64_t, int>;
    priority_queue<P, vector<P>, std::greater<P>> pq;
    dists[source] = 0;
    pq.push({0, source});
    map<int, int> sp_parents;

    while (!pq.empty()) {
        auto [cur_dist, u] = pq.top();
        pq.pop();
        if (dists[u] != -1 && cur_dist > dists[u])
            continue;
        for (const NetworkEdge &e : network[u]) {
            if (!e.free)
                continue;
            int v = e.target;
            int64_t w = e.reweighted_weight;
            assert(w >= 0);
            if (dists[v] == -1 || dists[u] + w < dists[v]) {
                dists[v] = dists[u] + w;
                sp_parents[v] = u;
                pq.push({dists[v], v});
            }
        }
    }
    return {dists, sp_parents};
}

void Suurballe::makeResidual(const vector<int> &path) {
    for (int i = 0; i + 1 < (int)path.size(); i++) {
        int u = path[i];
        int v = path[i + 1];
        for (NetworkEdge &e : network[u]) {
            if (e.target == v)
                e.free = false;
        }
    }
    for (int i = (int)path.size() - 1; i > 0; i--) {
        int u = path[i];
        int v = path[i - 1];
        for (NetworkEdge &e : network[u]) {
            if (e.target == v)
                e.free = true;
        }
    }
}

void Suurballe::reweight(const vector<int64_t> &dists) {
    for (int u = 0; u < N; u++) {
        for (NetworkEdge &e : network[u]) {
            if (e.free && dists[u] != 1 && dists[e.target] != 1) {
                e.reweighted_weight = e.weight + dists[u] - dists[e.target];
            }
        }
    }
    for (int u = 0; u < N; u++) {
        for (NetworkEdge &e : network[u]) {
            if (!e.free)
                e.reweighted_weight = 0;
        }
    }
}

InternalResult Suurballe::computeResult(int source, int sink) {
    InternalResult res;
    vector<vector<int>> paths;
    for (int iter = 0; iter < 2; iter++) {
        int u = source;
        paths.push_back({source});
        bool found = false;
        while (u != sink) {
            for (NetworkEdge &e : network[u]) {
                if (!e.free && !e.back) {
                    e.free = true;
                    res.cost += e.weight;
                    u = e.target;
                    found = true;
                    break;
                }
            }
            paths.back().push_back(u);
        }
        if (!found)
            break;
    }

    if (paths.size() < 2) {
        throw std::runtime_error("failed to extract two paths");
    }

    for (const auto &path : paths) {
        for (int i = 0; i + 1 < (int)path.size(); i++) {
            int u = path[i];
            int v = path[i + 1];
            for (NetworkEdge &e : network[u]) {
                if (e.target == v)
                    e.free = false;
            }
        }
    }

    res.path1 = paths[0];
    res.path2 = paths[1];
    return res;
}
