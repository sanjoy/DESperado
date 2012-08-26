#ifndef __CIPHER_MODES_HPP
#define __CIPHER_MODES_HPP

#include <string>
#include <cstdio>
#include <cstring>

#include <stdint.h>

#include "des.hpp"

namespace des {

template<typename RealSource>
class ECBModeSource {
 public:
  explicit ECBModeSource(RealSource source) : source_(source) { }

  ALWAYS_INLINE(bool is_end() const);
  ALWAYS_INLINE(BitVector<64> read_block());

 private:
  RealSource source_;
};


template<typename RealSink>
class ECBModeSink {
 public:
  explicit ECBModeSink(RealSink sink) : sink_(sink) { }

  ALWAYS_INLINE(void consume_block(const BitVector<64> &block));

 private:
  RealSink sink_;
};


};

#include "cipher-modes-inl.hpp"

#endif // __CIPHER_MODES_HPP
