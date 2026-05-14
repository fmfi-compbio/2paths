import sys
import os
import glob
import datetime
import time


def parse_time(fn):
    secs, ram = 0, 0

    for i, line in enumerate(open(fn)):
        if i == 4:  # wall clock
            t = line.strip("\n").split(" ")[-1]
            t = t.split(".")[0]  # strip milliseconds
            if t.count(":") == 1:  # min:sec
                t = time.strptime(t, "%M:%S")
                secs = int(
                    datetime.timedelta(
                        hours=t.tm_hour, minutes=t.tm_min, seconds=t.tm_sec
                    ).total_seconds()
                )
            elif t.count(":") == 2:  # hour:min:sec
                t = time.strptime(t, "%H:%M:%S")
                secs = int(
                    datetime.timedelta(
                        hours=t.tm_hour, minutes=t.tm_min, seconds=t.tm_sec
                    ).total_seconds()
                )
        elif i == 9:  # Max RAM in kbytes
            ram = round(float(line.strip("\n").split(" ")[-1]) / 1024 / 1024, 1)

    return secs, ram


def main():
    WD = sys.argv[1]

    print(
        "Chromosome",
        "N",
        "Step",
        "Time (s)",
        "RAM (GB)",
        sep="\t",
    )

    for fn in glob.glob(os.path.join(WD, "benchmark", "chr*", "*.time")):
        fields = fn.split("/")

        step = fields[-1][:-5]
        chrom = fields[-2]

        time, ram = parse_time(fn)
        print(chrom, "0", step, time, ram, sep="\t")

    for fn in glob.glob(os.path.join(WD, "benchmark", "chr*", "n*", "*.time")):
        fields = fn.split("/")

        step = fields[-1][:-5]
        n = fields[-2][1:]
        chrom = fields[-3]

        time, ram = parse_time(fn)
        print(chrom, n, step, time, ram, sep="\t")


if __name__ == "__main__":
    main()
