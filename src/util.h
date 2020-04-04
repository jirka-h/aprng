#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>

void* safe_malloc(size_t size, unsigned long line);
#define SAFEMALLOC(size) safe_malloc(size, __LINE__)

void* safe_calloc(size_t nmemb, size_t size, unsigned long line);
#define SAFECALLOC(nmemb, size) safe_calloc(nmemb, size, __LINE__)

void* safe_free (void *p);
#endif
