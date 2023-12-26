#pragma once

#include "ringbufferbase-system.hpp"
#include <array>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <functional>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

namespace am {

using bytes_view = std::span<const char>;

template <typename Buffer> struct buffers_2 {
  using value_type = Buffer;
  using const_iterator = const value_type *;
  buffers_2()
      : buffer_count_(0){};
  explicit buffers_2(const value_type &buffer)
      : buffer_count_(1) {
    buffers_[0] = buffer;
  }
  buffers_2(const value_type &buffer1, const value_type &buffer2)
      : buffer_count_(2) {
    buffers_[0] = buffer1;
    buffers_[1] = buffer2;
  }

  const_iterator begin() const { return std::addressof(buffers_[0]); }
  const_iterator end() const {
    return std::addressof(buffers_[0]) + buffer_count_;
  }
  bool empty() const { return !buffer_count_; }

  std::size_t size() const {
    if (buffer_count_ == 0)
      return 0;
    auto res = size_t(buffers_[0].size());
    if (buffer_count_ == 2)
      res += buffers_[1].size();
    return res;
  }

  inline std::byte &operator[](size_t pos) const {
    auto b0size = buffers_[0].size();
    if (pos < b0size) {
      return buffers_[0][pos];
    }
    return buffers_[1][pos - b0size];
  }

  inline std::size_t count() const { return buffer_count_; }

private:
  std::array<Buffer, 2> buffers_;
  std::size_t buffer_count_;
};

struct LinnearArray {
  LinnearArray(std::size_t size);
  std::size_t size() const;
  inline char &at(std::size_t pos) { return *(ptr_ + pos); }
  inline const char &at(std::size_t pos) const { return *(ptr_ + pos); }

  inline char *data();
  const char *data() const;

  std::vector<char> to_vector();

private:
  char *ptr_;
  std::size_t len_;
  LinearMemInfo mapped_;
};



struct RingBufferBase {
  RingBufferBase(std::size_t size, std::size_t low_watermark,
             std::size_t high_watermark);

  void reset();

  /// Reduce filled sequence by marking first size bytes of filled sequence as
  /// nonfilled sequence.
  /**
   * Doesn't move or copy anything. Size of nonfilled sequence grows up by size
   * bytes. Start of nonfilled sequence doesn't change. Size of filled sequence
   * reduces by size bytes. Start of filled sequence moves up (circular) by
   * size bytes.
   */
  void commit(std::size_t len);

  /// Reduce nonfilled sequence by marking first size bytes of
  /// nonfilled sequence as filled sequence.
  /**
   * Doesn't move or copy anything. Size of filled sequence grows up by size
   * bytes. Start of filled sequence doesn't change. Size of nonfilled sequence
   * reduces by size bytes. Start of nonfilled sequence moves up (circular) by
   * size bytes.
   */
  void consume(std::size_t size);

  void memcpy_in(const void *data, size_t sz);
  void memcpy_out(void *data, size_t sz);

  bool empty() const;
  std::size_t ready_size() const;
  std::size_t ready_write_size() const;
  bool below_high_watermark() const;
  bool below_low_watermark() const;

  void check(int len, std::string_view method) const;
  inline char char_at(std::size_t pos) const;
  int peek_int() const;
  buffers_2<std::string_view> peek_string_view(int len) const;
  buffers_2<bytes_view> peek_span(int len) const;
  std::span<char> peek_linear_span(int len);
  std::size_t peek_pos() const;

protected:
  LinnearArray _data;

  std::size_t _size;
  std::size_t filled_start_;
  std::size_t filled_size_;
  std::size_t non_filled_start_;
  std::size_t non_filled_size_;
  std::size_t _low_watermark;
  std::size_t _high_watermark;
  std::function<void()> on_commit_{};
  std::function<void()> on_consume_{};
};

} // namespace am
