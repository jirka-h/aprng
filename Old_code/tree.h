/* vim: set expandtab cindent fdm=marker ts=2 sw=2: */


/*
 * =====================================================================================
 *
 *       Filename:  tree.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/12/2013 02:24:21 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Jirka Hladky (JH), hladky DOT jiri AT gmail DOT com
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef _TREE_H
#define _TREE_H


#include <inttypes.h>
#include <stddef.h>

typedef uint8_t letter;


/*
 * Type: stackElementT
 * -------------------
 * This is the type of the objects entered in the stack.
 * Edit it to change the type of things to be placed in
 * the stack.
 */

//typedef char stackElementT;
typedef struct {
  letter a;         //Element
  size_t i;        //Index
} stackElementT;

/*
 * Type: stackT
 * --------------
 * This is the type for a stack, i.e., it is a type that
 * holds the information necessary to keep track of a stack.
 * It has a pointer `contents' to a dynamically-allocated
 * array (used to hold the contents of the stack), an integer
 * `maxSize' that holds the size of this array (i.e., the
 * maximum number of things that can be held in the stack),
 * and another integer `top,' which stores the array index of
 * the element at the top of the stack.
 */

typedef struct {
  stackElementT *contents;
  size_t maxSize;
  size_t top;
  size_t max_top_reached;
} stackT;



typedef struct {
  uint64_t n;   //Number of letters generated so far
  letter a;       //Current element
  letter first;   //First letter
  letter** map;   //Translate matrix 2D array
  size_t* length;  //Array of the length of the subst., with index=0 for the first letter
  size_t map_size; //Number of elements of the length vector (length of alphabet)
  size_t  i;          //Index to use from the current element a
  stackT stack;        //Stack
#ifndef NDEBUG
  uint64_t control_sum; //Sum of all generated values
#endif
} sturm_word_t;

sturm_word_t* sturm_word_new(letter first, letter** const map, const size_t* const length, size_t map_size, size_t max_length);
uint64_t traverse(uint64_t elements, letter* buf, sturm_word_t* data);
//uint64_t traverse_with_fill(uint64_t elements, letter* buf, sturm_word_t* data, letter fill, uint64_t where);
void sturm_word_delete(sturm_word_t* data);
size_t sturm_word_get_current_size(sturm_word_t* data);
size_t sturm_word_get_max_size(sturm_word_t* data);


#endif
