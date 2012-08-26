#ifndef __CIPHER_MODES_INL_HPP
#define __CIPHER_MODES_INL_HPP

#include <stdint.h>

namespace des {

bool ECBModeSource::is_eof() const {
  return feof(fptr_);
}


BitVector<64> ECBModeSource::read_block() {
  uint8_t word_bytes[8];
  memset(word_bytes, 0, 8);
  assert(!feof(fptr_));
  fread(&word_bytes, 1, 8, fptr_);

  BitVector<64> r(word_bytes);
  std::cout << "Reading block " << r.to_string() << std::endl;
  return r;
}


void ECBModeSink::consume_block(const BitVector<64> &block) {
  int result = fwrite(block.data_, 1, 8, fptr_);
  std::cout << sizeof(block.data_) << std::endl;
  assert(sizeof(block.data_) == 8);
  if (result != 8) {
    throw std::runtime_error("error: failed (short) write");
  }
  std::cout << "Writing block " << block.to_string() << std::endl;
}


};

#endif // __CIPHER_MODES_INL_HPP
