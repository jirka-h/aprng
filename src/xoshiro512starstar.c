/*  Written in 2018 by David Blackman and Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

See <http://creativecommons.org/publicdomain/zero/1.0/>. */

#include <stdint.h>
#include <string.h>

/* This is xoshiro512** 1.0, one of our all-purpose, rock-solid generators
   with increased state size. It has excellent (about 1ns) speed, a state
   (512 bits) that is large enough for any parallel application, and it
   passes all tests we are aware of.

   For generating just floating-point numbers, xoshiro512+ is even faster.

   The state must be seeded so that it is not everywhere zero. If you have
   a 64-bit seed, we suggest to seed a splitmix64 generator and use its
   output to fill s. */

static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}


// xoshiro512** state vector
static uint64_t s[8];

// SplitMix64 state
static uint64_t x; /* The state can be seeded with any value. */

uint64_t SplitMix64_next() {
	uint64_t z = (x += 0x9e3779b97f4a7c15);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
	z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
	return z ^ (z >> 31);
}

void init_with_SplitMix64(uint64_t seed) {
  x = seed;
  for (int i=0; i<8; ++i) {
    s[i] = SplitMix64_next();
  }
}

uint64_t next(void) {
	const uint64_t result = rotl(s[1] * 5, 7) * 9;

	const uint64_t t = s[1] << 11;

	s[2] ^= s[0];
	s[5] ^= s[1];
	s[1] ^= s[2];
	s[7] ^= s[3];
	s[3] ^= s[4];
	s[4] ^= s[5];
	s[0] ^= s[6];
	s[6] ^= s[7];

	s[6] ^= t;

	s[7] = rotl(s[7], 21);

	return result;
}


/* This is the jump function for the generator. It is equivalent
   to 2^256 calls to next(); it can be used to generate 2^256
   non-overlapping subsequences for parallel computations. */

void jump(void) {
	static const uint64_t JUMP[] = { 0x33ed89b6e7a353f9, 0x760083d7955323be, 0x2837f2fbb5f22fae, 0x4b8c5674d309511c, 0xb11ac47a7ba28c25, 0xf1be7667092bcc1c, 0x53851efdb6df0aaf, 0x1ebbc8b23eaf25db };

	uint64_t t[sizeof s / sizeof *s];
	memset(t, 0, sizeof t);
	for(int i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
		for(int b = 0; b < 64; b++) {
			if (JUMP[i] & UINT64_C(1) << b)
				for(int w = 0; w < sizeof s / sizeof *s; w++)
					t[w] ^= s[w];
			next();
		}
	
	memcpy(s, t, sizeof s);	
}
	
	
/* This is the long-jump function for the generator. It is equivalent to
   2^384 calls to next(); it can be used to generate 2^128 starting points,
   from each of which jump() will generate 2^128 non-overlapping
   subsequences for parallel distributed computations. */

void long_jump(void) {
	static const uint64_t LONG_JUMP[] = { 0x11467fef8f921d28, 0xa2a819f2e79c8ea8, 0xa8299fc284b3959a, 0xb4d347340ca63ee1, 0x1cb0940bedbff6ce, 0xd956c5c4fa1f8e17, 0x915e38fd4eda93bc, 0x5b3ccdfa5d7daca5 };

	uint64_t t[sizeof s / sizeof *s];
	memset(t, 0, sizeof t);
	for(int i = 0; i < sizeof LONG_JUMP / sizeof *LONG_JUMP; i++)
		for(int b = 0; b < 64; b++) {
			if (LONG_JUMP[i] & UINT64_C(1) << b)
				for(int w = 0; w < sizeof s / sizeof *s; w++)
					t[w] ^= s[w];
			next();
		}
	
	memcpy(s, t, sizeof s);	
}
