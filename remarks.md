# Remarks

## HCA strips

### Prestrips

Original `strainMiner` cuts the set of columns twice (on the set of all columns, then on each subset).
On the opposite, `strainMiner-py` cuts only one time the set of all columns.

## ILP-QBC strips

Original `strainMiner` applies ILP-QBC on each prestrip while `strainMiner-py` applies ILP-QBC on the matrix resulting from the merge of the prestrips.
* It explains why the refactored version is slower than the original version
* It could explain why the original version does not consider the ILP-QBC results because of column threshold issue