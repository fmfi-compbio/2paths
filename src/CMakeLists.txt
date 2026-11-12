cmake_minimum_required(VERSION 3.28)

include(ExternalProject)

project(2paths)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-O3)

find_package(OpenMP REQUIRED)
find_package(ortools CONFIG REQUIRED)

# KMC
message(STATUS "KMC3 will be built from source")
ExternalProject_Add(kmc
    GIT_REPOSITORY https://github.com/refresh-bio/KMC.git
    GIT_TAG d6c24dc88010508336d824bbffbf1ef3f49fddf8 # v3.2.4
    UPDATE_COMMAND ""
    BUILD_IN_SOURCE 1
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND make kmc
    INSTALL_COMMAND ""
)
ExternalProject_Get_Property(kmc SOURCE_DIR)
SET(KMC_SOURCE_DIR ${SOURCE_DIR})
SET(KMC_INCLUDE_DIR ${KMC_SOURCE_DIR})
add_library(KMC STATIC IMPORTED)
set_target_properties(KMC PROPERTIES IMPORTED_LOCATION ${KMC_INCLUDE_DIR}/bin/libkmc_core.a)
add_dependencies(KMC kmc)

### gfa_scorer
add_executable(gfa_scorer
        gfa_scorer.cpp
        cpp-lib/GfaUtils.cpp
        cpp-lib/Graph.cpp
        cpp-lib/GraphScorer.cpp)

target_include_directories(gfa_scorer PUBLIC ${KMC_INCLUDE_DIR})

target_link_libraries(gfa_scorer
        PRIVATE KMC
        PRIVATE OpenMP::OpenMP_CXX)

### paths_finder
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

target_link_libraries(paths_finder ortools::ortools)

### table_producer
add_executable(
        table_producer
        table_producer.cpp
        cpp-lib/GfaUtils.cpp
        cpp-lib/Graph.cpp
)

