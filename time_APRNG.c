/* vim: set expandtab cindent fdm=marker ts=2 sw=2: */

/*
gcc -O3 -Wall -Wextra -I./ -o time_APRNG time_APRNG.c aprng.c tree.c -lmylib -lrt

gcc -O3 -DNDEBUG -DCOMPILE_WITH_APRNG -Wall -Wextra -I./ -o time_APRNG_with time_APRNG.c aprng.c tree.c -lmylib -lrt
gcc -O3 -DNDEBUG -Wall -Wextra -I./ -o time_APRNG_ohne time_APRNG.c aprng.c tree.c -lmylib -lrt
*/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <inttypes.h>
#include "aprng.h"

#define SIZEOF(a) ( sizeof (a) / sizeof (a[0]) )

typedef struct {
uint64_t a;
uint64_t c;
uint64_t s;
} e64_LCG_t;

uint32_t e64_LCG_gen(e64_LCG_t* d) {
  d->s = d->a * d->s + d->c;
  return (uint32_t) ( (d->s>>32) & 0xFFFFFFFF ) ;
} 

typedef struct {
  e64_LCG_t* g;
  uint32_t buf[1000];
  uint16_t pointer;
  uint16_t size;
} e64_LCG_buf_t;

uint32_t e64_LCG_buf_get(e64_LCG_buf_t* d) {
  uint16_t i;

  if ( d->pointer < d->size ) {
    return d->buf[d->pointer++];
  } else {
    for (i=0;i<d->size;++i) {
      d->buf[i] = e64_LCG_gen(d->g);
    }
    d->pointer = 1;
    return d->buf[0];
  }
} 


uint64_t timespecDiff(struct timespec *timeA_p, struct timespec *timeB_p)
{
  return ((timeA_p->tv_sec * 1000000000) + timeA_p->tv_nsec) -
           ((timeB_p->tv_sec * 1000000000) + timeB_p->tv_nsec);
}

int main(int argc, char **argv)
{
  const unsigned int size = 4;
  APRNG_t* A[size];
  static const char* A_names[] = { "Fibonacci", "Tribonacci", "AR1", "AR2" };
  letter rule[6] = { 2, 1, 0, 1, 2, 0};

  e64_LCG_t lcg_param[] = {
    {2862933555777941757ULL, 1ULL, 0ULL},
    {3202034522624059733ULL, 1ULL, 0ULL},
    {3202034522624059733ULL, 1ULL, 0ULL},
  };

  e64_LCG_buf_t lcg_buf[SIZEOF(lcg_param)];
  lcg_buf[0].g = &lcg_param[0];
  lcg_buf[1].g = &lcg_param[1];
  lcg_buf[2].g = &lcg_param[2];
  lcg_buf[0].size = SIZEOF(lcg_buf[0].buf);
  lcg_buf[1].size = SIZEOF(lcg_buf[1].buf);
  lcg_buf[2].size = SIZEOF(lcg_buf[1].buf);

  static const char* lcg_names[] = { "64_2862933555777941757", "64_3202034522624059733", "64_3935559000370003845" };

  uint64_t i, j, k;
  int kk;
  const unsigned int buf_size = 1000;
  letter l[buf_size];
  struct timespec t[4];
  double timeElapsed[2];
  uint32_t d;
  double sum; 


  A[0] = create_Fibonacci();
  A[1] = create_Tribonacci();
  A[2] = create_AR(rule, 3);
  A[3] = create_AR(rule, 6);

  for (kk=1; kk<argc; ++kk) {
    k = atoi(argv[kk]);
    sum = 0.0;

    lcg_param[0].s = 0;
    lcg_param[0].s = 0;
    lcg_param[0].s = 0;

    lcg_buf[0].pointer = lcg_buf[0].size;
    lcg_buf[1].pointer = lcg_buf[1].size;
    lcg_buf[2].pointer = lcg_buf[2].size;


#ifndef COMPILE_WITH_APRNG
#if 0
    get_word_APRNG (A[k], buf_size, l);
#else
    for (i=0; i<buf_size; ++i) {
      l[i] = i % 3;
    }
#endif
#endif

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t[0]);
    clock_gettime(CLOCK_MONOTONIC, &t[2]);
    for (i=0; i<1.0e7; ++i) {
#ifdef COMPILE_WITH_APRNG
      get_word_APRNG (A[k], buf_size, l);
#endif
      for (j=0; j<buf_size; ++j) {
        d = e64_LCG_gen(&lcg_param[l[j]]);
//        d = e64_LCG_buf_get(&lcg_buf[l[j]]);
//        sum += d/4294967296.0;
//        d = e64_LCG_gen(&lcg_param[2]);
//        sum += d/4294967296.0;
      }
    }
    sum = e64_LCG_gen(&lcg_param[l[0]]);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t[1]);
    clock_gettime(CLOCK_MONOTONIC, &t[3]);
    timeElapsed[0] = (double) timespecDiff(&t[1], &t[0]) / (double) 1E9;
    timeElapsed[1] = (double) timespecDiff(&t[3], &t[2]) / (double) 1E9;
    fprintf(stdout, "%s\nWALLCLOCK TIME:\t%g\n"
        "CPU TIME:\t%g\n"
        "\tto generate %" PRIu64 ". Last value %" PRIu32 ", average %g\n", 
        A_names[k], timeElapsed[1], timeElapsed[0], i*buf_size, d, sum);
  }
  return 0;

}

