#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

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

void* safe_free (void *p)
{
  if (p == NULL)
    return NULL;
  free (p);
  return NULL;
}
