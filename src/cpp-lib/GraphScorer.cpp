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
    // XXX: possibly slow because copying strings
    std::vector<std::string> string_stack;

    std::pair max_scores_ = {INT_MIN,0};

    // XXX: we don't have a score for sinks

    vertex_stack.emplace(vertex_idx, 1);
    const std::string &vertex_idx_sequence = graph.sequences.at(vertex_idx);
    string_stack.push_back(vertex_idx_sequence);
    const auto vertex_idx_sequence_length = vertex_idx_sequence.length();

    std::pair<int, int> counts_prefix = {0,0}; // Count now, sum later to suffix
    if (vertex_idx_sequence_length >= k) {
        counts_prefix = check_kmer_both_strands(vertex_idx_sequence, 0, vertex_idx_sequence_length, ckmc_file, k, kmer);
    }

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
            // std::ranges::take_view<std::ranges::join_view<std::vector<std::string>>>
            auto joined_string_stack =
                string_stack | std::views::join |
                std::views::take(vertex_idx_sequence_length + k - 1);
            std::string resulting_sequence;
            auto count = std::ranges::distance(joined_string_stack);
            resulting_sequence.reserve(static_cast<std::size_t>(count));
            std::ranges::copy(joined_string_stack,
                              std::back_inserter(resulting_sequence));
            // std::cerr << count << " " << resulting_sequence.size() << std::endl;

            int start = vertex_idx_sequence_length < k ? 0 : vertex_idx_sequence_length - (k - 1);
            std::pair<int, int> counts_suffix = check_kmer_both_strands(resulting_sequence, start, resulting_sequence.size(), ckmc_file, k, kmer);
            // (100 * pos1 + beta neg1) > (100 * pos2 + beta neg2) iff pos1 > pos2 because pos1 + neg1 = pos2 + neg2 = # of k-mers in string.
            counts_suffix = {counts_prefix.first + counts_suffix.first, counts_prefix.second + counts_suffix.second};
            if (counts_suffix.first > max_scores_.first) {
                max_scores_ = counts_suffix;
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
                                            const size_t start,
                                            const size_t end,
                                            CKMCFile &kmc_db, const uint32_t k,
                                            CKmerAPI &kmer) {
    int pos = 0;
    int neg = 0;
    for (size_t i = start; i + k <= end; i++) {
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
