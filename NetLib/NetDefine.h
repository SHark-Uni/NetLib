#pragma once

namespace Common
{
	class SerializeBuffer;
}
namespace NetLib
{
	constexpr size_t SBUFFER_POOL_SIZE = 6000;
	constexpr size_t SBUFFER_DEFAULT_SIZE = 1536;
	constexpr size_t SESSION_POOL_SIZE = 6000;

	constexpr size_t RINGBUFFER_POOL_SIZE = 6000;

	constexpr size_t RINGBUFFER_QUEUE_SIZE = 8192;
}