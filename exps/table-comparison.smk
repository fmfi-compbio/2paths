from os.path import join as pjoin

WD = config['wd']
BETA = -50

PARSER = "gfa_scorer" # change to the path of gfa_scorer binary 
PATH_FINDER = "path_finder_binary" # change to the path of path_finder binary 
TABLE_PRODUCER = "table_producer_binary"  # change to the path of table_producer binary 
SAMPLE = "HG01123" # change to the ID of the SAMPLE, for whom reads were generated
HEADER_SCRIPT = "scripts/change_header.py"

Ns = [32]
CHROMs = ["chr20"] 
ALPHAs = [0,0.3,0.5,0.7,0.9,1]
ALGs = ["greedy", "mincostflow"]

rule all:
    input:
        expand(pjoin(WD, "{chrom}", "n{n}", "table-{algo}-alpha{alpha}"),
            chrom=CHROMs,
            n=Ns,
            algo=ALGs,
            alpha=ALPHAs)
    

rule score_graph_full:
    input:
        pre=pjoin(WD, "{chrom}", "reads.kmc_pre"),
        suf=pjoin(WD, "{chrom}", "reads.kmc_suf"),
        gfa=pjoin(WD, "{chrom}", "n{n}", "pangenome-full.gfa"),
    output:
        pgf=pjoin(WD, "{chrom}", "n{n}", "pangenome-full.pgf"),
    threads: workflow.cores
    params:
        kmc_prefix=pjoin(WD, "{chrom}", "reads"),
        beta=BETA,
        parser=PARSER,
        script_=HEADER_SCRIPT,
    shell:
        """
        {params.parser} {params.beta} {input.gfa} {params.kmc_prefix} {threads} > {output.pgf}.tmp
        python3 {params.script_} {input.gfa} {output.pgf}.tmp > {output.pgf}
        """

rule extract_2paths_full:
    input:
        pgf=pjoin(WD, "{chrom}", "n{n}", "pangenome-full.pgf"),
        gfa=pjoin(WD, "{chrom}", "n{n}", "pangenome-full.gfa"),
    output:
        gfa=pjoin(WD, "{chrom}", "n{n}", "pangenome-2paths-on-full-{algo}-alpha{alpha}.gfa"),
    params:
        path_finder=PATH_FINDER,
    shell:
        """
        export LD_LIBRARY_PATH="/opt/or-tools/lib/:$LD_LIBRARY_PATH"
        {params.path_finder} {input.pgf} {wildcards.alpha} {wildcards.algo} {input.gfa} > {output.gfa}
        """

rule contingency_table:
    input:
        pgf=pjoin(WD, "{chrom}", "n{n}", "pangenome-full.pgf"),
        full_gfa=pjoin(WD, "{chrom}", "n{n}", "pangenome-full.gfa"),
        hapl_gfa=pjoin(WD, "{chrom}", "n{n}", "pangenome-2paths-on-full-{algo}-alpha{alpha}.gfa"),
    output:
        table=pjoin(WD, "{chrom}", "n{n}", "table-{algo}-alpha{alpha}"),
    params:
        sample=SAMPLE,
    shell:
        """
        {TABLE_PRODUCER} {input.pgf} {input.hapl_gfa} {input.full_gfa} :{params.sample} > {output.table}
        """
