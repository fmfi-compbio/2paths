# Experiments

```
# Get reference
wget https://s3-us-west-2.amazonaws.com/human-pangenomics/T2T/CHM13/assemblies/analysis_set/chm13v2.0.fa.gz
gunzip chm13v2.0.fa.gz
samtools faidx chm13v2.0.fa

# Get VCF from HPRCv1
wget https://s3-us-west-2.amazonaws.com/human-pangenomics/pangenomes/freeze/freeze1/minigraph-cactus/hprc-v1.1-mc-chm13/hprc-v1.1-mc-chm13.vcfbub.a100k.wave.vcf.gz
wget https://s3-us-west-2.amazonaws.com/human-pangenomics/pangenomes/freeze/freeze1/minigraph-cactus/hprc-v1.1-mc-chm13/hprc-v1.1-mc-chm13.vcfbub.a100k.wave.vcf.gz.tbi

snakemake [-n] -c 16 -p --use-conda --config ref=chm13v2.0.fa vcf=hprc-v1.1-mc-chm13.vcfbub.a100k.wave.vcf.gz wd=OUTPUT-DIRECTORY
# accuracy results:
ls OUTPUT-DIRECTORY/results.tsv
# efficiency results:
ls OUTPUT-DIRECTORY/efficiency.tsv
```

Obtaining tables with true vs. predicted vertex counts for paths:

```
snakemake [-n] -s table-comparison.smk -c 16 -p --use-conda --config wd=OUTPUT-DIRECTORY
```
