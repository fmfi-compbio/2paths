#include "Graph.h"
#include "Types.h"

#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>
using namespace::std;

int Graph::add_vertex(const std::string &name, int score,
                      const std::string &sequence) {
    auto it2 = id_of.find(name);
    if (it2 != id_of.end()) {
        throw std::runtime_error("vertex already exists: " + name);
    }
    int id = (int)names.size();
    id_of.emplace(name, id);
    names.push_back(name);
    scores.push_back(score);
    edges.emplace_back();
    sequences.push_back(sequence);
    return id;
}

int Graph::get_id(const std::string &name) const {
    auto it = id_of.find(name);
    if (it == id_of.end()) {
        throw std::runtime_error("unknown vertex: " + name);
    }
    return it->second;
}

void Graph::add_edge(int u, int v) {
    if (u < 0 || v < 0 || u >= (int)edges.size() || v >= (int)edges.size()) {
        throw std::runtime_error("add_edge: vertex id out of range");
    }
    edges[u].push_back(v);
}

std::vector<int> get_toposort(std::vector<std::vector<int>> const &edges) {
    int N = edges.size();
    std::vector<int> indegs(N, 0);
    for (int u = 0; u < N; u++) {
        for (int v : edges[u]) {
            indegs[v]++;
        }
    }
    std::vector<int> ready;
    for (int u = 0; u < N; u++) {
        if (indegs[u] == 0) {
            ready.push_back(u);
        }
    }
    std::vector<bool> visited(N, false);
    std::vector<int> res;
    while (!ready.empty()) {
        int u = ready.back();
        visited[u] = true;
        res.push_back(u);
        ready.pop_back();
        for (int v : edges[u]) {
            if (!visited[v]) {
                indegs[v]--;
                if (indegs[v] == 0) {
                    ready.push_back(v);
                }
            }
        }
    }
    return res;
}

std::vector<int> get_reachable(int start,
                               const std::vector<std::vector<int>> &edges) {
    std::vector<bool> visited(edges.size(), false);
    std::queue<int> q;
    q.push(start);
    visited[start] = true;
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (int v : edges[u]) {
            if (!visited[v]) {
                visited[v] = true;
                q.push(v);
            }
        }
    }
    std::vector<int> result;
    for (int u = 0; u < edges.size(); u++) {
        if (visited[u])
            result.push_back(u);
    }
    return result;
}

std::vector<std::vector<int>>
get_reversed_edges(std::vector<std::vector<int>> const &edges) {
    std::vector<std::vector<int>> res(edges.size());
    for (int u = 0; u < edges.size(); u++) {
        for (int v : edges[u]) {
            res[v].push_back(u);
        }
    }
    return res;
}

std::vector<std::vector<int>>
get_undirected_edges(std::vector<std::vector<int>> const &edges) {
    auto rev_edges = get_reversed_edges(edges);
    int N = edges.size();
    std::vector<std::vector<int>> res(N);
    for (int u = 0; u < edges.size(); u++) {
        std::unordered_set<int> updated(edges[u].begin(), edges[u].end());
        updated.insert(rev_edges[u].begin(), rev_edges[u].end());
        res[u] = std::vector<int>(updated.begin(), updated.end());
    }
    return res;
}

std::vector<int> get_articulations(std::vector<std::vector<int>> const &edges) {
    int N = edges.size();
    std::vector<int> depths(N, -1), back_edges(N, 0), articulations;
    for (int u = 0; u < N; u++) {
        if (depths[u] == -1) {
            struct Frame {
                int u;
                size_t next_idx;
                int over_u;
                int children_cnt;
                bool is_articulation;
            };
            std::vector<Frame> stack;
            stack.push_back({u, 0, 0, 0, false});
            depths[u] = 0;
            while (!stack.empty()) {
                Frame &cur = stack.back();
                if (cur.next_idx < edges[cur.u].size()) {
                    int v = edges[cur.u][cur.next_idx++];
                    if (depths[v] == -1) {
                        depths[v] = depths[cur.u] + 1;
                        stack.push_back({v, 0, 0, 0, false});
                    } else if (depths[v] < depths[cur.u] &&
                               depths[v] != depths[cur.u] - 1) {
                        cur.over_u++;
                        back_edges[v]++;
                    } else if (depths[v] > depths[cur.u]) {
                        cur.over_u--;
                    }
                } else {
                    if ((depths[cur.u] == 0 && cur.children_cnt > 1) ||
                        cur.is_articulation) {
                        articulations.push_back(cur.u);
                    }
                    int over_v = cur.over_u;
                    stack.pop_back();
                    if (!stack.empty()) {
                        Frame &parent = stack.back();
                        if (depths[parent.u] != 0 &&
                            over_v == back_edges[parent.u]) {
                            parent.is_articulation = true;
                        }
                        parent.over_u += over_v;
                        back_edges[parent.u] = 0;
                        parent.children_cnt++;
                    }
                }
            }
        }
    }
    return articulations;
}

