#pragma once

#include "Graph.h"
#include "kmc/kmc_api/kmc_file.h"

struct GraphScorer {
    Graph &graph;
    // positive and negative counts in each vertex + (k-1) next bases, to calculate 100 * pos + beta * neg
    std::vector<std::pair<int,int>> counts;

    GraphScorer(Graph &g);

    void set_scores(const std::string &kmc_filename, int num_threads);

    std::pair<int, int> max_score(size_t vertex_idx, CKMCFile &ckmc_file, uint32 k,
                                  CKmerAPI &kmer) const;

    void print_path_names(const std::vector<int> &path,
                          std::ostream &out) const;

    static void print_path_names(const std::vector<std::string> &path,
                                 std::ostream &out);

    void write_internal_format(std::ostream &out) const;
};

void print_spaced(const std::vector<std::string> &items, std::ostream &out);

std::pair<int, int> check_kmer_both_strands(const std::string &sequence,
                                            CKMCFile &kmc_db, uint32 k,
                                            CKmerAPI &kmer);
