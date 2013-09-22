/* vim: set expandtab cindent fdm=marker ts=2 sw=2: */

/*
gcc -g -Wall -Wextra -I./ -o Tribonacci_LCG_mix Tribonacci_LCG_mix.c aprng.c tree.c CombGenAPRNG.c -lmylib -ltestu01
valgrind --tool=memcheck -v ./Tribonacci_LCG_mix
gcc -g -Wall -Wextra -I./ -o Tribonacci_LCG_mix Tribonacci_LCG_mix.c aprng.c tree.c CombGenAPRNG.c   -lmylib -ltestu01 -lefence

gcc -O3 -DNDEBUG -Wall -Wextra -I./ -o Tribonacci_LCG_mix Tribonacci_LCG_mix.c aprng.c tree.c CombGenAPRNG.c -lmylib -ltestu01 -lrt -lgsl -lgslcblas
./Tribonacci_LCG_mix 0 0 >(pv >/dev/null )

valgrind --tool=cachegrind ./Tribonacci_LCG_mix
*/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "aprng.h"
#include <assert.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include<gsl/gsl_permutation.h>
#include <gsl/gsl_combination.h>

#define SIZEOF(a) ( sizeof (a) / sizeof (a[0]) )

typedef struct {
uint64_t m;
uint64_t a;
uint64_t c;
uint64_t s;
uint64_t mask;
uint8_t shift;
} ulcg_CreatePow2LCGL_param_t;

typedef struct {
  size_t size;
  ulcg_CreatePow2LCGL_param_t* lcg;
  APRNG_t *A;
  char name[1024];
} LCG_Comb_APRNG_t;

inline uint32_t lcg_64_generate_32 (ulcg_CreatePow2LCGL_param_t* d) {
  if ( d->m < 65 ) {
    d->s = ( d->a * d->s + d->c ) & d->mask;
  } else {
    unsigned __int128 mult;
    mult = ( (unsigned __int128) d->a * (unsigned __int128) d->s + d->c ) % d->m;
    //d->s = ( uint64_t ) ( mult & 0xFFFFFFFFFFFFFFFFULL);
    d->s = ( uint64_t ) ( mult );
  }
  //return (uint32_t) ( (d->s>>d->shift) & 0xFFFFFFFF ) ;
  return (uint32_t) ( (d->s>>d->shift) );
}

inline uint32_t aprng_generate (LCG_Comb_APRNG_t* g) {
  letter l = get_letter_APRNG ( g->A );
  assert( l < g->size );
  return lcg_64_generate_32(&g->lcg[l]);
}

uint64_t timespecDiff(struct timespec *timeA_p, struct timespec *timeB_p)
{
  return ((timeA_p->tv_sec * 1000000000) + timeA_p->tv_nsec) -
           ((timeB_p->tv_sec * 1000000000) + timeB_p->tv_nsec);
}

void time_gen(LCG_Comb_APRNG_t* g, unsigned long long TEST) {
  uint64_t i;
  uint64_t sum = 0;
  struct timespec t[4];
  double timeElapsed[2];

  sleep(3);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t[0]);
  clock_gettime(CLOCK_MONOTONIC, &t[2]);
  for( i = 0; i < TEST; ++i) {
      sum += aprng_generate(g);
  }
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t[1]);
  clock_gettime(CLOCK_MONOTONIC, &t[3]);
  timeElapsed[0] = (double) timespecDiff(&t[1], &t[0]) / (double) 1E9;
  timeElapsed[1] = (double) timespecDiff(&t[3], &t[2]) / (double) 1E9;
  fprintf(stdout, "WALLCLOCK TIME:\t%g\n"
      "CPU TIME:\t%g\n"
      "to generate %llu 32-bit values.\n"
      "Sum: %" PRIu64 ", average-0.5: %Lg\n", 
                  timeElapsed[1], timeElapsed[0], TEST, sum, (long double)(sum)/ ( (long double)(TEST) * 4294967296.0) - 0.5L);

}

void run_fd3(LCG_Comb_APRNG_t* g, const char* path, unsigned long long TEST, uint8_t low32_flag) {
  size_t rc;
  uint32_t* buf;
  uint32_t* low32=NULL;
  const unsigned N = low32_flag ? 1 * 1024 : 1024;
  uint16_t i, j;
  uint8_t k;
  uint32_t bit;
  uint64_t total_values_generated = 0;
  size_t write_bits;

  struct timespec t[4];
  double timeElapsed[2];

  FILE *stream = fopen(path, "w");
  if ( !stream ) {
    perror("Error on fdopen");
    return;
  } 

  if (low32_flag) {
    low32 = malloc(N / 32 * sizeof(uint32_t));
    write_bits = N/32;
  } else {
    write_bits = N;
  }

  buf = malloc(N * sizeof(uint32_t));
  fprintf(stderr,"Generator %s\n", g->name);

//Time it
  time_gen(g, TEST);
  signal(SIGPIPE, SIG_IGN); 
//Generate random numbers
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t[0]);
  clock_gettime(CLOCK_MONOTONIC, &t[2]);
  while(1) {
    for( i = 0; i < N; ++i) {
      buf[i] = aprng_generate(g);
    }
    ++total_values_generated;
    if ( low32_flag ) {
      for (j =0; j < N/32; ++j) {
        low32[j]  = 0;
        for (k=0; k<32; ++k) {
          bit = buf[j*32+k]  & INT32_C(1);
          low32[j] ^=  bit << k;
          //printf("%x\n",low32[j]); 
        }
      }
      rc = fwrite(low32, sizeof(uint32_t), N/32, stream);
    } else {
      rc = fwrite(buf, sizeof(uint32_t), N, stream);
    }
    if ( rc < write_bits ) {
      perror("fwrite");
      fprintf(stderr, "ERROR: fwrite - bytes written %zu, bytes to write %zu\n",
		      rc * sizeof(uint32_t), N * sizeof(uint32_t));
      break;
    }
  }
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t[1]);
  clock_gettime(CLOCK_MONOTONIC, &t[3]);

  timeElapsed[0] = (double) timespecDiff(&t[1], &t[0]) / (double) 1E9;
  timeElapsed[1] = (double) timespecDiff(&t[3], &t[2]) / (double) 1E9;
  fprintf(stdout, "WALLCLOCK TIME:\t%g\n"
      "CPU TIME:\t%g\n"
      "\tto generate %" PRIu64 " GB.\n", 
		  timeElapsed[1], timeElapsed[0], total_values_generated * N / 268435456);


  fclose(stream);
  free(buf);
  if (low32_flag) free(low32);
}


