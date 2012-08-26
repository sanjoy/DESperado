#ifndef __DES_HPP
#define __DES_HPP

#include <cassert>
#include <cstring>
#include <string>
#include <iostream>
#include <stdexcept>

#include <stdint.h>

#include "utils.hpp"

namespace des {

template<typename RealSink> class ECBModeSink;


// A generic class used everywhere to represent a bunch of bits.
template<int Size>
class BitVector {
 public:
  BitVector() {
    for (int i = 0; i < BitSizeToWordSize(Size); i++) data_[i] = 0;
  }

  explicit BitVector(const uint8_t *data) {
    std::memcpy(&data_, data, sizeof(data_));
  }

  bool get_bit(int index) const {
    assert(index < Size && "BitVector index out of bounds!");
    assert(index >= 0 && "BitVector index out of bounds!");
    if (BitSizeToWordSize(Size) == 1) {
      return (data_[0] & (static_cast<Word>(1 << index)));
    } else {
      int word_index = index / BitsInWord;
      int bit_index = index % BitsInWord;
      return (data_[word_index] & (static_cast<Word>(1 << bit_index)));
    }
  }

  void set_bit(int index, bool value) {
    assert(index < Size && "BitVector index out of bounds!");
    assert(index >= 0 && "BitVector index out of bounds!");
    if (BitSizeToWordSize(Size) == 1) {
      if (value) {
        data_[0] |=  (static_cast<Word>(1 << index));
      } else {
        data_[0] &=  (~(static_cast<Word>(1 << index)));
      }
    } else {
      int word_index = index / BitsInWord;
      int bit_index = index % BitsInWord;
      if (value) {
        data_[word_index] |=  (static_cast<Word>(1 << bit_index));
      } else {
        data_[word_index] &=  (~(static_cast<Word>(1 << bit_index)));
      }
    }
  }

  template<typename Permutation>
  BitVector<Permutation::OutputSize> permute(Permutation& p) const {
    BitVector<Permutation::OutputSize> new_vector;
    for (int i = 0; i < Permutation::OutputSize; i++) {
      new_vector.set_bit(i, get_bit(p.source_bit(i)));
    }
    return new_vector;
  }

  struct LeftShiftPermutation {
    static const int OutputSize = Size;
    explicit LeftShiftPermutation(int amount) : amount_(amount) {}
    int source_bit(int index) { return (index + amount_) % Size; }
    int amount_;
  };

  ALWAYS_INLINE(BitVector<Size> left_shift(int amount) const) {
    // We should only need to optimize for two cases.  Moreover, since
    // Size is a compile time constant, the checks should not really
    // occur at runtime.
    amount = amount % Size;
    if (Size == (2 * BitsInWord)) {
      BitVector<Size> result;
      result.data_[0] = (data_[0] << amount) | (data_[1] >> (Size - amount));
      result.data_[1] = (data_[1] << amount) | (data_[0] >> (Size - amount));
      return result;
    } else if (BitSizeToWordSize(Size) == 1) {
      BitVector<Size> result;
      result.data_[0] =
          (data_[0] << amount) | (data_[0] >> (Size - amount - 1));
      return result;
    } else {
      LeftShiftPermutation shift_permute(amount);
      return permute(shift_permute);
    }
  }

  template<int new_size>
  BitVector<new_size> sub_vector(int begin) const {
    assert((begin + new_size) <= Size);
    BitVector<new_size> new_vector;

    if (begin % BitsInWord == 0) {
      int word_begin = begin / BitsInWord;
      int word_end = BitSizeToWordSize(begin + new_size);
      for (int i = word_begin; i < word_end; i++) {
        new_vector.data_[i - word_begin] = data_[i];
      }
    } else {
      // Slow, general case.
      for (int i = 0; i < new_size; i++) {
        new_vector.set_bit(i, get_bit(i + begin));
      }
    }
    return new_vector;
  }

  template<int OtherSize>
  BitVector<Size + OtherSize> append(BitVector<OtherSize> &other) {
    BitVector<Size + OtherSize> new_vector;
    if (Size % BitsInWord == 0 && OtherSize % BitsInWord == 0) {
      int words = static_cast<int>(Size / BitsInWord);
      for (int i = 0; i < words; i++) {
        new_vector.data_[i] = data_[i];
      }
      int other_words = static_cast<int>(OtherSize / BitsInWord);
      for (int i = 0; i < other_words; i++) {
        new_vector.data_[i + Size / BitsInWord] = other.data_[i];
      }
    } else {
      for (int i = 0; i < Size; i++) {
        new_vector.set_bit(i, get_bit(i));
      }
      for (int i = 0; i < OtherSize; i++) {
        new_vector.set_bit(i + Size, other.get_bit(i));
      }
    }

    return new_vector;
  }

