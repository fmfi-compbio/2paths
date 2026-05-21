#include "GfaUtils.h"
#include "Graph.h"
#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

void print_joined(std::vector<std::string> &tokens, char split = '\t') {
    for (auto &tok : tokens) {
        std::cout << tok << split;
    }
    std::cout << std::endl;
}

std::vector<std::string> split_on_char(const std::string &line,
                                       const std::vector<char> &split_chars) {
    std::vector<std::string> toks;
    std::string cur;
    for (char c : line) {
        if (std::find(split_chars.begin(), split_chars.end(), c) !=
            split_chars.end()) {
            if (!cur.empty()) {
                toks.push_back(cur);
                cur.clear();
            }
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty())
        toks.push_back(cur);
    return toks;
}

GfaReadResult read_gfa(std::istream &in, const GfaReadOptions &opt,
                       bool more_than_2_walks) {
    GfaReadResult rr;
    GfaReadOptions opts;

    std::string line;
    int p_seen = 0;
    while (std::getline(in, line)) {
        if (line.empty())
            continue;
        auto toks = split_on_char(line, {'\t'});
        if (toks.empty())
            continue;

        const std::string &type = toks[0];
        if (type == "S") {
            if (toks.size() < 3) {
                throw std::runtime_error(
                    "invalid S line (expected at least 3 fields)");
            }
            const std::string &name = toks[1];
            rr.graph.add_vertex(name, opts.default_score, toks[2]);
            int id = rr.graph.get_id(name);
            rr.graph.scores[id] = opts.default_score;
        } else if (type == "L") {
            if (toks.size() < 6) {
                throw std::runtime_error(
                    "invalid L line (expected at least 6 fields)");
            }
            const std::string &from = toks[1];
            const std::string &from_or = toks[2];
            const std::string &to = toks[3];
            const std::string &to_or = toks[4];
            if (opt.require_positive_orientation) {
                if (from_or != "+" || to_or != "+") {
                    throw std::runtime_error(
                        "negative orientation edges are not supported");
                }
            }
            int u = rr.graph.get_id(from);
            int v = rr.graph.get_id(to);
            rr.graph.add_edge(u, v);
        } else if (type == "W" && opt.parse_expected_paths_from_W) {
            if (toks.size() < 7) {
                throw std::runtime_error(
                    "invalid W line (expected at least 7 fields)");
            }
            const std::string &sample_id = toks[1];
            if (!opt.match_all_walks &&
                opt.expected_walk_sample_ids.find(sample_id) ==
                    opt.expected_walk_sample_ids.end()) {
                continue;
            }

            auto path = split_on_char(toks[6], {'<', '>'});

            int hap = -1;
            if (toks.size() >= 3) {
                const std::string &hap_s = toks[2];
                auto *b = hap_s.data();
                auto *e = hap_s.data() + hap_s.size();
                (void)std::from_chars(b, e, hap);
            }

            if (more_than_2_walks) {
                rr.paths.push_back(path);
            } else {
                if (hap == 0) {
                    rr.path1 = std::move(path);
                } else if (hap == 1) {
                    rr.path2 = std::move(path);
                } else {
                    if (p_seen == 0) {
                        rr.path1 = std::move(path);
                    } else {
                        rr.path2 = std::move(path);
                    }
                    p_seen++;
                }
            }
        }
    }
    return rr;
}

void output_new_gfa(std::istream &in, Graph &graph,
                    std::vector<std::string> &path1,
                    std::vector<std::string> &path2) {
    std::unordered_set<std::string> used_vertices;
    used_vertices.insert(path1.begin(), path1.end());
    used_vertices.insert(path2.begin(), path2.end());
    std::unordered_map<std::string, std::unordered_set<std::string>> used_edges;
    for (int i = 1; i < path1.size(); i++) {
        used_edges[path1[i - 1]].insert(path1[i]);
    }
    for (int i = 1; i < path2.size(); i++) {
        used_edges[path2[i - 1]].insert(path2[i]);
    }
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty())
            continue;
        auto toks = split_on_char(line, {' ', '\t'});
        if (toks.empty())
            continue;

        const std::string &type = toks[0];
        if (type == "S") {
            if (toks.size() < 3) {
                throw std::runtime_error(
                    "invalid S line (expected at least 3 fields)");
            }
            const std::string &name = toks[1];
            if (used_vertices.find(name) != used_vertices.end()) {
                print_joined(toks, '\t');
            }
        } else if (type == "L") {
            if (toks.size() < 6) {
                throw std::runtime_error(
                    "invalid L line (expected at least 6 fields)");
            }
            const std::string &from = toks[1];
            const std::string &to = toks[3];
            if (used_edges[from].find(to) != used_edges[from].end()) {
                print_joined(toks, '\t');
            }
        }
    }
    int i = 0;
    for (auto &path : {path1, path2}) {
        // SampleId is defined as 0, HapIndex, SeqId, SeqStart and SeqEnd are defined as 0 or 1 respectively
        std::string hapindex = (i == 0) ? "0" : "1";
        std::string w_path;
        w_path.reserve(path.size() * 2);
        for (auto &u : path) {
            w_path += ">" + u;
        }
        std::vector<std::string> tokens = {
            "W", "0", hapindex, hapindex, hapindex, hapindex, w_path};
        print_joined(tokens, '\t');
        i++;
    }
}
