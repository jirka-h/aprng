/* vim: set expandtab cindent fdm=marker ts=2 sw=2: */

/*
gcc -std=gnu99 -O2 -Wall -Wextra -I./ -o LCG-test LCG-test.c -lrt
gcc -O3 -Wall -Wextra -I./ -o LCG-test LCG-test.c -lrt
(time ( stdbuf -oL ./LCG-test 0 >(stdbuf -oL ./TestU01_raw_stdin_input_with_log -s) ) ) &> /dev/shm/log
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>

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
   long S;
} LCG_state;

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

#if 0
unif01_Gen * ulcg_CreatePow2LCGL_init (ulcg_CreatePow2LCGL_param_t* d ) {
  if ( d->m < 65 ) {
    return ulcg_CreatePow2LCGL((int) d->m, d->a, d->c, d->s);
  } else {
  return ulcg_CreateLCG(d->m, d->a, d->c, d->s);
  }
}
#endif

uint64_t timespecDiff(struct timespec *timeA_p, struct timespec *timeB_p)
{
  return ((timeA_p->tv_sec * 1000000000) + timeA_p->tv_nsec) -
           ((timeB_p->tv_sec * 1000000000) + timeB_p->tv_nsec);
}

void time_gen(ulcg_CreatePow2LCGL_param_t* g, unsigned long long TEST) {
  uint64_t i;
  uint64_t sum = 0;
  struct timespec t[4];
  double timeElapsed[2];

  sleep(3);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t[0]);
  clock_gettime(CLOCK_MONOTONIC, &t[2]);
  for( i = 0; i < TEST; ++i) {
      sum += lcg_64_generate_32(g);
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

void run_fd3(ulcg_CreatePow2LCGL_param_t* g, const char* path, unsigned long long TEST) {
  size_t rc;
  uint32_t* buf;
  const unsigned N = 1024;
  uint64_t i;
  uint64_t total_values_generated = 0;

  struct timespec t[4];
  double timeElapsed[2];

  FILE *stream = fopen(path, "w");
  if ( !stream ) {
    perror("Error on fdopen");
    return;
  } 

  buf = malloc(N * sizeof(uint32_t));

//Time it
  time_gen(g, TEST);
  signal(SIGPIPE, SIG_IGN); 
//Generate random numbers
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t[0]);
  clock_gettime(CLOCK_MONOTONIC, &t[2]);
  while(1) {
    for( i = 0; i < N; ++i) {
      buf[i] = lcg_64_generate_32(g);
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

  timeElapsed[0] = (double) timespecDiff(&t[1], &t[0]) / (double) 1E9;
  timeElapsed[1] = (double) timespecDiff(&t[3], &t[2]) / (double) 1E9;
  fprintf(stdout, "WALLCLOCK TIME:\t%g\n"
      "CPU TIME:\t%g\n"
      "\tto generate %" PRIu64 " GB.\n", 
		  timeElapsed[1], timeElapsed[0], total_values_generated * N / 268435456);


  fclose(stream);
  free(buf);
}


int main(int argc, char **argv)
{
  const uint64_t TEST = (uint64_t) 1.0E9;
  char* path=NULL;
  long int n=-1;

  ulcg_CreatePow2LCGL_param_t lcg_param[] = {
    {140737488355213ULL, 71971110957370ULL, 0ULL, 1ULL, ~0ULL,15},  // LCG(2^47 − 115, 71971110957370,0)
    {9223372036854775783ULL, 2307085864ULL, 0ULL, 1ULL, ~0ULL,31},  // LCG(2^63-25, 2307085864, 0)
    {59, 302875106592253ULL, 0ULL, 1ULL, ~0ULL,0},
    {63, 19073486328125ULL, 1ULL, 0ULL, ~0ULL,0},
    {63, 9219741426499971445ULL, 1ULL, 0ULL, ~0ULL,0},
    {64, 2862933555777941757ULL, 1ULL, 0ULL, ~0ULL,0},
    {64, 3202034522624059733ULL, 1ULL, 0ULL, ~0ULL,0},
    {64, 3935559000370003845ULL, 1ULL, 0ULL, ~0ULL,0},
  };


  static const char* lcg_names[] = { 
    "LCG(2^47 − 115, 71971110957370,0)",
    "LCG(2^63-25, 2307085864, 0)",
    "LCG(2^59, 13^13,0)",
    "LCG(2^63, 5^19,1)",
    "LCG(2^63, 9219741426499971445,1)",
    "LCG(2^64, 2862933555777941757,1)",
    "LCG(2^64, 3202034522624059733,1)",
    "LCG(2^64, 3935559000370003845,1)",
  };
  const int size = SIZEOF(lcg_param);
  assert(size==SIZEOF(lcg_names));
  int i;
  
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

  for (i=0; i<size; ++i) {
    if ( lcg_param[i].m < 65 ) {
      lcg_param[i].mask = lcg_param[i].mask >> (64-lcg_param[i].m) ;
      lcg_param[i].shift = lcg_param[i].m - 32;
    }
  }


  if (argc == 1) {
    //time all
    for (i=0; i<size; ++i) {
      printf("Generator %s\n", lcg_names[i]);
      time_gen(&lcg_param[i], TEST);
    }

  } else {
    printf("Generator %s\n", lcg_names[n]);
    run_fd3(&lcg_param[n], path, TEST);
  }


  return 0;
}