int main(int argc, char **argv)
{
  const uint64_t TEST = (uint64_t) 1.0E9;
  char* path=NULL;
  long int n=-1;
  long int low32_flag = -1;
  const int size = 6;
  APRNG_t* A[size];
  LCG_Comb_APRNG_t L[size];
  int i;
  gsl_combination * c;
  gsl_permutation *p;
  letter rule[3];
  int k;

  ulcg_CreatePow2LCGL_param_t lcg_param[] = {
    {9223372036854775783ULL, 2307085864ULL, 0ULL, 1ULL,~0ULL,31},  // LCG(2^63-25, 2307085864, 0)
    {59, 302875106592253ULL, 0ULL, 1ULL,~0ULL,0},
    {64, 3935559000370003845, 1ULL, 0ULL,~0ULL,0},
  };
  static const char* lcg_names[] = { 
    "LCG(2^63-25, 2307085864, 0)",
    "LCG(2^59, 13^13,0)",
    "LCG(2^64, 3935559000370003845,1)" };
  char name[1024];
  ulcg_CreatePow2LCGL_param_t lcg[SIZEOF(rule)*size];

  for (i=0; i<(int) SIZEOF(lcg_param); ++i) {
    if ( lcg_param[i].m < 65 ) {
      lcg_param[i].mask = lcg_param[i].mask >> (64-lcg_param[i].m) ;
      lcg_param[i].shift = lcg_param[i].m - 32;
    }
  }


  if ( argc != 4) {
    //Either no argument ot two arguments
    fprintf(stderr, "Usage %s [N] [low32] [path], where N is in range 0 - %d and low32 is 0(false) or 1 (true)\n", argv[0], size-1);
    for (i=1; i<argc; ++i) {
      fprintf(stderr, "Argument #%d:\t'%s'\n", i, argv[i]);
    }

    return 1;
  }

  if ( argc == 4 ) {
    char *p;
    n = strtol(argv[1], &p, 10);  //See also strtoull
    if ((p == argv[1]) || (*p != 0) || errno == ERANGE || (n < 0) || (n >= INT_MAX) || ( n>= size ) ) {
      fprintf(stderr, "Error when parsing %s. Generator index has to be in range in range 0 - %d. Parsed as %ld.\n", argv[1], size-1, n);
      return 1;
    }
    low32_flag = strtol(argv[2], &p, 10);  //See also strtoull
    if ((p == argv[2]) || (*p != 0) || errno == ERANGE || (low32_flag < 0) || (low32_flag >= INT_MAX) || ( low32_flag>= 2 ) ) {
      fprintf(stderr, "Error when parsing %s. Generator index has to be in range in range 0 - %d. Parsed as %ld.\n", argv[2], 1, low32_flag);
      return 1;
    }
    path = argv[3];
  }

  i = 0;
  c = gsl_combination_calloc (SIZEOF(lcg_param), SIZEOF(rule));
  do {
    p = gsl_permutation_calloc(SIZEOF(rule));
    do {
      for(k = 0; k < (int) SIZEOF(rule); k++){
        rule[k] = (int) gsl_combination_get (c, gsl_permutation_get(p, (size_t) k) );
      }
      lcg[SIZEOF(rule)*i] = lcg_param[rule[0]];
      lcg[SIZEOF(rule)*i+1] = lcg_param[rule[1]];
      lcg[SIZEOF(rule)*i+2] = lcg_param[rule[2]];
      snprintf(name, sizeof(name), "Tribonacci with %s, %s, %s { %d, %d, %d }", lcg_names[rule[0]], lcg_names[rule[1]], lcg_names[rule[2]], rule[0], rule[1], rule[2]);
      fprintf(stderr, "%s\n", name);

      A[i] = create_Tribonacci();
      L[i].A = A[i];
      L[i].lcg = &lcg[SIZEOF(rule)*i];
      L[i].size = SIZEOF(rule);
      strncpy(L[i].name, name, SIZEOF((L[i].name)));
      ++i;
    } while(gsl_permutation_next(p) == GSL_SUCCESS);
    gsl_permutation_free (p);
  } while (gsl_combination_next (c) == GSL_SUCCESS);
  gsl_combination_free (c);

  for (i=0;i<size;++i) {
    fprintf(stderr, "%"PRIu64", %"PRIu64", %"PRIu64"\n", L[i].lcg[0].m, L[i].lcg[1].m, L[i].lcg[2].m);
  }
  run_fd3(&L[n], path, TEST, (uint8_t)low32_flag);

  return 0;
}

