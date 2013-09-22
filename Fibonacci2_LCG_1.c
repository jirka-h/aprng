/* vim: set expandtab cindent fdm=marker ts=2 sw=2: */

/*
gcc -g -Wall -Wextra -I./ -o Fibonacci2_LCG_1 Fibonacci2_LCG_1.c aprng.c tree.c CombGenAPRNG.c -lmylib -ltestu01
valgrind --tool=memcheck -v ./Fibonacci2_LCG_1
gcc -g -Wall -Wextra -I./ -o Fibonacci2_LCG_1 Fibonacci2_LCG_1.c aprng.c tree.c CombGenAPRNG.c   -lmylib -ltestu01 -lefence

gcc -O3 -DNDEBUG -Wall -Wextra -I./ -o Fibonacci2_LCG_1 Fibonacci2_LCG_1.c aprng.c tree.c CombGenAPRNG.c fd3.c -lmylib -ltestu01 -lrt

time ( stdbuf -oL ./Fibonacci2_LCG_1 0 >(stdbuf -oL ~/Downloads/PractRand/RNG_test stdin -tlmax 1G  -tlfail) ) &> /dev/shm/log
time ( stdbuf -oL ./Fibonacci2_LCG_1 0 >(pv -W | stdbuf -oL ~/Downloads/PractRand/RNG_test stdin -tlmax 1G  -tlfail) ) | tee /dev/shm/log
*/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "CombGenAPRNG.h"
#include "fd3.h"

typedef struct {
int e;
unsigned long long a;
unsigned long long c;
unsigned long long s;
} ulcg_CreatePow2LCGL_param_t;

unif01_Gen * ulcg_CreatePow2LCGL_init (ulcg_CreatePow2LCGL_param_t* d ) {
  return ulcg_CreatePow2LCGL(d->e, d->a, d->c, d->s);
}

int main(int argc, char **argv)
{
  const uint64_t TEST = (uint64_t) 1.0E10;
  char* path=NULL;
  long int n=-1;
  const int size = 27;
  APRNG_t *A[size];
  unif01_Gen* g[size];
  int i;
  unif01_Gen* lcg[3 * size];
  ulcg_CreatePow2LCGL_param_t lcg_param[] = {
    {64, 2862933555777941757ULL, 1ULL, 0ULL},
    {64, 3202034522624059733ULL, 1ULL, 0ULL},
    {64, 3935559000370003845ULL, 1ULL, 0ULL},
  };
  static const char* lcg_names[] = { "64_2862933555777941757", "64_3202034522624059733", "64_3935559000370003845" };
  char name[1024];
 
  size_t set = 3; // { 0 1 2 } Set we choose elements from
  size_t length = 3; // How many elements? Possible comb = set^length
  size_t e[length]; //State of each element
  size_t k, l, p;

  if ( argc > 3 || argc == 2) {
    //Either no argument ot two arguments
    fprintf(stderr, "Usage %s [N] [path], where N is in range 0 - %d\n", argv[0], size-1);
    for (i=1; i<argc; ++i) {
      fprintf(stderr, "Argument #%d:\t'%s'\n", i, argv[i]);
    }

    return 1;
  }

  if ( argc == 3 ) {
    char *p;
    n = strtol(argv[1], &p, 10);  //See also strtoull
    if ((p == argv[1]) || (*p != 0) || errno == ERANGE || (n < 0) || (n >= INT_MAX) || ( n>= size ) ) {
      fprintf(stderr, "Error when parsing %s. Generator index has to be in range in range 0 - %d. Parsed as %ld.\n", argv[1], size-1, n);
      return 1;
    }
    path = argv[2];
  }


  //Start with { 0 0 0 }
  for (k=0; k<length; ++k) {
    e[k] = 0;
  }

  l = 0;
  do {
    //fprintf(stderr, "{ ");
    for (k=0; k<length; ++k) {
      //fprintf (stderr, "%zu ", e[k]);
      lcg[l*length+k] = ulcg_CreatePow2LCGL_init(&lcg_param[e[k]]);
    }
    //fprintf(stderr, "}\n");
    snprintf(name, sizeof(name), "Fibonacci2 %s, %s, fill %s { %zu %zu %zu }", lcg_names[e[0]], lcg_names[e[1]], lcg_names[e[2]], e[0], e[1], e[2]);
    fprintf(stderr, "%s\n", name);
    
    A[l] = create_Fibonacci();
    g[l] = CreateCombGenAPRNGFill( A[l], &lcg[l*length], 3, 2, name, "");

    p = 0;
    ++e[p];
    while ( e[p] == set && p < length) {
      e[p]=0;
      ++p;
      ++e[p];
    }
    ++l;
  } while ( p < length );

#if 0
  for (i=0;i<3;++i) {
    unif01_WriteNameGen (lcg[i]);
    unif01_TimerGenWr (lcg[i], TEST, FALSE);
  }
#endif

  if (argc == 1) {
    for (i=0;i<size;++i) {
      unif01_WriteNameGen (g[i]);
      unif01_TimerGenWr (g[i], TEST, FALSE);
    }

    for (i=0;i<size;++i) {
      unif01_WriteNameGen (g[i]);
      bbattery_SmallCrush (g[i]);
      unif01_WriteNameGen (g[i]);
      bbattery_Crush (g[i]);
      unif01_WriteNameGen (g[i]);
      bbattery_BigCrush (g[i]);
    }
  } else {
    run_fd3(g[n], path, TEST);
  }


  return 0;
}

