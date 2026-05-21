cmake_minimum_required(VERSION 3.28)
project(2paths)

set(CMAKE_CXX_STANDARD 23)
add_compile_options(-O3)
include_directories(../include)

add_library(kmc_api
        ../include/kmc/kmc_api/kmc_file.cpp
        ../include/kmc/kmc_api/kmer_api.cpp
        ../include/kmc/kmc_api/mmer.cpp
)
target_include_directories(kmc_api PUBLIC include/kmc/kmc_api)
find_package(OpenMP REQUIRED)

add_executable(gfa_scorer
        gfa_scorer.cpp
        cpp-lib/GfaUtils.cpp
        cpp-lib/Graph.cpp
        cpp-lib/GraphScorer.cpp)


target_link_libraries(gfa_scorer PRIVATE kmc_api)
target_link_libraries(gfa_scorer PRIVATE OpenMP::OpenMP_CXX)

add_executable(
        paths_finder
        paths_finder.cpp
        cpp-lib/Graph.cpp
        cpp-lib/GfaUtils.cpp
        cpp-lib/Greedy.cpp
        cpp-lib/Solver.cpp
        cpp-lib/MinCostFlow.cpp
        cpp-lib/Suurballe.cpp
)

find_package(ortools CONFIG REQUIRED)

target_link_libraries(paths_finder ortools::ortools)

add_executable(
        table_producer
        table_producer.cpp
        cpp-lib/GfaUtils.cpp
        cpp-lib/Graph.cpp
)

