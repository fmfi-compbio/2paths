import sys
import re

regex = re.compile("([0-9]+[MXID])")


def parse_cigar(cigar):
    tokens = regex.split(cigar)
    tokens = [(int(x[:-1]), x[-1]) for x in tokens if x != ""]
    return tokens


def main_gam():
    stats_fn = sys.argv[1]

    for line in open(stats_fn) if stats_fn != "-" else sys.stdin:
        if line.startswith("#"):
            continue
        qname, is_perfect, identity, cigar = line.strip("\n").split("\t")

        if is_perfect == "0" and identity == "0":
            print(qname, is_perfect, identity, ".", ".", ".", sep="\t")
            continue

        stats = {"D": 0, "M": 0, "I": 0, "X": 0}
        bad_cigar = False
        for l, op in parse_cigar(cigar):
            if op not in stats:
                # NOTE: sometimes, the previous check on unmapped
                # does not work. In some cases, we have low identity
                # but no CIGAR, so I had to do this
                bad_cigar = True
                break
            stats[op] += l

        if bad_cigar:
            print(qname, "0", "0", ".", ".", ".", sep="\t")
            continue

        nm = stats["D"] + stats["I"] + stats["X"]
        query_alignment_length = stats["M"] + stats["I"] + stats["X"]

        identity = 1 - nm / (query_alignment_length + stats["D"])

        print(qname, is_perfect, identity, nm, query_alignment_length, cigar, sep="\t")


def main_bam():
    import pysam

    bam_fn = sys.argv[1]
    bam = pysam.AlignmentFile(bam_fn, "rb")

    for aln in bam:
        qname = aln.qname

        if aln.is_secondary or aln.is_supplementary:
            continue
        if aln.is_unmapped:
            print(qname, 0, 0, sep="\t")
            continue

        clipped = aln.get_cigar_stats()[1][4] + aln.get_cigar_stats()[1][5] > 0
        nm = aln.get_tag("NM")

        is_perfect = not clipped and nm == 0
        identity = 1 - nm / (aln.query_alignment_length + aln.get_cigar_stats()[0][2])

        print(
            qname,
            int(is_perfect),
            round(identity, 6),
            nm,
            aln.query_alignment_length,
            aln.cigarstring,
            sep="\t",
        )


if __name__ == "__main__":
    mode = sys.argv.pop(1)
    if mode == "gam":
        main_gam()
    elif mode == "bam":
        main_bam()
