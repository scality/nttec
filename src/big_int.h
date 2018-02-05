/* -*- mode: c++ -*- */
#ifndef __NTL_BIG_INT_H__
#define __NTL_BIG_INT_H__

#include <inttypes.h>
#include <unistd.h>
#include <cstdint>

std::ostream& operator<<(std::ostream& dest, __uint128_t value);
std::ostream& operator<<(std::ostream& dest, __int128_t value);

struct uint256_t
{
  __uint128_t lo;
  __uint128_t hi;
  uint256_t(): lo(), hi() {}
  uint256_t(__uint128_t x): lo(x), hi(0) {}

  uint256_t operator*(__uint128_t x)
  {
    return lo * x;
  }
  uint256_t operator%(__uint128_t x)
  {
    return lo % x;
  }
  operator __uint128_t()
  {
      return lo;
  }
};

struct int256_t
{
  __uint128_t lo;
  __uint128_t hi;
  int256_t(): lo(), hi() {}
  int256_t(__uint128_t x): lo(x), hi(0) {}

  bool operator<(__uint128_t x)
  {
    return (lo < x);
  }
  bool operator<(int x)
  {
    return (lo < (__uint128_t)x);
  }
  bool operator!=(__uint128_t x)
  {
    return (lo != x);
  }
  bool operator!=(int x)
  {
    return (lo != (__uint128_t)x);
  }
  int256_t operator+(__uint128_t x)
  {
    return lo + x;
  }
  int256_t operator-(int256_t x)
  {
    return lo - x.lo;
  }
  int256_t operator*(int256_t x)
  {
    return lo * x.lo;
  }
  int256_t operator/(int256_t x)
  {
    return lo / x.lo;
  }
  int256_t operator/(int x)
  {
    return lo / x;
  }
  operator __uint128_t()
  {
    return lo;
  }
};

#endif
