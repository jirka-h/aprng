/* vim: set expandtab cindent fdm=marker ts=2 sw=2: */

#ifndef _LIBsturm_gen_STURMIAN_GENERATOR_H
#define _LIBsturm_gen_STURMIAN_GENERATOR_H

#include <string.h>
#include <inttypes.h>
#include "libaprng-sturmian_word.h"
#include "libaprng-util.h"

typedef struct {
  uint8_t* data;                     //Buffer to pass values
  size_t total_size;                 //Total size of buffer
  uint8_t* data_start;               //Start of valid data
  size_t valid_data_size;            //Size of valid data
} buf_t;

typedef struct {
  sturm_word_t* sturm_word;
  char* name;
  uint64_t generated;
  buf_t buf;
} sturm_gen_t;

sturm_gen_t* create_Fibonacci ();
sturm_gen_t* create_Tribonacci ();
sturm_gen_t* create_AR (const uint8_t* rule, size_t rule_size);
void sturm_gen_delete (sturm_gen_t* sturm_gen);

inline uint8_t get_uint8_t_sturm_gen (sturm_gen_t* sturm_gen) {
  uint8_t l;
  if ( sturm_gen->buf.valid_data_size) {
    l = *sturm_gen->buf.data_start;
    ++sturm_gen->buf.data_start;
    --sturm_gen->buf.valid_data_size;
  } else {
    traverse(sturm_gen->buf.total_size, sturm_gen->buf.data, sturm_gen->sturm_word);
    l = *sturm_gen->buf.data;
    sturm_gen->buf.data_start = sturm_gen->buf.data + 1;
    sturm_gen->buf.valid_data_size = sturm_gen->buf.total_size - 1;
  }

  ++sturm_gen->generated;
  return l;
}

inline size_t sturm_gen_get_word (sturm_gen_t* sturm_gen, const size_t elements, uint8_t* word) {
  if ( elements == 0 ) return 0;
  if ( sturm_gen->buf.valid_data_size >= elements) {
    memcpy(word, sturm_gen->buf.data_start, elements * sizeof(uint8_t) );
    sturm_gen->buf.data_start += elements;
    sturm_gen->buf.valid_data_size -= elements;
  } else {
    memcpy(word, sturm_gen->buf.data_start,  sturm_gen->buf.valid_data_size * sizeof(uint8_t) );
    word += sturm_gen->buf.valid_data_size;
    traverse(elements-sturm_gen->buf.valid_data_size, word, sturm_gen->sturm_word);
    sturm_gen->buf.data_start += sturm_gen->buf.valid_data_size;
    sturm_gen->buf.valid_data_size = 0;
  }
  sturm_gen->generated += elements;
  return elements;
}

void sturm_gen_status(const sturm_gen_t* sturm_gen);

#endif

