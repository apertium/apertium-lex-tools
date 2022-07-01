/*
 * Copyright (C) 2021 Apertium
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _LRX_BINARY_HEADER_
#define _LRX_BINARY_HEADER_

#include <cstdint>
#include <lttoolbox/endian_util.h>

// Global lttoolbox features
constexpr char HEADER_LRX[4]{'A', 'L', 'R', 'X'};
enum LRX_FEATURES : uint64_t {
  LRX_MMAP = (1ull << 0), // using mmap-compatible format rather than compressed format
  LRX_UNKNOWN = (1ull << 1), // Features >= this are unknown, so throw an error; Inc this if more features are added
  LRX_RESERVED = (1ull << 63), // If we ever reach this many feature flags, we need a flag to know how to extend beyond 64 bits
};

struct weight {
  int32_t id;
  char _pad[4]{};
  double pisu;
};

inline void weight_to_le(weight& w) {
  uint32_t id = static_cast<uint32_t>(w.id);
  to_le_32(id);

  uint64_t pisu = *reinterpret_cast<uint64_t*>(&w.pisu);
  to_le_64(pisu);
}

inline void weight_from_le(weight& w) {
  uint32_t id = static_cast<uint32_t>(w.id);
  from_le_32(id);

  uint64_t pisu = *reinterpret_cast<uint64_t*>(&w.pisu);
  from_le_64(pisu);
}

#endif
