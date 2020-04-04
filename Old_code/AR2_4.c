/* vim: set expandtab cindent fdm=marker ts=2 sw=2: */

/*
gcc -g -Wall -Wextra -I./ -o AR2_4 AR2_4.c aprng.c tree.c CombGenAPRNG.c -lmylib -ltestu01
valgrind --tool=memcheck -v ./AR2_4
gcc -g -Wall -Wextra -I./ -o AR2_4 AR2_4.c aprng.c tree.c CombGenAPRNG.c   -lmylib -ltestu01 -lefence

gcc -O3 -DNDEBUG -Wall -Wextra -I./ -o AR2_4 AR2_4.c aprng.c tree.c CombGenAPRNG.c fd3.c -lmylib -ltestu01 -lgsl -lgslcblas -lrt
*/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include<gsl/gsl_permutation.h>
#include "CombGenAPRNG.h"
#include "fd3.h"

typedef struct {
unsigned long long m;
unsigned long long a;
unsigned long long c;
unsigned long long s;
} ulcg_CreatePow2LCGL_param_t;

unif01_Gen * ulcg_CreatePow2LCGL_init (ulcg_CreatePow2LCGL_param_t* d ) {
  if ( d->m < 65 ) {
    return ulcg_CreatePow2LCGL((int) d->m, d->a, d->c, d->s);
  } else {
  return ulcg_CreateLCG(d->m, d->a, d->c, d->s);
  }
}

int main(int argc, char **argv)
{
  const uint64_t TEST = (uint64_t) 1.0E10;
  char* path=NULL;
  long int n=-1;
  const int size = 5;
  int i;
  APRNG_t* A[size];
  unif01_Gen* g[size];
  unif01_Gen* lcg[3*size];
  ulcg_CreatePow2LCGL_param_t lcg_param[] = {
    {140737488355213ULL, 71971110957370ULL, 0ULL, 1ULL},  // LCG(2^47 − 115, 71971110957370,0)
    {9223372036854775783ULL, 2307085864ULL, 0ULL, 1ULL},  // LCG(2^63-25, 2307085864, 0)
    {59, 302875106592253ULL, 0ULL, 1ULL},
    {63, 19073486328125ULL, 1ULL, 0ULL},
    {63, 9219741426499971445ULL, 1ULL, 0ULL},
  };
  static const char* lcg_names[] = { 
    "LCG(2^47 − 115, 71971110957370,0)",
    "LCG(2^63-25, 2307085864, 0)",
    "LCG(2^59, 13^13,0)",
    "LCG(2^63, 5^19,1)",
    "LCG(2^63, 9219741426499971445,1)" };
  char name[100];

  if ( argc > 3 || argc == 2) {
    int k;
    //Either no argument ot two arguments
    fprintf(stderr, "Usage %s [N] [path], where N is in range 0 - %d\n", argv[0], size);
    for (k=1; k<argc; ++k) {
      fprintf(stderr, "Argument #%d:\t'%s'\n", k, argv[k]);
    }

    return 1;
  }

  if ( argc == 3 ) {
    char *p;
    n = strtol(argv[1], &p, 10);  //See also strtoull
    if ((p == argv[1]) || (*p != 0) || errno == ERANGE || (n < 0) || (n >= INT_MAX) || ( n>= size ) ) {
      fprintf(stderr, "Error when parsing %s. Generator index has to be in range in range 0 - %d. Parsed as %ld.\n", argv[1], size - 1, n);
      return 1;
    }
    path = argv[2];
  }

  int k;
  letter rule[6];
  gsl_permutation *p;
  p = gsl_permutation_alloc(6);
  gsl_permutation_init(p);
  for(k=0;k<194;++k) {
     gsl_permutation_next(p);
  };
  for(k = 0; k < 6; k++){
    rule[k] = (int) gsl_permutation_get(p, (size_t) k) % 3;
  }

  for (k=0; k<size; ++k) {
    lcg[3*k] = ulcg_CreatePow2LCGL_init(&lcg_param[k]);
    lcg[3*k+1] = ulcg_CreatePow2LCGL_init(&lcg_param[k]);
    lcg[3*k+2] = ulcg_CreatePow2LCGL_init(&lcg_param[k]);
    snprintf(name, sizeof(name), "AR_%d%d%d%d%d%d with %s, %s, %s",  
        rule[0], rule[1], rule[2], rule[3], rule[4], rule[5], lcg_names[k], lcg_names[k], lcg_names[k]);
    fprintf(stderr, "%s\n", name);

    A[k] = create_AR(rule, sizeof(rule)/sizeof(*rule));
    g[k] = CreateCombGenAPRNG( A[k], &lcg[3*k], 3, name, "");
  }
  gsl_permutation_free (p);


  for (i=0;i<size;++i) {
    printf("==============%s==============\n",lcg_names[i]);
    unif01_WriteNameGen (lcg[i]);
    unif01_TimerGenWr (lcg[i], TEST, FALSE);
    printf("==============%s==============\n",lcg_names[i]);
  }


  if (argc == 1) {
   
    for (i=0;i<size;++i) {
      unif01_WriteNameGen (g[i]);
      unif01_TimerGenWr (g[i], TEST, FALSE);
    }

    for (i=0;i<size;++i) {
      //unif01_WriteNameGen (g[i]);
      //bbattery_SmallCrush (g[i]);
      //unif01_WriteNameGen (g[i]);
      //bbattery_Crush (g[i]);
      unif01_WriteNameGen (g[i]);
      bbattery_BigCrush (g[i]);
    }

  } else {
    run_fd3(g[n], path, TEST);
  }

  return 0;
}

