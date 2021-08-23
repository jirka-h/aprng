![C workflow](https://github.com/jirka-h/aprng/actions/workflows/c-cpp.yml/badge.svg)
![Code scanning](https://github.com/jirka-h/aprng/actions/workflows/codeql-analysis.yml/badge.svg)

aprng
=====

Random Number Generators Based on the Aperiodic Infinite Words

Release 0.01

This is a set of programs used for the article:

Infinite Words with Well Distributed Occurrences

tree.c/tree.h => Library to traverse an infinite word based on the mapping like 0->01, 1->0
aprng.c/aprng.h => Functions 
  create_Fibonacci
  create_Fibonacci_with2
  create_Tribonacci
  create_AR
  to create infinite aperiodic words. Main library.
CombGenAPRNG.[ch] => Library to implement aperiodic number generator based on the infinite words as defined in aprng.c 
                     and on the RNGs as defined in TestU01 http://www.iro.umontreal.ca/~simardr/testu01/tu01.html
                     very flexible thanks to a huge range of RNGs defined in TestU01
Fibonacci_LCG_mix.c
Fibonacci2_LCG_1.c
Tribonacci_LCG_mix.c
AR1_4.c
AR2_4.c		=> Programs to produce an infinite stream of random numbers based on the aperiodic infinite words and LCGs. 
		   Main purpose is to test the properties of APRNGs. Some versions still relies on TestU01 the newer one implements
		   LCGs on it's now in order
		      =>remove dependencies on the external programs/libraries
		      =>greater performance
		
		
Requirements:
TestU01 http://www.iro.umontreal.ca/~simardr/testu01/tu01.html (only when 
GSL library http://www.gnu.org/software/gsl/ (only for the test programs) 
PractRand http://pracrand.sourceforge.net/PractRand.txt (only to test the properties of the produced numbers)

See the header of each C file for the instruction how to compile it and use it.

TODO:
  * Make a standalone library with Autotools
  * Implement high quality APRNGs based on WELL generators, RanLux generators and MIXMAX generators
