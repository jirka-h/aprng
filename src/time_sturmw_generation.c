/* vim: set expandtab cindent fdm=marker ts=2 sw=2: */

/*
gcc -O3 -g -Wall -Wextra -I./ -o time_tree time_tree.c aprng.c tree.c util.c -lrt

gcc -O3 -DNDEBUG -Wall -Wextra -I./ -o time_tree time_tree.c aprng.c tree.c util.c -lrt
*/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <inttypes.h>
#include "aprng.h"

//#define HISTOGRAM

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
  uint8_t rule[6] = { 2, 1, 0, 1, 2, 0};

  uint64_t i, j;
#ifdef HISTOGRAM
  uint64_t k, histo[3];
#endif
  const unsigned int buf_size = 10000;
  uint8_t l[buf_size];
  struct timespec t[4];
  double timeElapsed[2];


  A[0] = create_Fibonacci();
  A[1] = create_Tribonacci();
  A[2] = create_AR(rule, 3);
  A[3] = create_AR(rule, 6);

  for (i=0; i<size; ++i) {
#ifdef HISTOGRAM
    histo[0]=0; histo[1]=0; histo[2]=0;
#endif

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t[0]);
    clock_gettime(CLOCK_MONOTONIC, &t[2]);
 
    for (j=0; j<1.0E6; ++j) {
      get_word_APRNG (A[i], buf_size, l);
#ifdef HISTOGRAM
      for (k=0; k<buf_size; ++k) {
        ++histo[l[k]];
      }
#endif
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t[1]);
    clock_gettime(CLOCK_MONOTONIC, &t[3]);
    timeElapsed[0] = (double) timespecDiff(&t[1], &t[0]) / (double) 1E9;
    timeElapsed[1] = (double) timespecDiff(&t[3], &t[2]) / (double) 1E9;
    fprintf(stdout, "%s\nWALLCLOCK TIME:\t%g\n"
        "CPU TIME:\t%g\n"
        "\tto generate first %g letters. Last value %d\n", 
        A_names[i], timeElapsed[1], timeElapsed[0], (double) j*buf_size, l[buf_size-1]);
    fprintf(stdout, "Size of Stack %zu Bytes, Max stack size %zu Bytes\n", 
        sturm_word_get_current_size(A[i]->sturm_word), sturm_word_get_max_size(A[i]->sturm_word));
    fprintf(stdout, "Generated %Lg\n", (long double) A[i]->generated);
#ifdef HISTOGRAM
    for (k=0; k<sizeof(histo)/sizeof(*histo); ++k) {
      fprintf(stdout, "%" PRIu64 " %Lg%%\n", k, (long double) histo[k] / (long double) A[i]->generated * 100.0L);
    }
#endif
  delete_APRNG(A[i]);
  }
  
  
  return 0;
}

