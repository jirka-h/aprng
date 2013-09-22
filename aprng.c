/* vim: set expandtab cindent fdm=marker ts=2 sw=2: */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//Usefull utils from TestU01
#include <util.h>
#include <addstr.h>


#include <inttypes.h>
#include "aprng.h"

#define MAX_LENGTH 4096

Array* initArray(size_t initialSize) {
  Array* a;
  a = (Array*) util_Malloc(sizeof(Array));
  a->array = (letter *)util_Malloc(initialSize * sizeof(letter));
  a->used = 0;
  a->size = initialSize;
  return a;
}

void insertArray(Array *a, letter element) {
  if (a->used == a->size) {
    a->size *= 2;
    a->array = (letter *)util_Realloc(a->array, a->size * sizeof(letter));
  }
  a->array[a->used++] = element;
}

void insertArraytoArray(Array *a, Array *b) {
  if ( b->used > 0 ) {
    if (a->size - a->used < b->used) {
      a->size = a->used + b->used;
      a->array = (letter *)util_Realloc(a->array, a->size * sizeof(letter));
    }
    memcpy(&a->array[a->used], &b->array[0], b->used * sizeof(letter));
    a->used += b->used;
  }
}

void util_FreeArray(Array *a) {
  util_Free(a->array);
  a->array = NULL;
  a->used = a->size = 0;
  util_Free(a);
}

void printArray(Array *a) {
  letter m;
  
  for(m=0; m < a->used; ++m) {
    fprintf(stderr, "%d, ", a->array[m]);
  }
  fprintf(stderr, "\n");
}

void clearArray(Array *a) {
  a->used = 0;
}

void init_buf ( buf_t* buf, size_t size ) {
  buf->data	= (letter*) util_Malloc ( size * sizeof(letter) );
  buf->total_size = size;
  buf->valid_data_size = 0;
  buf->data_start = buf->data;
}

void destroy_buf ( buf_t* buf )
{
  util_Free (buf->data);
  buf->data	= NULL;
  buf->data_start = NULL;
  buf->total_size = 0;
  buf->valid_data_size = 0;
}	

