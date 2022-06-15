#ifndef XOSHIRO512_H
#define XOSHIRO512_H

#include <inttypes.h>

void init_with_SplitMix64(uint64_t seed);
uint64_t next(void);
void jump(void);
void long_jump(void);

#endif