std::pair<std::vector<std::string>, std::vector<Graph>>
Graph::split_by_articulations(std::string const &s, std::string const &t) const {
    int start = get_id(s), target = get_id(t);
    auto v1 = get_reachable(start, edges);
    std::unordered_set<int> v_reach(v1.begin(), v1.end());
    auto redges = get_reversed_edges(edges);
    auto v2 = get_reachable(target, redges);
    std::vector<int> reachable;
    for (int u : v2) {
        if (v_reach.find(u) != v_reach.end()) {
            reachable.push_back(u);
        }
    }
    auto induced = get_induced_subgraph(reachable);
    auto undir_edges = get_undirected_edges(induced.edges);
    auto a = get_articulations(undir_edges);
    std::unordered_set<int> artics(a.begin(), a.end());
    artics.erase(induced.get_id(s));
    artics.erase(induced.get_id(t));
    auto toposort = get_toposort(induced.edges);

    std::vector<Graph> res;
    std::vector<int> component;
    std::vector<std::string> sorted_artics;
    for (int i = 0; i < toposort.size(); i++) {
        int u = toposort[i];
        component.push_back(u);
        if (artics.find(u) != artics.end()) {
            sorted_artics.push_back(induced.names[u]);
            res.push_back(induced.get_induced_subgraph(component));
            component = {u};
        }
    }
    res.push_back(induced.get_induced_subgraph(component));
    return {sorted_artics, res};
}

Graph Graph::get_induced_subgraph(std::vector<int> const &vertices) const {
    Graph res;
    for (int u : vertices) {
        res.add_vertex(names[u], scores[u], sequences[u]);
    }
    std::unordered_set<int> v_set(vertices.begin(), vertices.end());
    for (int u : vertices) {
        for (int v : edges[u]) {
            if (v_set.find(v) != v_set.end()) {
                res.add_edge(res.get_id(names[u]), res.get_id(names[v]));
            }
        }
    }
    return res;
}

vector<vector<NetworkEdge>> create_network(const Graph &graph, Alpha alpha) {
    vector<vector<NetworkEdge>> network;
    network.assign(4 * (int)graph.edges.size(), {});

    for (int i = 0; i < (int)graph.edges.size(); i++) {
        int u = 4 * i;
        network[u].push_back(NetworkEdge{
            u + 2, -alpha.get_first_pass_score_numerator(graph.scores[i]),
            true, false});
        network[u + 1].push_back(NetworkEdge{
            u + 3, -alpha.get_second_pass_score_numerator(graph.scores[i]),
            true, false});
        network[u + 2].push_back(
            NetworkEdge{u, -alpha.get_first_pass_score_numerator(graph.scores[i]),
                 false, true});
        network[u + 3].push_back(NetworkEdge{
            u + 1, -alpha.get_second_pass_score_numerator(graph.scores[i]),
            false, true});

        for (int j : graph.edges[i]) {
            int v = 4 * j;
            for (int x = 0; x < 2; x++) {
                for (int y = 0; y < 2; y++) {
                    network[u + x + 2].push_back(
                        NetworkEdge{v + y, 0, true, false});
                    network[v + y].push_back(
                        NetworkEdge{u + x + 2, 0, false, true});
                }
            }
        }
    }
    return network;
}

