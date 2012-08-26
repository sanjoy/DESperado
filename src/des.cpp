#include <iostream>

#include "des.hpp"

using namespace std;
using namespace des;

struct PC1 {
  static const int OutputSize = 56;
  int source_bit(int index) {
    static const int table[] = {
      56, 48, 40, 32, 24, 16, 8,
      0, 57, 49, 41, 33, 25, 17,
      9, 1, 58, 50, 42, 34, 26,
      18, 10, 2, 59, 51, 43, 35,
      62, 54, 46, 38, 30, 22, 14,
      6, 61, 53, 45, 37, 29, 21,
      13, 5, 60, 52, 44, 36, 28,
      20, 12, 4, 27, 19, 11, 3,
    };

    return table[index];
  }
};

struct PC2 {
  static const int OutputSize = 48;
  int source_bit(int index) {
    static const int table[] = {
      13, 16, 10, 23, 0, 4, 2, 27, 14, 5, 20, 9,
      22, 18, 11, 3, 25, 7, 15, 6, 26, 19, 12, 1,
      40, 51, 30, 36, 46, 54, 29, 39, 50, 44, 32, 47,
      43, 48, 38, 55, 33, 52, 45, 41, 49, 35, 28, 31};

    return table[index];
  }
};

SmartPointer<SubKeys> KeyGenerator::CreateSubKeys(
    const BitVector<64> &initial_key) {
  static const int shift_amounts[] = { 1, 1, 2, 2, 2, 2, 2, 2,
                                       1, 2, 2, 2, 2, 2, 2, 1 };

  PC1 perm_pc1;
  BitVector<56> pc1 = initial_key.permute<PC1>(perm_pc1);

  BitVector<28> C[16], D[16];
  C[0] = pc1.sub_vector<28>(0).left_shift(shift_amounts[0]);
  D[0] = pc1.sub_vector<28>(28).left_shift(shift_amounts[0]);

  for (int i = 1; i < 16; i++) {
    C[i] = C[i - 1].left_shift(shift_amounts[i]);
    D[i] = D[i - 1].left_shift(shift_amounts[i]);
  }

  SmartPointer<SubKeys> ret(new SubKeys);

  PC2 pc2;

  for (int i = 0; i < 16; i++) {
    ret->keys_[i] = C[i].append(D[i]).permute(pc2);
  }

  return ret;
}
