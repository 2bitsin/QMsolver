#pragma once

#include <type_traits>

namespace detail
{
	template <typename _Qtype>
	auto _fallback_popcount (_Qtype value)
	{
		if constexpr (std::is_unsigned_v<_Qtype>)
		{
			int count = 0;
			for (; value; ++count)
				value &= (value - 1);
			return count;
		}
		else
		{
			using vt = std::make_unsigned_t<_Qtype>;
			return popcount (static_cast<vt>(value));
		}
	}
}

#ifdef _MSC_VER
#include <intrin.h>
#endif

template <typename _Qtype>
auto popcount (_Qtype value)
{

#ifdef _MSC_VER	
	if constexpr (std::is_signed_v<_Qtype>)
	{
		using vt = std::make_unsigned_t<_Qtype>;
		return popcount (static_cast<vt>(value));
	}
	else
	{
		if constexpr (sizeof (value) <= 2)
			return static_cast<int>(__popcnt16 (value));
		else if constexpr (sizeof (value) <= 4)
			return static_cast<int>(__popcnt (value));
		else if constexpr (sizeof (value) <= 8)
			return static_cast<int>(__popcnt64 (value));
		else
		{
			static_assert(sizeof (value) <= 8, 
				"This isn't supposed to happen");
			return -1;
		}
	}
#elif defined(__clang__) || defined(__GNUC__)
	if constexpr (std::is_signed_v<_Qtype>)
	{
		using vt = std::make_unsigned_t<_Qtype>;
		return popcount (static_cast<vt>(value));
	}
	else
	{
		if constexpr (sizeof (value) <= 2)
			return static_cast<int>(__builtin_popcount (value));
		else if constexpr (sizeof (value) <= 4)
			return static_cast<int>(__builtin_popcountl (value));
		else if constexpr (sizeof (value) <= 8)
			return static_cast<int>(__builtin_popcountll (value));
		else
		{
			static_assert(sizeof (value) <= 8, 
				"This isn't supposed to happen");
			return -1;
		}
	}
#else 
	return _fallback_popcount (value);
#endif
}
