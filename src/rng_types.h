#ifndef RNGTYPES_H
#define RNGTYPES_H

#include <inttypes.h>

typedef struct {
    const char *name;
    size_t size;
    void (*set) (void *state, uint64_t seed);
    uint64_t (*get) (void *state);
} rng_type;

#endif
