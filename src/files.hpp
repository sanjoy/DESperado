#ifndef __FILES_HPP
#define __FILES_HPP

#include <string>
#include <stdexcept>
#include <unistd.h>

#include "utils.hpp"

namespace des {

class FileSource {
 public:
  class Reader {
   public:
    Reader(uint8_t* contents, int end)
        : contents_(contents),
          current_(0),
          end_(end) { }

    ALWAYS_INLINE(bool is_end() const) {
      return current_ >= end_;
    }

    ALWAYS_INLINE(void read(uint8_t* buffer, int size)) {
      for (int i = 0; i < min(size, end_ - current_); i++) {
        buffer[i] = contents_[current_ + i];
      }
      current_ += size;
    }

   private:
    uint8_t* contents_;
    int current_, end_;
  };

  explicit FileSource(std::string file_name);

  Reader create_reader() { return Reader(contents_, file_size_); }

  int file_size() const { return file_size_; }

  ~FileSource();

 protected:
  int fd_;
  int mmap_size_;
  int file_size_;
  uint8_t* contents_;
};


class FileSink {
 public:
  class Writer {
   public:
    Writer(int fd) : fd_(fd) { }

    ALWAYS_INLINE(void write(const uint8_t* buffer, int size)) {
      int written_size = ::write(fd_, buffer, size);
      if (written_size != size) throw std::runtime_error("error: short write");
    }

   private:
    int fd_;
  };

  explicit FileSink(std::string file_name);

  Writer create_writer() { return Writer(fd_); }

  ~FileSink();

 protected:
  int fd_;
};


struct Constants {
  static const char* Magic;
  static const int MagicSize = sizeof("DESperado") - 1;
  static const int MinFileSize = MagicSize +
                                 8 + // File size
                                 8; // IV
};


class CiphertextFileSource : public FileSource {
 public:
  CiphertextFileSource(std::string);
  uint64_t actual_size() const { return actual_size_; }

 private:
  uint64_t actual_size_;
};


class CiphertextFileSink : public FileSink {
 public:
  CiphertextFileSink(std::string file_name, int64_t file_size);
};


typedef FileSource PlaintextFileSource;

class PlaintextFileSink : public FileSink {
 public:
  PlaintextFileSink(std::string file_name, uint64_t actual_file_size);
  ~PlaintextFileSink();

 private:
  int64_t actual_file_size_;
};

};

#endif // __FILES_HPP
