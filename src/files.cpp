#include "files.hpp"

#include <cassert>
#include <fcntl.h>
#include <stdexcept>
#include <sys/mman.h>
#include <sys/stat.h>

using namespace std;
using namespace des;

FileSource::FileSource(string file_name) {
  fd_ = open(file_name.c_str(), O_RDONLY);
  if (fd_ == -1) {
    throw runtime_error(string("error: could not open file `") +
                        file_name + "` for reading");
  }

  struct stat buf;
  if (fstat(fd_, &buf) == -1) {
    throw runtime_error(string("error: could not stat file `") +
                       file_name + "`");
  }
  int pagesize = getpagesize();
  file_size_ = buf.st_size;
  mmap_size_ = ((file_size_ + pagesize - 1) / pagesize) * pagesize;

  contents_ = reinterpret_cast<uint8_t*>(
      mmap(NULL, mmap_size_, PROT_READ, MAP_PRIVATE, fd_, 0));
  if (contents_ == MAP_FAILED) {
    throw runtime_error(string("error: could not mmap `") + file_name +
                        "` into memory");
  }
}

FileSource::~FileSource() {
  munmap(contents_, mmap_size_);
  close(fd_);
}


FileSink::FileSink(string file_name)
    : buffer_size_(2 * 1024 * 1024),
      current_(0) {
  fd_ = open(file_name.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (fd_ == -1) {
    throw runtime_error(string("error: could not open `") +
                               file_name + "` for writing");
  }
  buffer_ = new uint8_t[buffer_size_];
}

FileSink::~FileSink() {
  flush_buffer();
  close(fd_);
  delete []buffer_;
}


CiphertextFileSource::CiphertextFileSource(string file_name)
    : FileSource(file_name) {
  // Check header
  if (file_size_ < Constants::MinFileSize) goto inconsistent;

  for (int i = 0; i < Constants::MagicSize; i++) {
    if (contents_[i] != Constants::Magic[i]) goto inconsistent;
  }

  contents_ += Constants::MagicSize;

  actual_size_ = 0;
  for (int i = 0; i < 8; i++) {
    actual_size_ *= 256;
    actual_size_ += contents_[i];
  }
  contents_ += 8;

  {
    int expected_size = (((actual_size_ + 7) / 8) * 8 + Constants::MinFileSize);
    if (expected_size != file_size_) {
      goto inconsistent;
    }
  }

  contents_ += 8; // IV, not used yet.
  return;

inconsistent:
  throw runtime_error(string("error: `") + file_name +
                      string("` does not have a valid header.  Are you "
                             "sure it is encrypted?"));
}


CiphertextFileSink::CiphertextFileSink(string file_name, int64_t file_size)
    : FileSink(file_name) {
  int total_written = 0;
  total_written += ::write(fd_, Constants::Magic, Constants::MagicSize);
  uint8_t size_serialized[8];
  for (int i = 7; i >= 0; i--) {
    size_serialized[i] = file_size % 256;
    file_size /= 256;
  }

  total_written += ::write(fd_, size_serialized, 8);
  total_written += ::write(fd_, size_serialized, 8); // IV, not used

  assert(total_written == Constants::MinFileSize);
}

PlaintextFileSink::PlaintextFileSink(string file_name,
                                     uint64_t actual_file_size)
    : FileSink(file_name),
      actual_file_size_(actual_file_size) { }


PlaintextFileSink::~PlaintextFileSink() {
  ftruncate(fd_, actual_file_size_);
}

const char* Constants::Magic = "DESperado";
