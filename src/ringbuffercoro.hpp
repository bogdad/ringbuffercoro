#pragma once

#include "ringbuffer.hpp"
#include <coroutine>
#include <cstddef>

namespace am {

template <typename ConstBuffer, typename MutableBuffer>
struct RingBufferCoro: public RingBuffer<ConstBuffer, MutableBuffer> {
	struct AwaiterNotFull {
		bool await_ready() const noexcept;
  		bool await_suspend(std::coroutine_handle<> coroutine) noexcept;
  		void await_resume() noexcept {}

  		AwaiterNotFull operator co_await() const noexcept;
	};

	struct AwaiterNotEmpty {
		bool await_ready() const noexcept;
  		bool await_suspend(std::coroutine_handle<> coroutine) noexcept;
  		void await_resume() noexcept {}

  		AwaiterNotFull operator co_await() const noexcept;
	};

	AwaiterNotFull wait_not_full(std::size_t guaranteed_free_size);
	AwaiterNotEmpty wait_not_empty(std::size_t guaranteed_filled_size);

	RingBufferCoro(std::size_t size, std::size_t low_watermark, std::size_t high_watermark): RingBuffer<ConstBuffer, MutableBuffer>(size, low_watermark, high_watermark) {}
};

} // namespace am
