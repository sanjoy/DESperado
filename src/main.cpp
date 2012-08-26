#include <string>
#include <iostream>

#include "cipher-modes.hpp"
#include "des.hpp"

using namespace std;
using namespace des;

namespace {


void show_usage() {
  printf("Usage:\n");
}


class Arguments {
 public:
  Arguments() : encrypt_set_(false) { }

#define STRING_ACCESSOR(field)                                          \
  void set_ ## field( const string & field) { field ## _ = field; }     \
  string field () const { return field ## _ ; }

  STRING_ACCESSOR(key);
  STRING_ACCESSOR(input_file);
  STRING_ACCESSOR(output_file);

#undef STRING_ACCESSOR

  void set_encrypt(bool value) { encrypt_ = value; encrypt_set_ = true; }
  bool encrypt() const { return encrypt_; }

  void validate() {
    if (!encrypt_set_) {
      show_usage();
      throw runtime_error("error: you must tell me whether to encrypt or "
                          "decrypt");
    }

    if (!IsHexStr(key_) || key_.length() != 16) {
      show_usage();
      throw runtime_error("error: key must be a hex string");
    }

    if (input_file_.length() == 0) {
      show_usage();
      throw runtime_error("error: please enter a valid output file");
    }

    if (output_file_.length() == 0) {
      show_usage();
      throw runtime_error("error: please enter a valid input file");
    }
  }

 private:
  string key_;
  string input_file_;
  string output_file_;
  bool encrypt_;
  bool encrypt_set_;

  static bool IsHexChar(char c) {
    return ('0' <= c && c <= '9') || ('a' <= c && c <= 'f') ||
        ('A' <= c && c <= 'F');
  }

  static bool IsHexStr(string str) {
    for (int i = 0; i < static_cast<int>(str.length()); i++) {
      if (!IsHexChar(str[i])) return false;
    }
    return true;
  }
};


bool parse_args(Arguments* args, int argc, char** argv) {
  argc--;
  argv++;


#define CMD_CASE(field_name) do {               \
    if (i < (argc - 1)) {                       \
      args->set_ ## field_name(argv[i + 1]);    \
      i++;                                      \
    } else {                                    \
      show_usage();                             \
      return false;                             \
    }                                           \
  } while(0)

  for (int i = 0; i < argc; i++) {
    string arg = argv[i];

    if (arg == "--help" || arg == "-h") {
      show_usage();
      return false;
    } else if (arg == "--input-file" || arg == "-i") {
      CMD_CASE(input_file);
    } else if (arg == "--output-file" || arg == "-o") {
      CMD_CASE(output_file);
    } else if (arg == "--encrypt" || arg == "-e") {
      args->set_encrypt(true);
    } else if (arg == "--decrypt" || arg == "-d") {
      args->set_encrypt(false);
    } else if (arg == "--key" || arg == "-k") {
      CMD_CASE(key);
    } else {
      cout << "invalid " << argv[i] << endl;
      return false;
    }
  }

  return true;

#undef CMD_CASE
}

}


int main(int argc, char** argv) {

  try {
    Arguments args;
    if (!parse_args(&args, argc, argv)) {
      show_usage();
      return 0;
    }
    args.validate();

    ECBModeSource source(args.input_file());
    ECBModeSink sink(args.output_file());

    SmartPointer<SubKeys> subkeys = KeyGenerator::CreateSubKeys(
        BitVector<64>::FromHexString(args.key()));

    if (!args.encrypt()) subkeys->invert();

    DES::Execute(subkeys.get_pointer(), source, sink);
  } catch (const exception &e) {
    cerr << e.what() << endl;
    return -1;
  }

  return 0;
}
