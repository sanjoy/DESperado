#ifndef __CIPHER_MODES_HPP
#define __CIPHER_MODES_HPP

#include <string>
#include <cstdio>
#include <cstring>

#include <stdint.h>

#include "des.hpp"

namespace des {

class ECBModeSource {
 public:
  explicit ECBModeSource(std::string file_name);

  ALWAYS_INLINE(bool is_eof() const);
  ALWAYS_INLINE(BitVector<64> read_block());

     //  int file_size() const { return file_size_; }

  ~ECBModeSource();

 private:
  FILE* fptr_;
};


class ECBModeSink {
 public:
  explicit ECBModeSink(std::string file_name);

  ALWAYS_INLINE(void consume_block(const BitVector<64> &block));

  ~ECBModeSink();

 private:
  FILE* fptr_;
};

};

#include "cipher-modes-inl.hpp"

#endif // __CIPHER_MODES_HPP
