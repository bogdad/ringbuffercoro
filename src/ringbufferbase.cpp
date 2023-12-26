#include "ringbufferbase.hpp"

#include "ringbufferbase-system.hpp"

#include <cstddef>
#include <cstring>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace am {

LinnearArray::LinnearArray(std::size_t size)
    : ptr_(nullptr)
    , len_(0)
    , mapped_(size) {
  len_ = mapped_.len_;
  ptr_ = mapped_.p1_;
}

std::size_t LinnearArray::size() const { return len_; }

inline char *LinnearArray::data() { return ptr_; }

const char *LinnearArray::data() const { return ptr_; }

std::vector<char> LinnearArray::to_vector() {
  auto res = std::vector<char>(size());
  memcpy(res.data(), data(), size());
  return res;
}

RingBufferBase::RingBufferBase(std::size_t size, std::size_t low_watermark,
                               std::size_t high_watermark)
    : _data(size)
    , _size(_data.size())
    , filled_start_(0)
    , filled_size_(0)
    , non_filled_start_(0)
    , non_filled_size_(_data.size())
    , _low_watermark(low_watermark)
    , _high_watermark(high_watermark) {}

void RingBufferBase::reset() {
  filled_start_ = 0;
  filled_size_ = 0;
  non_filled_start_ = 0;
  non_filled_size_ = _size;
}

void RingBufferBase::commit(std::size_t len) {
  non_filled_size_ += len;
  filled_size_ -= len;
  filled_start_ += len;
  filled_start_ %= _size;
  on_commit_();
}

void RingBufferBase::consume(std::size_t len) {
  filled_size_ += len;
  non_filled_size_ -= len;
  non_filled_start_ += len;
  non_filled_start_ %= _size;
  on_consume_();
}

void RingBufferBase::memcpy_in(const void *data, size_t sz) {
  auto left_to_the_right = _size - non_filled_start_;
  if (sz > left_to_the_right) {
    std::memcpy(&_data.at(non_filled_start_), data, left_to_the_right);
    std::memcpy(_data.data(),
                static_cast<const char *>(data) + left_to_the_right,
                sz - left_to_the_right);
  } else {
    std::memcpy(&_data.at(non_filled_start_), data, sz);
  }
  consume(sz);
}

void RingBufferBase::memcpy_out(void *data, size_t sz) {
  check(sz, "memcpy_out");

  auto left_to_the_right = _size - filled_start_;
  if (sz > left_to_the_right) {
    std::memcpy(data, &_data.at(filled_start_), left_to_the_right);
    std::memcpy(static_cast<char *>(data) + left_to_the_right, _data.data(),
                sz - left_to_the_right);
  } else {
    std::memcpy(data, &_data.at(filled_start_), sz);
  }
  commit(sz);
}

bool RingBufferBase::empty() const { return filled_size_ == 0; }

std::size_t RingBufferBase::ready_size() const { return filled_size_; }

std::size_t RingBufferBase::ready_write_size() const {
  return non_filled_size_;
}

bool RingBufferBase::below_high_watermark() const {
  return ready_size() < _high_watermark;
}

bool RingBufferBase::below_low_watermark() const {
  return ready_size() < _low_watermark;
}

void RingBufferBase::check(int len, std::string_view method) const {
  if (len > filled_size_) {
    // "RingBuffer " << method << ": cant read " << len
    //           << " >= " << filled_size_ << " debug " << this;
    throw std::runtime_error("bad state");
  }
}

inline char RingBufferBase::char_at(std::size_t pos) const {
  if (pos < _size)
    return _data.at(pos);
  return _data.at(pos - _size);
}

using raw_int = union {
  std::array<char, 4> c;
  int i;
};

int RingBufferBase::peek_int() const {
  check(4, "peek_int");
  int ret = 0;
  if (filled_size_ + 4 < _size) {
    std::memcpy(&ret, &_data.at(filled_start_), sizeof(ret));
  } else {
    raw_int ri;
    ri.c[0] = char_at(filled_start_);
    ri.c[1] = char_at(filled_start_ + 1);
    ri.c[2] = char_at(filled_start_ + 2);
    ri.c[3] = char_at(filled_start_ + 3);
    ret = ri.i;
  }
  return ret;
}

buffers_2<std::string_view> RingBufferBase::peek_string_view(int len) const {
  check(len, "peek_string_view");
  auto left_to_the_right = _size - filled_start_;
  if (len > left_to_the_right) {
    return {std::string_view(&_data.at(filled_start_), left_to_the_right),
            std::string_view(_data.data(), len - left_to_the_right)};
  } else {
    return buffers_2(std::string_view(&_data.at(filled_start_), len));
  }
}

buffers_2<std::span<const char>> RingBufferBase::peek_span(int len) const {
  check(len, "peek_span");
  auto left_to_the_right = _size - filled_start_;
  if (len > left_to_the_right) {
    return {std::span(&_data.at(filled_start_), left_to_the_right),
            std::span(_data.data(), len - left_to_the_right)};
  } else {
    return buffers_2(std::span(&_data.at(filled_start_), len));
  }
}

std::span<char> RingBufferBase::peek_linear_span(int len) {
  check(len, "peek_linear_span");
  static_assert(std::same_as<LinnearArray, decltype(_data)>,
                "_data should be linear array, to support liear view");
  return {&_data.at(filled_start_), static_cast<std::size_t>(len)};
}

std::size_t RingBufferBase::peek_pos() const { return filled_start_; }

} // namespace am
