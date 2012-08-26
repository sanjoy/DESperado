#ifndef __DES_INL_HPP
#define __DES_INL_HPP

namespace des {


template<int length, int element_size> struct ConcatHelper;

template<int element_size>
struct ConcatHelper<1, element_size> {
  static ALWAYS_INLINE(
      BitVector<element_size> concatenate(BitVector<element_size>* list)) {
    return *list;
  }
};

template<int length, int element_size>
struct ConcatHelper {
  static ALWAYS_INLINE(
      BitVector<length * element_size> concatenate(
          BitVector<element_size>* list)) {
    BitVector<element_size * (length - 1)> tail =
        ConcatHelper<length - 1, element_size>::concatenate(list + 1);
    return list->append(tail);
  };
};

template<int Size>
template<int list_length, int element_size>
BitVector<list_length * element_size> BitVector<Size>::concatenate(
    BitVector<element_size>* list) {
  return ConcatHelper<list_length, element_size>::concatenate(list);
}


class IP {
 public:
  static const int OutputSize = 64;

  int source_bit(int index) {
    static const int table[] = {
      57, 49, 41, 33, 25, 17, 9, 1,
      59, 51, 43, 35, 27, 19, 11, 3,
      61, 53, 45, 37, 29, 21, 13, 5,
      63, 55, 47, 39, 31, 23, 15, 7,
      56, 48, 40, 32, 24, 16, 8, 0,
      58, 50, 42, 34, 26, 18, 10, 2,
      60, 52, 44, 36, 28, 20, 12, 4,
      62, 54, 46, 38, 30, 22, 14, 6
    };

    return table[index];
  }
};


class IPInverse {
 public:
  static const int OutputSize = 64;

  int source_bit(int index) {
    static const int table[] = {
      39, 7, 47, 15, 55, 23, 63, 31,
      38, 6, 46, 14, 54, 22, 62, 30,
      37, 5, 45, 13, 53, 21, 61, 29,
      36, 4, 44, 12, 52, 20, 60, 28,
      35, 3, 43, 11, 51, 19, 59, 27,
      34, 2, 42, 10, 50, 18, 58, 26,
      33, 1, 41, 9, 49, 17, 57, 25,
      32, 0, 40, 8, 48, 16, 56, 24
    };

    return table[index];
  }
};


template<typename Source, typename Sink>
void DES::Execute(SubKeys *keys, Source& source, Sink& sink) {
  while (!source.is_eof()) {
    BitVector<64> input_block = source.read_block();

    IP inital_permutation;
    BitVector<64> permuted_input_block =
        input_block.permute(inital_permutation);

    BitVector<32> current_l, current_r, tmp_l;
    current_l = permuted_input_block.sub_vector<32>(0);
    current_r = permuted_input_block.sub_vector<32>(32);

    for (int i = 0; i < 16; i++) {
      tmp_l = current_l;
      current_l = current_r;
      current_r = tmp_l ^ Fiestel(current_r, keys->key_at(i));
    }

    IPInverse inverse_initial_permutation;
    BitVector<64> output_block = current_r.append(current_l).permute(
        inverse_initial_permutation);

    sink.consume_block(output_block);
  }
}


class ESelection {
 public:
  static const int OutputSize = 48;

  int source_bit(int index) {
    static const int table[] = {
      31, 0, 1, 2, 3, 4, 3, 4,
      5, 6, 7, 8, 7, 8, 9, 10,
      11, 12, 11, 12, 13, 14, 15, 16,
      15, 16, 17, 18, 19, 20, 19, 20,
      21, 22, 23, 24, 23, 24, 25, 26,
      27, 28, 27, 28, 29, 30, 31, 0
    };

    return table[index];
  }
};


class SMapper {
 public:
  static const int OutputBlockSize = 4;
  static const int ConsumeBlockSize = 6;

