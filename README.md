FT_compiler is a compiler to detect transient memory errors developed in
HPC Research Lab, Ohio State University.

Relevant Papers : 
Tavarageri, Sanket, Sriram Krishnamoorthy, and Ponnuswamy Sadayappan.
"Compiler-assisted detection of transient memory errors." ACM SIGPLAN Notices
49.6 (2014): 204-215.

Prerequisites:
1) NTL: A Library for doing Number Theory
2) GMP: The GNU Multiple Precision Arithmetic Library
3) Barvinok: A library for counting the number of integer points in parametric
and non-parametric polytopes based on polylib.
4) PET: Polyhedral extraction tool extracts polyhedral model from C source 
using LLVM/Clang to ISL representation.
5) LLVM/Clang: >= 3.3
6) C/C++ compiler
7) Python


To compile the tool:
1) replace the path for prerequisites in Makefile,
and run make;

2) Link barvinok library:
export LD_LIBRARY_PATH=/Users/baow/Softwares/barvinok/barvinok-0.36_install/lib:$LD_LIBRARY_PATH

3) To run the tool:
./ft_compiler test/lu.c

Compiling generated resilient codes:
1) The correctness of resilient codes has been tested with Intel icc compiler.

2) While compiling the checksum-inserted codes, it may be necessary to include
-std=c99 -D_BSD_SOURCE flags.

To report issues please contact Wenlei Bao (bao.79@osu.edu)
