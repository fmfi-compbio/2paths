#pragma once

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

struct TwoPathsResult {
    double score = 0.0;
    std::vector<std::string> path1, path2;
};

struct Alpha {
    int64_t numerator = 0;
    int64_t denominator = 1;

    Alpha() {}

    void set(int64_t numerator, int64_t denominator) {
        this->denominator = denominator;
        this->numerator = numerator;
    }

    int64_t get_first_pass_score_numerator(int64_t vertex_score) const {
        return vertex_score * denominator;
    }

    int64_t get_second_pass_score_numerator(int64_t vertex_score) const {
        int64_t coeff = numerator;
        if (vertex_score < 0) {
            coeff = 2 * denominator - numerator;
        }
        return vertex_score * coeff;
    }

    int64_t get_both_passes_score_numerator(int64_t vertex_score) const {
        return get_first_pass_score_numerator(vertex_score) +
               get_second_pass_score_numerator(vertex_score);
    }
};
