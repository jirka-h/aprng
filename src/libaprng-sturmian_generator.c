/* vim: set expandtab cindent fdm=marker ts=2 sw=2: */

/*
gcc -Wall -Wextra -Wpedantic -c libaprng-sturmian_generator.c libaprng-sturmian_word.c libaprng-util.c
*/

/*
This is the implementation of Sturmian Words generators for
Fibonacci - https://en.wikipedia.org/wiki/Fibonacci_word https://oeis.org/A003849
Tribonacci - https://oeis.org/A080843
Arnoux-Rauzy - 
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <inttypes.h>
#include "libaprng-sturmian_generator.h"
#include "libaprng-util.h"

#define MAX_LENGTH 4096


void init_buf ( buf_t* buf,const size_t size ) {
  buf->data	= (uint8_t*) SAFEMALLOC ( size * sizeof(uint8_t) );
  buf->total_size = size;
  buf->valid_data_size = 0;
  buf->data_start = buf->data;
}

void destroy_buf ( buf_t* buf )
{
  safe_free (buf->data);
  buf->data	= NULL;
  buf->data_start = NULL;
  buf->total_size = 0;
  buf->valid_data_size = 0;
}	

sturm_gen_t* create_AR (const uint8_t* rule, const size_t rule_size) {

  unsigned int i;
  int histogram[3]={0};
  for (i=0; i<rule_size; ++i) {
    //fprintf(stderr, "rule[%u] = %u\n", i, rule[i]);
    if ( rule[i] > 2 ) {

      fprintf(stderr, "ERROR: create_AR: rule vector has to have values in range 0-2\n");
      fprintf(stderr, "rule[%u] = %u\n", i, rule[i]);
      //for (i=0; i<rule_size; ++i) {
      //  fprintf(stderr, "rule[%u] = %u\n", i, rule[i]);
      //}
      rule[i]>2 ? fprintf(stderr,"Compiler bug?\n") : fprintf(stderr,"Compiler still thinks that rule[%u]==%u is GREATER THAN 2\n", i, rule[i]);
      return(NULL);
    }
    ++histogram[rule[i]];
  }

  for (i=0; i<3; ++i) {
    if (histogram[i]==0) {
      fprintf(stderr, "ERROR: create_AR: rule vector has to have each value at least once!\n");
    }
  }

  //Sigma rules
  Array* sigma_rule[9];
  for (i=0; i<9;++i) {
    sigma_rule[i] = initArray(2);
  }
  //sigma_0: 0->0  1->10 2->20
  insertArray(sigma_rule[0],0);                                 //0->0
  insertArray(sigma_rule[1],1); insertArray(sigma_rule[1],0);   //1->10
  insertArray(sigma_rule[2],2); insertArray(sigma_rule[2],0);   //2->20

  //sigma_1: 0->01 1->1  2->21
  insertArray(sigma_rule[3],0); insertArray(sigma_rule[3],1);   //0->01
  insertArray(sigma_rule[4],1);                                 //1->1
  insertArray(sigma_rule[5],2); insertArray(sigma_rule[5],1);   //2->21

  //sigma_2: 0->02 1->12 2->2
  insertArray(sigma_rule[6],0); insertArray(sigma_rule[6],2);   //0->02
  insertArray(sigma_rule[7],1); insertArray(sigma_rule[7],2);   //1->12
  insertArray(sigma_rule[8],2);                                 //2->2 

  //Generate rule
  Array* gen_rules[3]; //final rules
  Array* temp[3];      //temp[0] => current, temp[1] => next, temp[2] => temp for the swap

  uint8_t l, m;
  int r;
  
  temp[0] = initArray(1024);
  temp[1] = initArray(1024);

  //Loop over the alphabet
  for(l=0; l<3; ++l) {

    clearArray(temp[0]);
    insertArray(temp[0], l);   //We will derive rule l -> {}

    //Loop from the back over supplied rules
    for(r = rule_size-1; r>=0; --r) {
      clearArray(temp[1]);
      
      //Loop the current word and apply rule
      //fprintf(stderr, "Iterating through: "); printArray(temp[0]);
      for(m=0;m<temp[0]->used;++m) {
        insertArraytoArray(temp[1],sigma_rule[rule[r]*3 + temp[0]->array[m]]);
        //fprintf(stderr, "rule %d applied to %d: ", rule[r]*3 + temp[0]->array[m], temp[0]->array[m]);printArray(temp[1]);
      }
      
      //We have a new word in temp[1]. swap it
      temp[2]=temp[0];
      temp[0]=temp[1];
      temp[1]=temp[2];
   }

   //Deep copy
   gen_rules[l] = initArray(temp[0]->used);
   insertArraytoArray(gen_rules[l], temp[0]);
   
  }

  //Print gen_rules
  for(l=0; l<3; ++l) {
    fprintf(stderr, "%d ->", l);
    printArray(gen_rules[l]);
    fprintf(stderr, "\n");
  }


  sturm_gen_t* sturm_gen = (sturm_gen_t*) SAFEMALLOC( sizeof(sturm_gen_t) );
  size_t subst_length[3];
  uint8_t* subst[3];

  for(l=0;l<3;++l) {
    subst_length[l] = gen_rules[l]->used;
    subst[l] = SAFEMALLOC(gen_rules[l]->used * sizeof(uint8_t) );
    for (m=0;m<gen_rules[l]->used;++m) {
      subst[l][m] = gen_rules[l]->array[m];
    }
  }

  sturm_gen->sturm_word = sturm_word_new(0, subst, subst_length, sizeof(subst_length)/sizeof(*subst_length), MAX_LENGTH);

  char name[1024];
  snprintf(name, sizeof(name), "%s", "Arnoux-Rauzy:");
  for (i=0; i<rule_size; ++i) {
    snprintf(name+strlen(name), sizeof(name)-strlen(name), " s%d", rule[i]);
  }
  
  for (l=0; l<3; ++l) {
    snprintf(name+strlen(name), sizeof(name)-strlen(name), "\n%u -> ", l);
    for(m=0; m < gen_rules[l]->used; ++m) {
      snprintf(name+strlen(name), sizeof(name)-strlen(name), "%u", gen_rules[l]->array[m]);
    }
  }

  for(l=0;l<3;++l) {
    safe_free(subst[l]);
  }

  freeArray(temp[0]);
  freeArray(temp[1]);
  for(l=0; l<3; ++l) {
    freeArray(gen_rules[l]);
  }
  for (i=0; i<9;++i) {
    freeArray(sigma_rule[i]);
  }


  size_t len = strlen (name);
  sturm_gen->name = SAFECALLOC (len + 1, sizeof (char));
  strncpy (sturm_gen->name, name, len + 1);
  sturm_gen->generated = 0;
  init_buf (&sturm_gen->buf, 16384);
  return sturm_gen;
}

sturm_gen_t* create_Fibonacci () {
  sturm_gen_t* sturm_gen = (sturm_gen_t*) SAFEMALLOC( sizeof(sturm_gen_t) );

  size_t subst_length[] = {2, 1};
  uint8_t subst[][2] = { {0,1}, 
                        {0,127}};
  uint8_t *map[sizeof(subst_length)/sizeof(uint8_t)];  //we need to map 2D array to array of arrays
  uint8_t s;
  for(s=0; s<sizeof(subst_length)/sizeof(uint8_t); ++s) {
    map[s] = subst[s];
  }
  sturm_gen->sturm_word = sturm_word_new(0, map, subst_length, sizeof(subst_length)/sizeof(*subst_length), MAX_LENGTH );
  const char *name = "Fibonacci 0 -> 01, 1 -> 0";
  size_t len = strlen (name) + 1;
  sturm_gen->name = SAFECALLOC (len, sizeof (char));
  strncpy (sturm_gen->name, name, len);
  sturm_gen->generated = 0;
  init_buf (&sturm_gen->buf, 16384);

  return sturm_gen;
}

sturm_gen_t* create_Tribonacci () {
  sturm_gen_t* sturm_gen = (sturm_gen_t*) SAFEMALLOC( sizeof(sturm_gen_t) );
  // 0 -> 01
  // 1 -> 02
  // 2 -> 0
  size_t subst_length[] = {2, 2, 1};
  uint8_t subst[][2] = { {0,1}, 
                        {0,2},
                        {0,127} };


  uint8_t *map[sizeof(subst_length)/sizeof(uint8_t)];  //we need to map 2D array to array of arrays
  uint8_t s;
  for(s=0; s<sizeof(subst_length)/sizeof(uint8_t); ++s) {
    map[s] = subst[s];
  }
  sturm_gen->sturm_word = sturm_word_new(0, map, subst_length, sizeof(subst_length)/sizeof(*subst_length), MAX_LENGTH );
  const char *name = "Tribonacci 0 -> 01, 1 -> 02, 2 -> 0";
  size_t len = strlen (name) + 1;
  sturm_gen->name = SAFECALLOC (len, sizeof (char));
  strncpy (sturm_gen->name, name, len);
  sturm_gen->generated = 0;
  init_buf (&sturm_gen->buf, 16384);
  return sturm_gen;
}

void sturm_gen_status(const sturm_gen_t* sturm_gen) {
  fprintf(stdout, "\n=======================================================================================\n");
#ifndef NDEBUG
  fprintf(stdout, "%s, uint8_ts generated %" PRIu64 ", control sum %" PRIu64 "\n", sturm_gen->name, sturm_gen->generated, sturm_gen->sturm_word->control_sum);
#else
  fprintf(stdout, "%s, uint8_ts generated %" PRIu64 "\n", sturm_gen->name, sturm_gen->generated);
#endif
  fprintf(stdout, "Current elements on stack to hold sturm_gen status: %zu\n", sturm_gen->sturm_word->stack.top);
  fprintf(stdout, "Maximum elements on stack to hold sturm_gen status: %zu\n", sturm_gen->sturm_word->stack.max_top_reached);
  fprintf(stdout, "=======================================================================================\n");
}

void sturm_gen_delete (sturm_gen_t* sturm_gen) {
  safe_free(sturm_gen->name);
  destroy_buf(&sturm_gen->buf);
  sturm_word_delete(sturm_gen->sturm_word);
  safe_free(sturm_gen);
}

extern inline uint8_t get_uint8_t_sturm_gen (sturm_gen_t* sturm_gen);
extern inline size_t sturm_gen_get_word (sturm_gen_t* sturm_gen, const size_t elements, uint8_t* word);


