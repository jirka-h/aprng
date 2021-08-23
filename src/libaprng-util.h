/* vim: set expandtab cindent fdm=marker ts=2 sw=2: */

#ifndef _LIBAPRNG_UTIL_H
#define _LIBAPRNG_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

/* Help routines for malloc/calloc/realloc/free */

void* safe_malloc (size_t size, unsigned long line);
#define SAFEMALLOC(size) safe_malloc(size, __LINE__)

void* safe_calloc (size_t nmemb, size_t size, unsigned long line);
#define SAFECALLOC(nmemb, size) safe_calloc(nmemb, size, __LINE__)

void* safe_realloc(void *ptr, size_t size, unsigned long line);
#define SAFEREALLOC(ptr, size) safe_realloc(ptr, size, __LINE__)

FILE* safe_fopen (const char *pathname, const char *mode, unsigned long line);
#define SAFEFOPEN(pathname, mode) safe_fopen(pathname, mode, __LINE__)

void* safe_free (void *ptr);

/* Simple stack to represent the SUBSTITUTION TREES */

/*
 * Type: stackElementT
 * -------------------
 * This is the type of the objects entered in the stack.
 */

typedef struct {
  uint8_t a;       //Element
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

inline void StackInit(stackT *stackP, const int maxSize)
{
  stackElementT *newContents;

  /* Allocate a new array to hold the contents. */

  newContents = (stackElementT *)SAFEMALLOC(sizeof(stackElementT) * maxSize);

  if (newContents == NULL) {
    fprintf(stderr, "Insufficient memory to initialize stack.\n");
    exit(EXIT_FAILURE);  /* Exit, returning error code. */
  }

  stackP->contents = newContents;
  stackP->maxSize = maxSize;
  stackP->top = 0;  /* I.e., empty */
  stackP->max_top_reached = 0;
}

inline void StackDestroy(stackT *stackP)
{
  /* Get rid of array. */
  safe_free(stackP->contents);

  stackP->contents = NULL;
  stackP->maxSize = 0;
  stackP->top = 0;  /* I.e., empty */
  stackP->max_top_reached = 0;
}

inline int StackIsEmpty(const stackT *stackP)
{
  return stackP->top == 0;
}

inline int StackIsFull(const stackT *stackP)
{
  return stackP->top >= stackP->maxSize;
}

//TODO - ERROR handling other than exit
inline void StackPush(stackT *stackP, const stackElementT element)
{
  if (StackIsFull(stackP)) {
    fprintf(stderr, "Can't push element on stack: stack is full.\n");
    exit(1);  /* Exit, returning error code. */
}

  /* Put information in array; update top. */

  stackP->contents[stackP->top] = element;
  ++stackP->top;
  if (stackP->top > stackP->max_top_reached) stackP->max_top_reached = stackP->top;
}

inline stackElementT StackPop(stackT *stackP)
{
  if (StackIsEmpty(stackP)) {
    fprintf(stderr, "Can't pop element from stack: stack is empty.\n");
    exit(1);  /* Exit, returning error code. */
  }

  return stackP->contents[--stackP->top];
}


/* Simple array to represent Sturmian words */

typedef struct {
  uint8_t *array;
  size_t used;
  size_t size;
} Array;

Array* initArray(size_t initialSize);
void insertArray(Array *a, const uint8_t element);
void insertArraytoArray(Array *a, const Array *b);
void freeArray(Array *a);
void clearArray(Array *a);
void printArray(const Array *a);

#endif
