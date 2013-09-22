/* vim: set expandtab cindent fdm=marker ts=2 sw=2: */

#ifndef _COMB_GEN_APRNG_H
#define _COMB_GEN_APRNG_H


#include <TestU01.h>
#include "aprng.h"

typedef struct {
  size_t size;
  unif01_Gen** g;
  APRNG_t *A;
} unif01_Comb_APRNG_t;

typedef struct {
  size_t size;
  unif01_Gen** g;
  APRNG_t *A;
  unsigned int fill_frequency;
  unsigned int n_after_fill;
} unif01_Comb_APRNG_fill_t;

unsigned long GetBitsCombGenAPRNG (void *vpar, void *junk);
double GetU01CombGenAPRNG (void *vpar, void *junk);
void WrCombGenAPRNG (void *vsta);
unif01_Gen* CreateCombGenAPRNG (APRNG_t *A, unif01_Gen **g, size_t size, char *mess, char *name);


unsigned long GetBitsCombGenAPRNGFill (void *vpar, void *junk);
double GetU01CombGenAPRNGFill (void *vpar, void *junk);
void WrCombGenAPRNGFill (void *vsta);
unif01_Gen* CreateCombGenAPRNGFill (APRNG_t *A, unif01_Gen **g, size_t size, unsigned int fill_frequency, char *mess, char *name);

#endif

