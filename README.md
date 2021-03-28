# Database-Relations-Sort-and-Hash-Joins
Implementation of 2 multipass (two pass) algorithms for performing join on relations that are larger than what the one-pass algorithms of can handle.

Given M memory blocks and two large relations R(X,Y) and S(Y,Z), we develop iterator for the operations in implementation specifications.

## Implementation Specifications
- Given M memory blocks and two large relations R(X,Y) and S(Y,Z), we develop iterator for the operations in implementation specifications.
- All attributes, X, Y and Z are strings and Y may be a non-key attribute. 
- Join condition (R.Y==S.Y)
- Each block can store 100 tuples for both relations, R and S

### SortMerge Join
- open() - Creates sorted sublists for R and S, each of size M blocks
- getnext() - Uses 1 memory block for each sublist and gets minimum Y of R & S. Joins this minimum Y value with the other table and return. Checks for B(R)+B(S) < M*M
- close() - close all files

### Hash Join
- open() - Creates M hashed sublists for R and S
- getnext() - For each Ri and Si thus created, loads the smaller of the two in the
main memory and creates a search structure over it. Can use M blocks
to achieve this. Then recursively loads the other file in the remaining blocks
and for each record of this file, searches corresponding records (with same join
attribute value) from the other file.
- close() - close all files



### Run Instruction
```sh
<JoinRS.sh> <path of R file> <path of S file> <sort/hash> <M>
```

The script produces a file <R filename>\_<S filename>\_join.txt containing all the tuples formed as a result of join of R(X,Y) and S(Y,Z).

The benchmarking details can be found in the file analysis.pdf.

