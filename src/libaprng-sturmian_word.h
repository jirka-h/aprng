/* vim: set expandtab cindent fdm=marker ts=2 sw=2: */

/* Generate Sturmian words using method described in the article by Jiri Patera:
GENERATING THE FIBONACCI CHAIN IN O ( log n ) SPACE AND O ( n ) TIME

Sturmian word is represented as a tree
*/

#ifndef _LIBAPRNG_STURMIAN_WORD_H
#define _LIBAPRNG_STURMIAN_WORD_H

#include <inttypes.h>
#include <stddef.h>
#include "libaprng-util.h"

typedef struct {
  uint64_t n;      //Number of uint8_ts generated so far
  uint8_t a;       //Current element
  uint8_t first;   //First uint8_t
  uint8_t** map;   //Translate matrix 2D array
  size_t* length;  //Array of the length of the subst., with index=0 for the first letter
  size_t map_size; //Number of elements of the length vector (length of alphabet)
  size_t  i;       //Index to use from the current element a
  stackT stack;    //Stack
#ifndef NDEBUG
  uint64_t control_sum; //Sum of all generated values
#endif
} sturm_word_t;

sturm_word_t* sturm_word_new(uint8_t first, uint8_t** const map, const size_t* const length, size_t map_size, size_t max_length);
uint64_t traverse(const uint64_t elements, uint8_t* buf, sturm_word_t* data);
void sturm_word_delete(sturm_word_t* data);
size_t sturm_word_get_current_size(const sturm_word_t* data);
size_t sturm_word_get_max_size(const sturm_word_t* data);


#endif
