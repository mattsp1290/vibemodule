// Copyright 2012 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// Ported to vibemodule - platform-agnostic version

#ifndef STMLIB_STMLIB_H_
#define STMLIB_STMLIB_H_

#include <cstdint>
#include <cstddef>

#ifndef NULL
#define NULL 0
#endif

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;      \
  TypeName& operator=(const TypeName&) = delete

#define CLIP(x) if (x < -32767) x = -32767; if (x > 32767) x = 32767;

#define CONSTRAIN(var, min, max) \
  if (var < (min)) { \
    var = (min); \
  } else if (var > (max)) { \
    var = (max); \
  }

#define JOIN(lhs, rhs)    JOIN_1(lhs, rhs)
#define JOIN_1(lhs, rhs)  JOIN_2(lhs, rhs)
#define JOIN_2(lhs, rhs)  lhs##rhs

#define STATIC_ASSERT(expression, message) \
  struct JOIN(__static_assertion_at_line_, __LINE__) \
  { \
    static_assert(expression, #message); \
  };

// Platform-agnostic: no RAM section attributes needed
#define IN_RAM

// Loop unrolling macros for performance
#define UNROLL2(x) x; x;
#define UNROLL4(x) x; x; x; x;
#define UNROLL8(x) x; x; x; x; x; x; x; x;

namespace stmlib {

struct Word {
  uint16_t value;
  uint8_t bytes[2];
};

struct LongWord {
  uint32_t value;
  uint16_t words[2];
  uint8_t bytes[4];
};

template<uint8_t a, uint8_t b, uint8_t c, uint8_t d>
struct FourCC {
  static const uint32_t value = (((uint32_t)a) << 24) |
                                (((uint32_t)b) << 16) |
                                (((uint32_t)c) << 8) |
                                (uint32_t)d;
};

}  // namespace stmlib

#endif  // STMLIB_STMLIB_H_