  // Maps bits in groups of MapGroupFunction::ConsumeBlockSize to groups
  // of MapGroupFunction::OutputBlockSize.
  template<typename MapGroupFunction>
  BitVector<(Size * MapGroupFunction::OutputBlockSize) /
            MapGroupFunction::ConsumeBlockSize> map_grouped(
                MapGroupFunction &mapper) {
    static_assert((Size % MapGroupFunction::ConsumeBlockSize) == 0);

    const int num_groups = Size / MapGroupFunction::ConsumeBlockSize;
    BitVector<MapGroupFunction::OutputBlockSize> output_blocks[num_groups];

    for (int i = 0; i < num_groups; i++) {
      output_blocks[i] = mapper.map(
          sub_vector<MapGroupFunction::ConsumeBlockSize>(
              i * MapGroupFunction::ConsumeBlockSize), i);
    }

    return concatenate<num_groups, MapGroupFunction::OutputBlockSize>(
        output_blocks);
  }


  BitVector<Size> operator^(const BitVector<Size> &other) {
    int size = BitSizeToWordSize(Size);
    BitVector<Size> new_vector;
    for (int i = 0; i < size; i++) {
      new_vector.data_[i] = data_[i] ^ other.data_[i];
    }

    return new_vector;
  }

  std::string to_string() const {
    char buffer[Size + 1 + (Size - 1) / 8];
    int counter = 0;
    for (int i = 0; i < Size; i++) {
      if (i % 8 == 0 && i != (Size - 1) && i != 0) buffer[counter++] = ' ';
      buffer[counter++] = get_bit(i) ? '1' : '0';
    }
    buffer[counter++] = 0;
    return std::string(buffer);
  }

  static BitVector<Size> FromString(std::string str) {
    assert(str.length() == Size);
    BitVector<Size> new_vector;
    for (int i = 0; i < Size; i++) {
      new_vector.set_bit(i, str[i] == '1');
    }
    return new_vector;
  }

  static BitVector<Size> FromHexString(const std::string &str) {
    if (str.length() != Size / 4) {
      throw std::runtime_error(std::string("error: hex string too small `")
                               + str + std::string("`"));
    }

    BitVector<Size> new_vector;
    for (int i = 0; i < static_cast<int>(str.length()); i++) {
      char c = str[i];
      int value = 0;
      if (c >= '0' && c <= '9') {
        value = c - '0';
      } else if (c >= 'a' && c <= 'f') {
        value = c - 'a';
      } else if (c >= 'A' && c <= 'F') {
        value = c - 'A';
      } else {
        throw std::runtime_error(std::string("error: invalid hex string `") +
                                 str + std::string("`"));
      }

      new_vector.set_bit(i * 4,     (value & 8) != 0);
      new_vector.set_bit(i * 4 + 1, (value & 4) != 0);
      new_vector.set_bit(i * 4 + 2, (value & 2) != 0);
      new_vector.set_bit(i * 4 + 3, (value & 1) != 0);
    }

    return new_vector;
  }

 private:
  typedef unsigned int Word;
  static int ALWAYS_INLINE(BitSizeToWordSize(int size)) {
    return (size + 8 * sizeof(Word) - 1) / (8 * sizeof(Word));
  }

  static const int BitsInWord = sizeof(Word) * 8;

  Word data_[(Size + 8 * sizeof(Word) - 1) / (8 * sizeof(Word))];

  template<int list_length, int element_size>
  ALWAYS_INLINE(BitVector<list_length * element_size> concatenate(
      BitVector<element_size> *list));

  template<int size> friend class BitVector;
  template<typename RealSink> friend class ECBModeSink;
};


class SubKeys {
 public:
  inline const BitVector<48> &key_at(int index) const {
    assert(index < 16 && index >= 0);
    if (inverted_) return keys_[15 - index];
    return keys_[index];
  }

  void invert() {
    inverted_ = !inverted_;
  }

 private:
  BitVector<48> keys_[16];
  bool inverted_;
  SubKeys() : inverted_(false) {}

  friend class KeyGenerator;
};


// Create a SubKeys object from an initial 64 bit key.
class KeyGenerator {
 public:
  static SmartPointer<SubKeys> CreateSubKeys(const BitVector<64> &initial_key);
};


class DES {
 public:
  template<typename Source, typename Sink>
  static void Execute(SubKeys *keys, Source &source, Sink &sink);

 private:
  static ALWAYS_INLINE(BitVector<32> Fiestel(const BitVector<32> &,
                                             const BitVector<48> &));
};

};

#include "des-inl.hpp"

#endif // __DES_HPP
