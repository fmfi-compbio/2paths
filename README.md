# Main

In this repo, we present code for producing results in the article: "Efficient algorithms for pangenome personalization".

## C++ source code

### Prerequisites 

To compile the source code, the following tools should be installed:

1. KMC API headers, which can be downloaded with `./download_include.sh`. Run the script from the root of the repo after cloning.
2. OpenMP library for C++ multithreading
3. OR-TOOLS library for min-cost flow.

OR-TOOLS can be downloaded from their [offical website](https://developers.google.com/optimization/install/cpp/).

You can use `cmake` to compile code with rules from `CMakeLists.txt`.
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
