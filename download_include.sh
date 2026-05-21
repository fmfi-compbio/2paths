#!/usr/bin/bash

base="https://raw.githubusercontent.com/refresh-bio/KMC/refs/heads/master/kmc_api/"

suffixes=("kmc_file.cpp" "kmc_file.h" "kmer_api.cpp" "kmer_api.h" "kmer_defs.h" "mmer.cpp" "mmer.h")

for s in "${suffixes[@]}"; do
    wget -P "include/kmc/kmc_api" "${base}${s}"
done
