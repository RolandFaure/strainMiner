# strainMiner
separating strains in metagenomic assemblies using ILP

*WARNING* strainMiner is not actively updated. Please go to [HairSplitter](https://github.com/RolandFaure/hairsplitter) to see a more up-to-date strain separator. The strainMiner presented here nonetheless is the only one implementing ILP, and thus can perform slightly better than HairSplitter in some cases.

## Installation

You will need to have installed in your PATH (for example in a conda environment):
- minimap2
- samtools
- racon

Additionally, you wil need to have installed:
- pysam
- pandas
- sklearn
- gurobipy with a valid gurobi license

Additionally, you will need to have installed `pysam`,  and `gurobipy`, to use with a valid gurobi license.
To use your gurobi license, modify lines 27 to 31 of `strainminer.py`

Then download and build strainMiner:
```
git clone https://github.com/RolandFaure/strainMiner.git
cd strainMiner
mkdir build
cd build
cmake ..
make
```

## Quick start

```
usage: strainminer.py [-h] -a ASSEMBLY -b BAM -r READS [-e ERROR_RATE] -o OUT [--window WINDOW]

optional arguments:
  -h, --help            show this help message and exit
  -a ASSEMBLY, --assembly ASSEMBLY
                        Assembly file in gfa format
  -b BAM, --bam BAM     Alignment file in BAM format
  -r READS, --reads READS
  -e ERROR_RATE, --error_rate ERROR_RATE
                        Estimation of the error rate of the data
  -o OUT, --out-folder OUT
                        Name of the output folder
  --window WINDOW       Size of window to perform read separation (must be at least twice shorter than average read length) [5000]
```
