/* vim: set expandtab cindent fdm=marker ts=2 sw=2: */


#ifndef _APRNG_H
#define _APRNG_H

#include <string.h>
#include <inttypes.h>
#include "tree.h"

typedef struct {
  letter *array;
  size_t used;
  size_t size;
} Array;

Array* initArray(size_t initialSize);
void insertArray(Array *a, letter element);
void freeArray(Array *a);
void clearArray(Array *a);
void printArray(Array *a);

typedef struct {
  letter* data;                       //Buffer to pass values
  size_t total_size;                 //Total size of buffer
  letter* data_start;                 //Start of valid data
  size_t valid_data_size;            //Size of valid data
} buf_t;

void init_buf ( buf_t* data, size_t size );
void destroy_buf ( buf_t* data );

typedef struct {
  sturm_word_t* sturm_word;
  char* name;
  uint64_t generated;
  buf_t buf;
} APRNG_t;

APRNG_t* create_Fibonacci ();
APRNG_t* create_Fibonacci_with2 ();
APRNG_t* create_Tribonacci ();
APRNG_t* create_Dummy ();
APRNG_t* create_AR ( letter* rule, size_t rule_size);
void delete_APRNG (APRNG_t* APRNG);

//letter get_letter_APRNG (APRNG_t* APRNG);
#ifdef __GNUC_GNU_INLINE__
extern 
#endif
inline letter get_letter_APRNG (APRNG_t* APRNG) {
  letter l;
  if ( APRNG->buf.valid_data_size) {
    l = *APRNG->buf.data_start;
    ++APRNG->buf.data_start;
    --APRNG->buf.valid_data_size;
  } else {
    traverse(APRNG->buf.total_size, APRNG->buf.data, APRNG->sturm_word);
    l = *APRNG->buf.data;
    APRNG->buf.data_start = APRNG->buf.data + 1;
    APRNG->buf.valid_data_size = APRNG->buf.total_size - 1;
  }

  ++APRNG->generated;
  return l;
}

//size_t get_word_APRNG (APRNG_t* APRNG, size_t elements, letter* word);
#ifdef __GNUC_GNU_INLINE__
extern
#endif
inline size_t get_word_APRNG (APRNG_t* APRNG, size_t elements, letter* word) {
  if ( elements == 0 ) return 0;
  if ( APRNG->buf.valid_data_size >= elements) {
    memcpy(word, APRNG->buf.data_start, elements * sizeof(letter) );
    APRNG->buf.data_start += elements;
    APRNG->buf.valid_data_size -= elements;
  } else {
    memcpy(word, APRNG->buf.data_start,  APRNG->buf.valid_data_size * sizeof(letter) );
    word += APRNG->buf.valid_data_size;
    traverse(elements-APRNG->buf.valid_data_size, word, APRNG->sturm_word);
    APRNG->buf.data_start += APRNG->buf.valid_data_size;
    APRNG->buf.valid_data_size = 0;
  }
  APRNG->generated += elements;
  return elements;
}



void report_status_APRNG(APRNG_t* APRNG);

#endif

