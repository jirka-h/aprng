/* vim: set expandtab cindent fdm=marker ts=2 sw=2: */

/*
gcc -Wall -Wextra -Wpedantic -c libaprng-util.c
*/


#include "libaprng-util.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/* Help routines for malloc/calloc/realloc/free */

void* safe_malloc(size_t size, unsigned long line)
{
    errno = 0;
    void* p = malloc(size);
    if (!p) {
        fprintf(stderr, "[%s:%lu] Malloc failed (%zu bytes)\n\t%s\n",
                __FILE__, line, size, strerror (errno));
        exit(EXIT_FAILURE);
    }
    return p;
}

void* safe_calloc(size_t nmemb, size_t size, unsigned long line)
{
    errno = 0;
    void* p = calloc(nmemb, size);
    if (!p) {
        fprintf(stderr, "[%s:%lu] Calloc failed (%zu bytes)\n\t%s\n",
                __FILE__, line, nmemb, strerror (errno));
        exit(EXIT_FAILURE);
    }
    return p;
}

void* safe_realloc(void *ptr, size_t size, unsigned long line)
{
    errno = 0;
    void* p = realloc(ptr, size);
    if ( (!p) && (size != 0) ) {
        fprintf(stderr, "[%s:%lu] Realloc failed (%zu bytes)\n\t%s\n",
                __FILE__, line, size, strerror (errno));
        exit(EXIT_FAILURE);
    }
    return p;
}

void* safe_free (void *ptr)
{
  if (ptr == NULL)
    return NULL;
  free (ptr);
  return NULL;
}

FILE* safe_fopen (const char *pathname, const char *mode, unsigned long line)
{
    errno = 0;
    FILE* f = fopen(pathname, mode);
    if (!f) {
        fprintf(stderr, "[%s:%lu] fopen failed (%s, %s)\n\t%s\n",
                __FILE__, line, pathname, mode, strerror (errno));
        exit(EXIT_FAILURE);
    }
    return f;
}

/* Simple array to represent Sturmian words */

Array* initArray(size_t initialSize) {
  Array* a;
  a = (Array*) SAFEMALLOC(sizeof(Array));
  a->array = (uint8_t *)SAFEMALLOC(initialSize * sizeof(uint8_t));
  a->used = 0;
  a->size = initialSize;
  return a;
}

void insertArray(Array *a, const uint8_t element) {
  if (a->used == a->size) {
    a->size *= 2;
    a->array = (uint8_t *)SAFEREALLOC(a->array, a->size * sizeof(uint8_t));
  }
  a->array[a->used++] = element;
}

void insertArraytoArray(Array *a, const Array *b) {
  if ( b->used > 0 ) {
    if (a->size - a->used < b->used) {
      a->size = a->used + b->used;
      a->array = (uint8_t *)SAFEREALLOC(a->array, a->size * sizeof(uint8_t));
    }
    memcpy(&a->array[a->used], &b->array[0], b->used * sizeof(uint8_t));
    a->used += b->used;
  }
}

void freeArray(Array *a) {
  safe_free(a->array);
  a->array = NULL;
  a->used = a->size = 0;
  safe_free(a);
}

void printArray(const Array *a) {
  uint8_t m;
  
  for(m=0; m < a->used; ++m) {
    fprintf(stderr, "%d, ", a->array[m]);
  }
  fprintf(stderr, "\n");
}

void clearArray(Array *a) {
  a->used = 0;
}

extern inline void StackInit(stackT *stackP,const int maxSize);
extern inline void StackDestroy(stackT *stackP);
extern inline int StackIsEmpty(const stackT *stackP);
extern inline int StackIsFull(const stackT *stackP);
extern inline void StackPush(stackT *stackP, const stackElementT element);
extern inline stackElementT StackPop(stackT *stackP);


