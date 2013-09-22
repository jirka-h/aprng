/* vim: set expandtab cindent fdm=marker ts=2 sw=2: */

/*
gcc -O3 -Wall -Wextra -c fd3.c
*/
//Link with -lrt

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <error.h>
#include <time.h>
#include <inttypes.h>
#include <unistd.h>
#include <signal.h>


#include <TestU01.h>
#include "fd3.h"

uint64_t timespecDiff(struct timespec *timeA_p, struct timespec *timeB_p)
{
  return ((timeA_p->tv_sec * 1000000000) + timeA_p->tv_nsec) -
           ((timeB_p->tv_sec * 1000000000) + timeB_p->tv_nsec);
}

void run_fd3(unif01_Gen *g, const char* path, unsigned long long TEST) {
  size_t rc;
  uint32_t* buf;
  const unsigned N = 1024;
  uint64_t i;
  uint64_t total_values_generated = 0;

  uint64_t sum = 0;
  struct timespec t[4];
  double timeElapsed[2];

  FILE *stream = fopen(path, "w");
  if ( !stream ) {
    perror("Error on fdopen");
    return;
  } 

  buf = malloc(N * sizeof(uint32_t));

  unif01_WriteNameGen (g);
  unif01_TimerGenWr (g, TEST, FALSE);
  //unif01_WriteState (g);
//Time it
#if 0
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t[0]);
  for( i = 0; i < TEST; ++i) {
      sum += g->GetBits(g->param, g->state);
  }
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t[1]);
  timeElapsed[0] = (double) timespecDiff(&t[1], &t[0]) / (double) 1E9;
  fprintf(stdout, "CPUTIME to generate %llu 32-bit values: %g seconds, sum: %" PRIu64 ", average-0.5: %Lg\n", 
		  TEST, timeElapsed[0], sum, (long double)(sum)/ ( (long double)(TEST) * 4294967296.0) - 0.5L);

  unif01_WriteState (g);
//
#endif

  signal(SIGPIPE, SIG_IGN); 
//Generate random numbers
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t[0]);
  clock_gettime(CLOCK_MONOTONIC, &t[2]);
  while(1) {
    for( i = 0; i < N; ++i) {
      buf[i] = g->GetBits(g->param, g->state);
    }
    ++total_values_generated;
    rc = fwrite(buf, sizeof(uint32_t), N, stream);
    if ( rc < N ) {
      perror("fwrite");
      fprintf(stderr, "ERROR: fwrite - bytes written %zu, bytes to write %zu\n",
		      rc * sizeof(uint32_t), N * sizeof(uint32_t));
      break;
    }
  }
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t[1]);
  clock_gettime(CLOCK_MONOTONIC, &t[3]);
  sleep(3);

  timeElapsed[0] = (double) timespecDiff(&t[1], &t[0]) / (double) 1E9;
  timeElapsed[1] = (double) timespecDiff(&t[3], &t[2]) / (double) 1E9;
  fprintf(stdout, "WALLCLOCK TIME:\t%g\n"
      "CPU TIME:\t%g\n"
      "\tto generate %" PRIu64 " GB.\n", 
		  timeElapsed[1], timeElapsed[0], total_values_generated * N / 268435456);

  unif01_WriteState (g);
  unif01_TimerGenWr (g, TEST, FALSE);


  fclose(stream);
  free(buf);
}

