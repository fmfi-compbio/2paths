#include "GraphScorer.h"

#include <climits>
#include <ostream>
#include <ranges>
#include <stack>
#include <string>
#include <omp.h>

GraphScorer::GraphScorer(Graph &g) : graph(g) {counts.resize(graph.scores.size());}

std::pair<int, int> GraphScorer::max_score(const size_t vertex_idx, CKMCFile &ckmc_file,
                                           const uint32 k, CKmerAPI &kmer) const {
    // vertex_id, depth
    std::stack<std::pair<size_t, size_t>> vertex_stack;
    // possibly slow because copying strings
    std::vector<std::string> string_stack;

    std::pair max_scores_ = {INT_MIN,0};

    vertex_stack.emplace(vertex_idx, 1);
    const std::string &vertex_idx_sequence = graph.sequences.at(vertex_idx);
    string_stack.push_back(vertex_idx_sequence);
    const auto vertex_idx_sequence_length = vertex_idx_sequence.length();

    while (!vertex_stack.empty()) {
        auto [curr_vertex, depth] = vertex_stack.top();
        vertex_stack.pop();
        // if going to a different branch, cut strings. possibly slow
        string_stack.resize(depth - 1);
        string_stack.push_back(graph.sequences.at(curr_vertex));

        std::size_t length = 0;
        for (const auto &s : string_stack)
            length += s.size();

        // length - vertex_idx_sequence_length is at least k - 1
        if (length >= (k - 1) + vertex_idx_sequence_length) {
            auto joined_string_stack =
                string_stack | std::views::join |
                std::views::take(vertex_idx_sequence_length + k - 1);
            std::string resulting_sequence;
            resulting_sequence.reserve(k - 1);
            std::ranges::copy(joined_string_stack,
                              std::back_inserter(resulting_sequence));
            auto counts_ = check_kmer_both_strands(resulting_sequence, ckmc_file, k, kmer);
            // (100 * pos1 + beta neg1) > (100 * pos2 + beta neg2) iff pos1 > pos2 because pos1 + neg1 = pos2 + neg2 = # of k-mers in string.
            if (counts_.first > max_scores_.first) {
                max_scores_ = counts_;
            }
        } else {
            for (auto child : graph.edges.at(curr_vertex)) {
                vertex_stack.emplace(child, depth + 1);
            }
        }
    }
    // vertices at the end of dag, where there are 0 extensions of length k-1
    if (max_scores_.first == INT_MIN) {
        return {0,0};
    }
    return max_scores_;
}

void GraphScorer::set_scores(const std::string &kmc_filename, const int num_threads) {
    CKMCFile kmc_db;

    if (!kmc_db.OpenForRA(kmc_filename)) {
        throw std::runtime_error("Cannot open KMC DB");
    }

    const auto k = kmc_db.KmerLength();

    #pragma omp parallel for num_threads(num_threads) schedule(dynamic)
    for (size_t vertex_idx = 0; vertex_idx < graph.sequences.size();
         vertex_idx++) {
        CKmerAPI kmer(k);
        counts[vertex_idx] = max_score(vertex_idx, kmc_db, k, kmer);
    }
    kmc_db.Close();
}

void GraphScorer::print_path_names(const std::vector<int> &path,
                                   std::ostream &out) const {
    std::vector<std::string> names;
    names.reserve(path.size());
    for (int u : path) {
        if (u < 0 || u >= (int)graph.names.size()) {
            throw std::runtime_error(
                "print_path_names: vertex id out of range");
        }
        names.push_back(graph.names[u]);
    }
    print_spaced(names, out);
}

void GraphScorer::print_path_names(const std::vector<std::string> &path,
                                   std::ostream &out) {
    print_spaced(path, out);
}

void GraphScorer::write_internal_format(std::ostream &out) const {
    out << graph.names.size() << '\n';
    for (int u = 0; u < (int)graph.names.size(); u++) {
        out << graph.names[u] << ' ' << counts[u].first << ' ' << counts[u].second << ' '
            << graph.edges[u].size() << '\n';
        for (int v : graph.edges[u]) {
            out << graph.names[v] << ' ';
        }
        out << '\n';
    }
}
void print_spaced(const std::vector<std::string> &items, std::ostream &out) {
    for (size_t i = 0; i < items.size(); i++) {
        if (i)
            out << ' ';
        out << items[i];
    }
}

std::pair<int, int> check_kmer_both_strands(const std::string &sequence,
                                            CKMCFile &kmc_db, const uint32_t k,
                                            CKmerAPI &kmer) {
    int pos = 0;
    int neg = 0;
    for (size_t i = 0; i + k <= sequence.length(); i++) {
        kmer.from_string(sequence.substr(i, k));

        bool seen = false;
        if (kmc_db.IsKmer(kmer)) {
            pos++;
            seen = true;
        }

        kmer.reverse();
        if (!seen && kmc_db.IsKmer(kmer)) {
            pos++;
            seen = true;
        }

        if (!seen) {
            neg++;
        }
    }
    return {pos, neg};
}
