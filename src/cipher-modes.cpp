#include "cipher-modes.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <exception>

using namespace std;
using namespace des;


ECBModeSource::ECBModeSource(string file_name) {
  fptr_ = fopen(file_name.c_str(), "rb");
  if (fptr_ == NULL) {
    throw runtime_error(string("error: could not open file `") +
                        file_name + string("`"));
  }
}


ECBModeSource::~ECBModeSource() {
  fclose(fptr_);
}


ECBModeSink::ECBModeSink(std::string file_name) {
  fptr_ = fopen(file_name.c_str(), "wb");
  if (fptr_ == NULL) {
    throw runtime_error(string("error: could not open output file `" +
                               file_name + "`"));
  }
}

ECBModeSink::~ECBModeSink() {
  fclose(fptr_);
}
