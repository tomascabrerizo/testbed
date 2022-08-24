#ifndef CORE_TYPE_H
#define CORE_TYPE_H

/* TODO: Rename this file to core_common.h or something like that */

#include <stdint.h>
#include <stdbool.h>

#define internal static

#define OFFSET_OFF(struct, x) ((unsigned long long)(&(((struct *)(0))->x)))

#define CORE_MAX(a, b) ((a) >= (b)) ? (a) : (b)
#define CORE_MIN(a, b) ((a) >= (b)) ? (a) : (b)
#define CORE_IS_POWER_OF_TWO(x) ((x) != 0 && (((x) & ((x)-1)) == 0))
#define CORE_ALING(x, a) (((x) + (a-1)) & (~((a)-1)))
static inline uint64_t core_next_power_of_two(uint64_t n) {
  uint64_t z = 1;
  while(z <= n) z <<= 1;
  return z;
}
static inline uint64_t core_prev_power_of_two(uint64_t n) {
  uint64_t z = 1;
  while(z <= n) z <<= 1;
  return z >> 1;
}

#endif /* CORE_TYPE_H */