  BitVector<4> map(const BitVector<6> &input, int index) {
    static const int S[8][4][16]= {
      {
        { 14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7 },
        { 0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8 },
        { 4, 1, 14, 8, 13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0 },
        { 15, 12, 8, 2, 4, 9, 1, 7, 5, 11, 3, 14, 10, 0, 6, 13 }
      },

      {
        { 15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12, 0, 5, 10 },
        { 3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5 },
        { 0, 14, 7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 2, 15 },
        { 13, 8, 10, 1, 3, 15, 4, 2, 11, 6, 7, 12, 0, 5, 14, 9 }
      },

      {
        { 10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12, 7, 11, 4, 2, 8 },
        { 13, 7, 0, 9, 3, 4, 6, 10, 2, 8, 5, 14, 12, 11, 15, 1 },
        { 13, 6, 4, 9, 8, 15, 3, 0, 11, 1, 2, 12, 5, 10, 14, 7 },
        { 1, 10, 13, 0, 6, 9, 8, 7, 4, 15, 14, 3, 11, 5, 2, 12 }
      },

      {
        { 7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15 },
        { 13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 1, 10, 14, 9 },
        { 10, 6, 9, 0, 12, 11, 7, 13, 15, 1, 3, 14, 5, 2, 8, 4 },
        { 3, 15, 0, 6, 10, 1, 13, 8, 9, 4, 5, 11, 12, 7, 2, 14 }
      },

      {
        { 2, 12, 4, 1, 7, 10, 11, 6, 8, 5, 3, 15, 13, 0, 14, 9 },
        { 14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10, 3, 9, 8, 6 },
        { 4, 2, 1, 11, 10, 13, 7, 8, 15, 9, 12, 5, 6, 3, 0, 14 },
        { 11, 8, 12, 7, 1, 14, 2, 13, 6, 15, 0, 9, 10, 4, 5, 3 }
      },

      {
        { 12, 1, 10, 15, 9, 2, 6, 8, 0, 13, 3, 4, 14, 7, 5, 11 },
        { 10, 15, 4, 2, 7, 12, 9, 5, 6, 1, 12, 14, 0, 11, 3, 8 },
        { 9, 14, 15, 5, 2, 8, 12, 3, 7, 0, 4, 10, 1, 13, 11, 6 },
        { 4, 3, 2, 12, 9, 5, 15, 10, 11, 14, 1, 7, 6, 0, 8, 13 }
      },

      {
        { 4, 11, 2, 14, 15, 0, 8, 13, 3, 12, 9, 7, 5, 10, 6, 1 },
        { 13, 0, 11, 7, 4, 9, 1, 10, 14, 3, 5, 12, 2, 15, 8, 6 },
        { 1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5, 9, 2 },
        { 6, 11, 13, 8, 1, 4, 10, 7, 9, 5, 0, 15, 14, 2, 3, 12 }
      },

      {
        { 13, 2, 8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7 },
        { 1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2 },
        { 7, 11, 4, 1, 9, 12, 14, 2, 0, 6, 10, 13, 15, 3, 5, 8 },
        { 2, 1, 14, 7, 4, 10, 8, 13, 15, 12, 9, 0, 3, 5, 6, 11 }
      }
    };

    int i = input.get_bit(0) * 2 + input.get_bit(5);
    int j = input.get_bit(1) * 8 + input.get_bit(2) * 4 +
            input.get_bit(3) * 2 + input.get_bit(4);

    BitVector<4> result;
    int value = S[index][i][j];

    result.set_bit(0, (value & 8) != 0);
    result.set_bit(1, (value & 4) != 0);
    result.set_bit(2, (value & 2) != 0);
    result.set_bit(3, (value & 1) != 0);

    return result;
  }
};


class SBPermutation {
 public:
  static const int OutputSize = 32;

  int source_bit(int index) {
    static const int table[] = {
      15, 6, 19, 20, 28, 11, 27, 16,
      0, 14, 22, 25, 4, 17, 30, 9,
      1, 7, 23, 13, 31, 26, 2, 8,
      18, 12, 29, 5, 21, 10, 3, 24
    };

    return table[index];
  };
};


BitVector<32> DES::Fiestel(const BitVector<32> &input,
                           const BitVector<48> &key) {
  ESelection e_selection;
  BitVector<48> expanded_and_xored = input.permute(e_selection) ^ key;
  SMapper s_mapper;
  BitVector<32> after_sb = expanded_and_xored.map_grouped(s_mapper);
  SBPermutation sb_permutation;
  return after_sb.permute(sb_permutation);
}

};

#endif // __DES_INL_HPP
