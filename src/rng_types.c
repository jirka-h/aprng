#include <inttypes.h>
#include <stddef.h>

#include "rng_types.h"
#include "xoshiro256plusplus.h"

static const rng_type xoshiro256plusplus_type = {
  "xoshiro256plusplus",          /* name */
  sizeof (xoshiro256plusplus_t),
  &xoshiro256plusplus_init_with_SplitMix64,
  &xoshiro256plusplus_next
};
