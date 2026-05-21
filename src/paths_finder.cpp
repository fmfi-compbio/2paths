#include "cpp-lib/GfaUtils.h"
#include "cpp-lib/Graph.h"
#include "cpp-lib/Greedy.h"
#include "cpp-lib/MinCostFlow.h"
#include "cpp-lib/Suurballe.h"
#include "cpp-lib/Types.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <memory>
#include <optional>
#include <ratio>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
using namespace std;

static void usage() {
    cerr << "Usage: ./program <graph.pgf> <alpha> <algorithm> (Optional "
            "<graph.gfa>); "
            "algorithm is either suurballe "
            "(works only for nonnegative scores), or mincostflow, or greedy.\n"
            "Program finds best pair of paths in the graph between vertices "
            "specified in <graph.pgf> based on alpha and scores of vertices. "
            "<graph.pgf> must match <graph.gfa>.\n"
            "<alpha> is a number between 0 and 1 with two decimal places.\n"
            "If <graph.gfa> is specified it outputs given GFA, but without "
            "unused vertices and edges in found paths. "
            "It also adds 2 W (walk lines) at the end of produced GFA. They "
            "represent found 2 paths.\n"
            "If <graph.gfa> is not specified, than it outputs optimal score "
            "and 2 found paths represented as sequences of names of vertices "
            "from <graph.pgf>.\n";
}

struct Program {
    string pgf_path, algorithm, gfa_path = "";
    string start, target;
    Alpha alpha;
    bool no_gfa = false;
    const int alpha_denominator = 100;

    Program(int argc, char **argv) {
        vector<string> args;
        for (int i = 0; i < argc; i++)
            args.push_back(argv[i]);
        pgf_path = args[1];
        alpha.set(parse_alpha(string(args[2])), alpha_denominator);
        algorithm = args[3];
        if (argc >= 5) {
            gfa_path = args[4];
        } else {
            no_gfa = true;
        }
    }

    void throw_wrong_alpha(string &alpha_str) {
        throw runtime_error("Alpha given as " + alpha_str +
                            " is in incorrect format. Should be number between "
                            "0 and 1 with at most 2 given decimal places.");
    }

    int parse_alpha(string alpha_str) {
        for (int i = 0; i < alpha_str.size(); i++) {
            if (i == 1) {
                if (alpha_str[i] != '.') {
                    throw_wrong_alpha(alpha_str);
                }
            } else {
                if ('0' > alpha_str[i] || alpha_str[i] > '9') {
                    throw_wrong_alpha(alpha_str);
                }
            }
        }
        if (alpha_str.size() == 1) {
            alpha_str += ".00";
        }
        if (alpha_str.size() == 3) {
            alpha_str += "0";
        }
        alpha_str.erase(alpha_str.find('.'), 1);
        int res = stoi(alpha_str);
        if (res > alpha_denominator) {
            throw_wrong_alpha(alpha_str);
        }
        return stoi(alpha_str);
    }

    Graph read_pgf() {
        Graph graph;
        ifstream in(pgf_path);
        if (!in) {
            throw runtime_error("failed to open scores file " + pgf_path);
        }
        int n = -1;
        string header;
        in >> header;
        assert(header == "Header:");
        if (!(in >> n)) {
            throw runtime_error(
                "failed to read vertex count from scores file " + pgf_path);
        }
        in >> start >> target;
        if (n < 0) {
            throw runtime_error("invalid vertex count in scores file " +
                                pgf_path);
        }
        unordered_map<string, int> score_of;
        score_of.reserve((size_t)n);
        vector<pair<string, string>> edgs;
        for (int i = 0; i < n; i++) {
            string name;
            int score = 0, outdeg = 0;
            if (!(in >> name >> score >> outdeg)) {
                cerr << name << " " << score << " " << outdeg << std::endl;
                throw runtime_error("invalid vertex record in scores file " +
                                    pgf_path);
            }
            if (auto [it, inserted] = score_of.emplace(name, (int)score);
                !inserted) {
                throw runtime_error("duplicate vertex name '" + name +
                                    "' in scores file " + pgf_path);
            }
            graph.add_vertex(name, score, "");
            for (int j = 0; j < outdeg; j++) {
                string ngb_name;
                in >> ngb_name;
                edgs.push_back({name, ngb_name});
            }
        }
        for (auto &[u, v] : edgs) {
            graph.add_edge(graph.get_id(u), graph.get_id(v));
        }
        return graph;
    }

    TwoPathsResult solve(Graph &graph, string s, string t,
                         bool mcf_split_on_articulations = true) {
        cerr << "Running algorithm " << algorithm << std::endl;
        unique_ptr<Solver> solver = [&]() -> std::unique_ptr<Solver> {
            if (algorithm == "suurballe") {
                bool all_positive = true;
                for (int &score : graph.scores) {
                    if (score < 0) {
                        all_positive = false;
                        score = 0;
                    }
                }
                if (!all_positive) {
                    std::cerr << "Warning: some scores are negative. All negative scores were substituted with zero.\n";
                }
                return make_unique<Suurballe>(graph, s, t, alpha);
            } else if (algorithm == "mincostflow") {
                return make_unique<MinCostFlow>(graph, s, t, alpha,
                                                mcf_split_on_articulations);
            } else if (algorithm == "greedy") {
                return make_unique<Greedy>(graph, s, t, alpha);
            } else {
                throw invalid_argument("Unknown algorithm");
            }
        }();
        TwoPathsResult result = solver->solve();
        assert(solver->resultValid(result));
        return result;
    }
};

int main(int argc, char **argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (argc < 4) {
        usage();
        return 1;
    }
    Program parser(argc, argv);
    cerr << "Reading pgf" << endl;
    Graph g = parser.read_pgf();
    cerr << "Solving" << endl;

    const int decimal_precision = 6;
    std::cout << std::fixed << std::showpoint;
    std::cout << std::setprecision(decimal_precision);
    std::cerr << std::fixed << std::showpoint;
    std::cerr << std::setprecision(decimal_precision);

    TwoPathsResult res = parser.solve(g, parser.start, parser.target);
    auto reachable = get_reachable(g.get_id(parser.start), g.edges);
    if (find(reachable.begin(), reachable.end(), g.get_id(parser.target)) == reachable.end()) {
        throw runtime_error("In given graph there is no path between source and target vertex from .pgf file .");
    }
    cerr << "Score of found paths: " << res.score << endl;
    if (!parser.no_gfa) {
        ifstream in(parser.gfa_path);
        output_new_gfa(in, g, res.path1, res.path2);
    } else {
        cout << res.score << endl << endl;
        for (string x : res.path1) {
            cout << x << " ";
        }
        cout << endl << endl;
        for (string x : res.path2) {
            cout << x << " ";
        }
        cout << endl;
    }
}
