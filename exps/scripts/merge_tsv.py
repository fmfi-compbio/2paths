import sys
import os
import glob


def main():
    WD = sys.argv[1]

    print(
        "Reference",
        "Chromosome",
        "N",
        "Read",
        "Perfect",
        "Identity",
        "NM",
        "AlignedLength",
        "CIGAR",
        sep="\t",
    )
    for tsv in glob.glob(os.path.join(WD, "*", "alignments.tsv")):
        chrom = tsv.split("/")[-2]
        for line in open(tsv):
            print("Linear", chrom, "0", line, sep="\t", end="")

    for tsv in glob.glob(os.path.join(WD, "*", "n*", "giraffe-*.tsv")):
        fields = tsv.split("/")
        graph = fields[-1][:-4].split("-")[1]
        n = tsv.split("/")[-2][1:]
        chrom = tsv.split("/")[-3]
        for line in open(tsv):
            print(graph, chrom, n, line, sep="\t", end="")


if __name__ == "__main__":
    main()
