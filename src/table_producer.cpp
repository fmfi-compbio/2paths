#include "cpp-lib/GfaUtils.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <math.h>
#include <string>
#include <unordered_map>
using namespace std;

static void usage() {
    cerr << "Usage: ./program <graph.pgf> <beta> {<graph.gfa> {:<walkd_id>}*}+\n"
            "For each vertex that is found in some of given GFAs is for each "
            "given GFA "
            "printed it's count in the accepted walks of the GFA. <graph.pgf> "
            "is used just for scores printing.\n"
            "<beta> is an integer lesser or equal to 0. Beta = -100 is interpreted as -1 "
            "(multiplication by 100 is needed for the problem to be stated in integers).\n"
            "Vertices in GFA files should correspond to vertices in given PGF "
            "file.\n"
            "By default every W (walk line) from GFA is accepted walk."
            "If some <walk_id>s are specified, only the W lines with this "
            "sample_id are accepted.\n"
            "First line of output is number of vertices in <graph.pgf>. Each "
            "next line is in format:"
            "<vertex_name> <vertex_seq_size> <vertex_score> "
            "<count_in_ith_gfa>*.\n";
}

constexpr int MAX_SINGLE_SCORE = 100;

struct Program {
    string pgf;
    vector<string> gfas;
    vector<vector<string>> watched_walks;
    int beta;

    Program(int argc, char **argv) {
        pgf = argv[1];
        beta = std::stoi(argv[2]);
        for (int i = 3; i < argc; i++) {
            if (argv[i][0] == ':') {
                string code(argv[i]);
                code.erase(0, 1);
                watched_walks.back().push_back(code);
            } else {
                gfas.push_back(argv[i]);
                watched_walks.push_back({});
            }
        }
    }

    unordered_map<string, int> get_scores_from_pgf() {
        ifstream in(pgf);
        string dummy;
        int n;
        unordered_map<string, int> res;
        in >> dummy >> n >> dummy >> dummy;
        for (int i = 0; i < n; i++) {
            string name;
            int score, pos_count, neg_count, ngbs;
            in >> name >> pos_count >> neg_count >> ngbs;
            score = MAX_SINGLE_SCORE * pos_count + beta * neg_count;
            res[name] = score;
            for (int j = 0; j < ngbs; j++) {
                in >> dummy;
            }
        }
        return res;
    }

    void print_tables() {
        unordered_map<string, unordered_map<int, int>> counts;
        unordered_map<string, int> seq_sizes;
        auto scores = get_scores_from_pgf();
        for (int i = 0; i < gfas.size(); i++) {
            GfaReadOptions opts;
            opts.expected_walk_sample_ids = std::unordered_set<std::string>(
                watched_walks[i].begin(), watched_walks[i].end());
            if (watched_walks[i].empty()) {
                opts.match_all_walks = true;
            }
            ifstream in(gfas[i]);
            auto res = read_gfa(in, opts, true);
            for (int i = 0; i < res.graph.sequences.size(); i++) {
                seq_sizes[res.graph.names[i]] = res.graph.sequences[i].size();
            }
            for (auto &path : res.paths) {
                for (string &u : path) {
                    counts[u][i]++;
                }
            }
        }
        cout << seq_sizes.size() << endl;
        for (auto &[u, seq_size] : seq_sizes) {
            cout << u << " " << seq_size << " " << scores[u] << " ";
            for (int i = 0; i < gfas.size(); i++) {
                cout << counts[u][i];
                if (i != gfas.size() - 1) {
                    cout << " ";
                }
            }
            cout << endl;
        }
    }
};

int main(int argc, char **argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (argc < 3) {
        usage();
        return 1;
    }
    Program program(argc, argv);
    program.print_tables();
}
