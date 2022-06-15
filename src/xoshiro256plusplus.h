#ifndef XOSHIRO256PLUSPLUS_H
#define XOSHIRO256PLUSPLUS_H

#include <inttypes.h>

typedef struct {
  uint64_t s[4];                     //State
  uint64_t x;                 //SplitMix64 state
} xoshiro256plusplus_t;

void xoshiro256plusplus_init_with_SplitMix64(void* gen, uint64_t seed);
uint64_t xoshiro256plusplus_next(void* gen);
void xoshiro256plusplus_jump(void* gen);
void xoshiro256plusplus_long_jump(void* gen);

#endif
