#ifndef __CIPHER_MODES_INL_HPP
#define __CIPHER_MODES_INL_HPP

#include <stdint.h>

namespace des {

template<typename RealSource>
bool ECBModeSource<RealSource>::is_end() const {
  return source_.is_end();
}


template<typename RealSource>
BitVector<64> ECBModeSource<RealSource>::read_block() {
  uint8_t word_bytes[8] = { 0 };
  assert(!source_.is_end());
  source_.read(reinterpret_cast<uint8_t*>(&word_bytes), 8);
  return BitVector<64>(word_bytes);
}


template<typename RealSink>
void ECBModeSink<RealSink>::consume_block(const BitVector<64> &block) {
  sink_.write(const_cast<uint8_t*>(
      reinterpret_cast<const uint8_t*>(&block.data_)), 8);
}


};

#endif // __CIPHER_MODES_INL_HPP
