#pragma once

namespace Common
{
	class SerializeBuffer;
}
namespace NetLib
{
	enum class NETLIB_POOL_SIZE
	{
		SBUFFER_POOL_SIZE = 4096,
		SBUFFER_DEFAULT_SIZE = 1536,
		SESSION_POOL_SIZE = 4096,
	};
}