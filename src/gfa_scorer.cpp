#include "cpp-lib/GfaUtils.h"
#include "cpp-lib/GraphScorer.h"

#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char **argv) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    if (argc != 5) {
        std::cerr << "Usage: ./gfa_scorer <integer from -100 to 0> graph.gfa kmers.kmc <number of threads>"
                  << std::endl;
        return 1;
    }

    int beta = std::stoi(argv[1]);
    if (beta >= 0) {
        std::cerr << "Beta should be negative or zero, given beta is " << beta
                  << std::endl;
        return 1;
    }
    std::string gfa_filename = argv[2];
    std::string kmc_filename = argv[3];
    int number_of_threads = std::stoi(argv[4]);

    std::ifstream stream(gfa_filename);
    if (!stream) {
        std::cerr << "Failed to open file " << gfa_filename << "\n";
        return 1;
    }

    try {
        GfaReadOptions opt;
        auto rr = read_gfa(stream, opt, false);
        GraphScorer gs(rr.graph);
        gs.set_scores(kmc_filename, beta, number_of_threads);
        gs.write_internal_format(std::cout);
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 2;
    }
}
