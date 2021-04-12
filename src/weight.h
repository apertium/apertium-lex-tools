/*
 * Copyright (C) 2011--2012 Universitat d'Alacant
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

#ifndef __WEIGHT_H__
#define __WEIGHT_H__

#include <cstdint>
#include <cstdlib>
#include <utility>

struct weight {
  int32_t id;
  char _pad[4]{};
  double pisu;
};

// This should all be optimized out on little-endian archs

template<typename T>
inline uint64_t U64(T t) {
  return static_cast<uint64_t>(t);
}

inline void weight_to_le(weight& w) {
  uint32_t id = static_cast<uint32_t>(w.id);
  uint8_t *bytes = reinterpret_cast<uint8_t*>(&w.id);
  bytes[3] = (id >> 24) & 0xFF;
  bytes[2] = (id >> 16) & 0xFF;
  bytes[1] = (id >> 8) & 0xFF;
  bytes[0] = id & 0xFF;

  bytes = reinterpret_cast<uint8_t*>(&w.pisu);
  uint64_t pisu = *reinterpret_cast<uint64_t*>(&w.pisu);
  bytes[7] = (pisu >> 56) & 0xFF;
  bytes[6] = (pisu >> 48) & 0xFF;
  bytes[5] = (pisu >> 40) & 0xFF;
  bytes[4] = (pisu >> 32) & 0xFF;
  bytes[3] = (pisu >> 24) & 0xFF;
  bytes[2] = (pisu >> 16) & 0xFF;
  bytes[1] = (pisu >> 8) & 0xFF;
  bytes[0] = pisu & 0xFF;
}

inline void weight_from_le(weight& w) {
  uint32_t id = static_cast<uint32_t>(w.id);
  uint8_t *bytes = reinterpret_cast<uint8_t*>(&id);
  id = (bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];
  w.id = static_cast<int32_t>(id);

  bytes = reinterpret_cast<uint8_t*>(&w.pisu);
  uint64_t pisu = (U64(bytes[7]) << 56ull) | (U64(bytes[6]) << 48ull) | (U64(bytes[5]) << 40ull) | (U64(bytes[4]) << 32ull) | (U64(bytes[3]) << 24ull) | (U64(bytes[2]) << 16ull) | (U64(bytes[1]) << 8ull) | U64(bytes[0]);
  w.pisu = *reinterpret_cast<double*>(&pisu);
}

#endif /* __WEIGHT_H__ */