APRNG_t* create_AR ( letter* rule, size_t rule_size) {

  unsigned int i;
  int histogram[3]={0};
  for (i=0; i<rule_size; ++i) {
    //fprintf(stderr, "rule[%u] = %u\n", i, rule[i]);
    if ( rule[i]>2 ) {
      fprintf(stderr, "ERROR: create_AR: rule vector has to have values in range 0-2\n");
      fprintf(stderr, "rule[%u] = %u\n", i, rule[i]);
      exit(1);
    }
    ++histogram[i];
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

  letter l, m;
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


  APRNG_t* APRNG = (APRNG_t*) util_Malloc( sizeof(APRNG_t) );
  size_t subst_length[3];
  letter* subst[3];

  for(l=0;l<3;++l) {
    subst_length[l] = gen_rules[l]->used;
    subst[l] = util_Malloc(gen_rules[l]->used * sizeof(letter) );
    for (m=0;m<gen_rules[l]->used;++m) {
      subst[l][m] = gen_rules[l]->array[m];
    }
  }

  APRNG->sturm_word = sturm_word_new(0, subst, subst_length, sizeof(subst_length)/sizeof(*subst_length), MAX_LENGTH);

  char name[1024];
  strcpy (name, "Arnoux-Rauzy:");
  for (i=0; i<rule_size; ++i) {
    addstr_Int (name, " s", rule[i]);
  }
  
  for (l=0; l<3; ++l) {
    addstr_Char(name, "", '\n');
    addstr_Uint(name, "", l);
    addstr_Char(name, " ->", ' ');
    for(m=0; m < gen_rules[l]->used; ++m) {
      addstr_Uint(name, "", gen_rules[l]->array[m]);
    }
  }

  for(l=0;l<3;++l) {
    util_Free(subst[l]);
  }

  util_FreeArray(temp[0]);
  util_FreeArray(temp[1]);
  for(l=0; l<3; ++l) {
    util_FreeArray(gen_rules[l]);
  }
  for (i=0; i<9;++i) {
    util_FreeArray(sigma_rule[i]);
  }


  size_t len = strlen (name);
  APRNG->name = util_Calloc (len + 1, sizeof (char));
  strncpy (APRNG->name, name, len);
  APRNG->generated = 0;
  init_buf (&APRNG->buf, 16384);
  return APRNG;
}

APRNG_t* create_Fibonacci () {
  APRNG_t* APRNG = (APRNG_t*) util_Malloc( sizeof(APRNG_t) );
#if 1
  size_t subst_length[] = {2, 1};
  letter subst[][2] = { {0,1}, 
                        {0,127}};
#else
  size_t subst_length[] = {13, 8};
  letter subst[][13] = { {0,1,0,0,1,0,1,0,0,1,0,0,1}, 
    {0,1,0,0,1,0,1,0,127,127,127,127,127} };
#endif

  letter *map[sizeof(subst_length)/sizeof(letter)];  //we need to map 2D array to array of arrays
  uint8_t s;
  for(s=0; s<sizeof(subst_length)/sizeof(letter); ++s) {
    map[s] = subst[s];
  }
  APRNG->sturm_word = sturm_word_new(0, map, subst_length, sizeof(subst_length)/sizeof(*subst_length), MAX_LENGTH );
  char name[60];
  strcpy (name, "Fibonacci 0 -> 01, 1 -> 0");
  //addstr_Int (name, "   s = ", s);   
  size_t len = strlen (name);
  APRNG->name = util_Calloc (len + 1, sizeof (char));
  strncpy (APRNG->name, name, len);
  APRNG->generated = 0;
  init_buf (&APRNG->buf, 16384);

  return APRNG;
}

APRNG_t* create_Tribonacci () {
  APRNG_t* APRNG = (APRNG_t*) util_Malloc( sizeof(APRNG_t) );
  // 0 -> 01
  // 1 -> 02
  // 2 -> 0
  size_t subst_length[] = {2, 2, 1};
  letter subst[][2] = { {0,1}, 
                        {0,2},
                        {0,127} };


  letter *map[sizeof(subst_length)/sizeof(letter)];  //we need to map 2D array to array of arrays
  uint8_t s;
  for(s=0; s<sizeof(subst_length)/sizeof(letter); ++s) {
    map[s] = subst[s];
  }
  APRNG->sturm_word = sturm_word_new(0, map, subst_length, sizeof(subst_length)/sizeof(*subst_length), MAX_LENGTH );
  char name[60];
  strcpy (name, "Tribonacci 0 -> 01, 1 -> 02, 2 -> 0");
  //addstr_Int (name, "   s = ", s);   
  size_t len = strlen (name);
  APRNG->name = util_Calloc (len + 1, sizeof (char));
  strncpy (APRNG->name, name, len);
  APRNG->generated = 0;
  init_buf (&APRNG->buf, 16384);
  return APRNG;
}

APRNG_t* create_Dummy () {
  APRNG_t* APRNG = (APRNG_t*) util_Malloc( sizeof(APRNG_t) );
  // 0 -> 00
  // 1 -> 11
#if 1
  size_t subst_length[] = {2, 2};
  letter subst[][2] = { {0,0}, 
                        {1,1} };

#else
  size_t subst_length[] = {40, 40};
  letter subst[][40] = { {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 
                         {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1} };
#endif

  letter *map[sizeof(subst_length)/sizeof(letter)];  //we need to map 2D array to array of arrays
  uint8_t s;
  for(s=0; s<sizeof(subst_length)/sizeof(letter); ++s) {
    map[s] = subst[s];
  }
  APRNG->sturm_word = sturm_word_new(0, map, subst_length, sizeof(subst_length)/sizeof(*subst_length), MAX_LENGTH );
  char name[60];
  strcpy (name, "Dummy 0 -> 00, 1 -> 11");
  //addstr_Int (name, "   s = ", s);   
  size_t len = strlen (name);
  APRNG->name = util_Calloc (len + 1, sizeof (char));
  strncpy (APRNG->name, name, len);
  APRNG->generated = 0;
  init_buf (&APRNG->buf, 16384);
  return APRNG;
}



void report_status_APRNG(APRNG_t* APRNG) {
  fprintf(stdout, "\n=======================================================================================\n");
#ifndef NDEBUG
  fprintf(stdout, "%s, letters generated %" PRIu64 ", control sum %" PRIu64 "\n", APRNG->name, APRNG->generated, APRNG->sturm_word->control_sum);
#else
  fprintf(stdout, "%s, letters generated %" PRIu64 "\n", APRNG->name, APRNG->generated);
#endif
  fprintf(stdout, "Current elements on stack to hold APRNG status: %zu\n", APRNG->sturm_word->stack.top);
  fprintf(stdout, "Maximum elements on stack to hold APRNG status: %zu\n", APRNG->sturm_word->stack.max_top_reached);
  fprintf(stdout, "=======================================================================================\n");
}

void delete_APRNG (APRNG_t* APRNG) {
  util_Free(APRNG->name);
  destroy_buf(&APRNG->buf);
  sturm_word_delete(APRNG->sturm_word);
  util_Free(APRNG);
}

