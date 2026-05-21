#pragma once

#include "Graph.h"
#include "kmc/kmc_api/kmc_file.h"

constexpr int MAX_SINGLE_SCORE = 100;

struct GraphScorer {
    Graph &graph;

    GraphScorer(Graph &g);

    void set_scores(const std::string &kmc_filename, int beta, int num_threads);

    int max_score(size_t vertex_idx, CKMCFile &ckmc_file, uint32 k,
                  CKmerAPI &kmer, int beta) const;

    void print_path_names(const std::vector<int> &path,
                          std::ostream &out) const;

    static void print_path_names(const std::vector<std::string> &path,
                                 std::ostream &out);

    void write_internal_format(std::ostream &out);
};

void print_spaced(const std::vector<std::string> &items, std::ostream &out);

std::pair<int, int> check_kmer_both_strands(const std::string &sequence,
                                            CKMCFile &kmc_db, uint32 k,
                                            CKmerAPI &kmer);
