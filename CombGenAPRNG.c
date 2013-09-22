/* vim: set expandtab cindent fdm=marker ts=2 sw=2: */
#include <TestU01.h>
#include <util.h>
#include <string.h>
#include <assert.h>

#include "aprng.h"
#include "CombGenAPRNG.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
unsigned long GetBitsCombGenAPRNG (void *junk, void *vsta)
{
#if 0
  static long long c=0;
  ++c;
  if ( c % 10000000LL == 0) fprintf(stderr, "GetBitsCombGenAPRNG %lld\n", c);
#endif
  unif01_Comb_APRNG_t *g = vsta;
  unif01_Gen** gen = g->g;
  APRNG_t *A = g->A;

  letter l = get_letter_APRNG ( A );
  assert( l < g->size );
  return gen[l]->GetBits(gen[l]->param, gen[l]->state);
}
#pragma GCC diagnostic pop  

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
double GetU01CombGenAPRNG (void *junk, void *vsta)
{
  unif01_Comb_APRNG_t *g = vsta;
  unif01_Gen** gen = g->g;
  APRNG_t *A = g->A;

  letter l = get_letter_APRNG ( A );
  assert( l < g->size );
  return gen[l]->GetU01(gen[l]->param, gen[l]->state);
}
#pragma GCC diagnostic pop  

void WrCombGenAPRNG (void *vsta)
{
  size_t i;
  unif01_Comb_APRNG_t *g = vsta;
  fprintf (stdout, "%zu Combined Generators based on %s", g->size, g->A->name);
  report_status_APRNG(g->A);
  for (i=0; i<g->size; ++i) {
   unif01_WriteNameGen (g->g[i]);
   unif01_WriteState   (g->g[i]); 
  }
}


unif01_Gen* CreateCombGenAPRNG (APRNG_t *A, unif01_Gen **g, size_t size, char *mess, char *name)
{
   unif01_Gen *gen;
   unif01_Comb_APRNG_t *paramC;
   size_t len, L, i;

   gen = util_Malloc (sizeof (unif01_Gen));
   paramC = util_Malloc (sizeof (unif01_Comb_APRNG_t));
   paramC->size = size;;
   paramC->g = g;
   paramC->A = A;


   len = strlen (A->name) + strlen (name) + strlen (mess);
   for (i=0; i<size;++i) {
     len += strlen (g[i]->name);
   }
   len += 3 + size;
   gen->name = util_Calloc (len + 1, sizeof (char));
   L = strlen (mess);
   if (L > 0) {
      strncpy (gen->name, mess, len);
      if (mess[L - 1] != ':')
         strncat (gen->name, ":", 3);
      strncat (gen->name, "\n", 3);
   }
   strncat (gen->name, A->name, len);
   strncat (gen->name, "\n", 3);
   for (i=0; i<size; ++i) {
     strncat (gen->name, g[i]->name, len);
     strncat (gen->name, "\n", 3);
   }
   strncat (gen->name, name, len);

   gen->param  = NULL;
   gen->state  = paramC;
   gen->Write  = WrCombGenAPRNG;
   gen->GetU01 = GetU01CombGenAPRNG;
   gen->GetBits =GetBitsCombGenAPRNG; 
   return gen;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
unsigned long GetBitsCombGenAPRNGFill (void *junk, void *vsta)
{
  unif01_Comb_APRNG_fill_t *g = vsta;
  letter l;
  ++g->n_after_fill;
  if ( g->n_after_fill % g->fill_frequency == 0) {
    g -> n_after_fill = 0;
    l = g->size - 1;
  } else {
    l = get_letter_APRNG ( g->A );
    assert( l < g->size-1 );
  }
  return g->g[l]->GetBits(g->g[l]->param, g->g[l]->state);
}
#pragma GCC diagnostic pop  

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
double GetU01CombGenAPRNGFill (void *junk, void *vsta)
{
  unif01_Comb_APRNG_fill_t *g = vsta;
  letter l;
  ++g->n_after_fill;
  if ( g->n_after_fill % g->fill_frequency == 0) {
    g -> n_after_fill = 0;
    l = g->size - 1;
  } else {
    l = get_letter_APRNG ( g->A );
    assert( l < g->size-1 );
  }
  return g->g[l]->GetU01(g->g[l]->param, g->g[l]->state);
}
#pragma GCC diagnostic pop  

void WrCombGenAPRNGFill (void *vsta)
{
  size_t i;
  unif01_Comb_APRNG_fill_t *g = vsta;
  fprintf (stdout, "%zu Combined Generators based on %s", g->size, g->A->name);
  report_status_APRNG(g->A);
  for (i=0; i<g->size-1; ++i) {
   unif01_WriteNameGen (g->g[i]);
   unif01_WriteState   (g->g[i]); 
  }
  fprintf (stdout, "with following generator value inserted at every %u position\n", g->fill_frequency);
  i = g->size - 1;
  unif01_WriteNameGen (g->g[i]);
  unif01_WriteState   (g->g[i]); 
  fprintf (stdout, "Values generated after last fill insertion: %u\n", g->n_after_fill);
}


unif01_Gen* CreateCombGenAPRNGFill (APRNG_t *A, unif01_Gen **g, size_t size, unsigned int fill_frequency, char *mess, char *name)
{
   unif01_Gen *gen;
   unif01_Comb_APRNG_fill_t *paramC;
   size_t len, L, i;

   gen = util_Malloc (sizeof (unif01_Gen));
   paramC = util_Malloc (sizeof (unif01_Comb_APRNG_fill_t));
   paramC->size = size;;
   paramC->g = g;
   paramC->A = A;
   paramC->fill_frequency = fill_frequency;
   paramC->n_after_fill = 0;

   len = strlen (A->name) + strlen (name) + strlen (mess);
   for (i=0; i<size;++i) {
     len += strlen (g[i]->name);
   }
   len += 3 + size;
   gen->name = util_Calloc (len + 1, sizeof (char));
   L = strlen (mess);
   if (L > 0) {
      strncpy (gen->name, mess, len);
      if (mess[L - 1] != ':')
         strncat (gen->name, ":", 3);
      strncat (gen->name, "\n", 3);
   }
   strncat (gen->name, A->name, len);
   strncat (gen->name, "\n", 3);
   for (i=0; i<size; ++i) {
     strncat (gen->name, g[i]->name, len);
     strncat (gen->name, "\n", 3);
   }
   strncat (gen->name, name, len);

   gen->param  = NULL;
   gen->state  = paramC;
   gen->Write  = WrCombGenAPRNGFill;
   gen->GetU01 = GetU01CombGenAPRNGFill;
   gen->GetBits =GetBitsCombGenAPRNGFill;
   return gen;
}


