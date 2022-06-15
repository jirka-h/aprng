/*  Written in 2019 by David Blackman and Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

See <http://creativecommons.org/publicdomain/zero/1.0/>. */

#include <stdint.h>
#include "xoshiro256plusplus.h"

/* This is xoshiro256++ 1.0, one of our all-purpose, rock-solid generators.
   It has excellent (sub-ns) speed, a state (256 bits) that is large
   enough for any parallel application, and it passes all tests we are
   aware of.

   For generating just floating-point numbers, xoshiro256+ is even faster.

   The state must be seeded so that it is not everywhere zero. If you have
   a 64-bit seed, we suggest to seed a splitmix64 generator and use its
   output to fill s. */

static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

uint64_t SplitMix64_next(void* state) {
  xoshiro256plusplus_t *gen = (xoshiro256plusplus_t *) state;
	uint64_t z = (gen->x += 0x9e3779b97f4a7c15);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
	z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
	return z ^ (z >> 31);
}

void init_with_SplitMix64(void* state, uint64_t seed) {
  xoshiro256plusplus_t *gen = (xoshiro256plusplus_t *) state;
  gen->x = seed;
  for (int i=0; i<4; ++i) {
    gen->s[i] = SplitMix64_next(gen);
  }
}

uint64_t next(void* state) {
  xoshiro256plusplus_t *gen = (xoshiro256plusplus_t *) state;
	const uint64_t result = rotl(gen->s[0] + gen->s[3], 23) + gen->s[0];

	const uint64_t t = gen->s[1] << 17;

	gen->s[2] ^= gen->s[0];
	gen->s[3] ^= gen->s[1];
	gen->s[1] ^= gen->s[2];
	gen->s[0] ^= gen->s[3];

	gen->s[2] ^= t;

	gen->s[3] = rotl(gen->s[3], 45);

	return result;
}


/* This is the jump function for the generator. It is equivalent
   to 2^128 calls to next(); it can be used to generate 2^128
   non-overlapping subsequences for parallel computations. */

void jump(void* state) {
  xoshiro256plusplus_t *gen = (xoshiro256plusplus_t *) state;

	static const uint64_t JUMP[] = { 0x180ec6d33cfd0aba, 0xd5a61266f0c9392c, 0xa9582618e03fc9aa, 0x39abdc4529b1661c };

	uint64_t s0 = 0;
	uint64_t s1 = 0;
	uint64_t s2 = 0;
	uint64_t s3 = 0;
	for(int i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
		for(int b = 0; b < 64; b++) {
			if (JUMP[i] & UINT64_C(1) << b) {
				s0 ^= gen->s[0];
				s1 ^= gen->s[1];
				s2 ^= gen->s[2];
				s3 ^= gen->s[3];
			}
			next(gen);
		}

	gen->s[0] = s0;
	gen->s[1] = s1;
	gen->s[2] = s2;
	gen->s[3] = s3;
}



/* This is the long-jump function for the generator. It is equivalent to
   2^192 calls to next(); it can be used to generate 2^64 starting points,
   from each of which jump() will generate 2^64 non-overlapping
   subsequences for parallel distributed computations. */

void long_jump(void* state) {
  xoshiro256plusplus_t *gen = (xoshiro256plusplus_t *) state;
	static const uint64_t LONG_JUMP[] = { 0x76e15d3efefdcbbf, 0xc5004e441c522fb3, 0x77710069854ee241, 0x39109bb02acbe635 };

	uint64_t s0 = 0;
	uint64_t s1 = 0;
	uint64_t s2 = 0;
	uint64_t s3 = 0;
	for(int i = 0; i < sizeof LONG_JUMP / sizeof *LONG_JUMP; i++)
		for(int b = 0; b < 64; b++) {
			if (LONG_JUMP[i] & UINT64_C(1) << b) {
				s0 ^= gen->s[0];
				s1 ^= gen->s[1];
				s2 ^= gen->s[2];
				s3 ^= gen->s[3];
			}
			next(gen);
		}

	gen->s[0] = s0;
	gen->s[1] = s1;
	gen->s[2] = s2;
	gen->s[3] = s3;
}
