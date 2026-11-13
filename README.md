# Main

In this repo, we present code for producing results in the article: "Efficient algorithms for pangenome personalization".

## C++ source code

### Prerequisites 

To compile the source code, the following libraries should be installed:

1. OpenMP library for C++ multithreading
1. OR-TOOLS library for min-cost flow.

OR-TOOLS binary distribution can be downloaded from their [offical website](https://developers.google.com/optimization/install/cpp/).

You can use `cmake` to compile code with rules from `CMakeLists.txt`.
```
git clone https://github.com/fmfi-compbio/2paths.git
cd 2paths
# e.g., ubuntu 24.04
wget https://github.com/google/or-tools/releases/download/v9.12/or-tools_amd64_ubuntu-24.04_cpp_v9.12.4544.tar.gz
tar xvfz or-tools_amd64_ubuntu-24.04_cpp_v9.12.4544.tar.gz
cmake -S src -B src/build -DCMAKE_PREFIX_PATH="$PWD/or-tools_x86_64_Ubuntu-24.04_cpp_v9.12.4544"
cmake --build src/build -j8
```
In that case you should add argument `-DCMAKE_PREFIX_PATH="<path/to/or-tools/dir>"` to `cmake` before running.

### Programs

There are three executables, that can be compiled. 

1. `gfa_scorer` is an executable that for a given graph in GFA, and $k$-mers database outputs scores of vertices in `pgf` format on standart output.
2. `paths_finder` is an executable that for a given graph in PGF runs 2-paths algorithm, and returns two paths in specificed format.
3. `table_producer` is an executable that for given set of GFAs and walks returns tables with stats that are further used in section "Recovery of true vertices".


> [!IMPORTANT]  
> To run `paths_finder` properly, header of graph in `PGF` should be changed, which can be done by `exps/scripts/change_header.py`:
```
python change_header.py graph.gfa score_graph.pgf > scored_graph_with_header.pgf
```
