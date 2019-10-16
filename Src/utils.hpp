#pragma once

#include <type_traits>
#include <iterator>
#include <experimental/generator>

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

 /*
	*	Utility functions to circumvent stupid int promotion rules >: (
	*/

template <typename _Itype>
auto not_ (_Itype i)
{
	return _Itype (~i);
}

template <typename _Itype>
auto xor_ (_Itype lhs, std::enable_if_t<true, _Itype> rhs)
{
	return _Itype (lhs ^ rhs);
}

template <typename _Itype>
auto and_ (_Itype lhs, std::enable_if_t<true, _Itype> rhs)
{
	return _Itype (lhs & rhs);
}

template <typename _Itype>
auto _or (_Itype lhs, std::enable_if_t<true, _Itype> rhs)
{
	return _Itype (lhs | rhs);
}


template <typename _Iter>
auto each_pair (_Iter b, _Iter e)->
std::experimental::generator<std::tuple<
	typename std::iterator_traits<_Iter>::value_type,
	typename std::iterator_traits<_Iter>::value_type>>
{
	using std::next;
	for (auto i = b; next (i) != e; ++i)
		for (auto j = next (i); j != e; ++j)
			co_yield{*i, *j};
	co_return;
}

template <typename _Callback, typename _Iter>
auto for_each_pair (_Callback&& f, _Iter b, _Iter e)
{
	using std::next;
	for (auto&& [lhs, rhs] : each_pair(b, e))
		f (lhs, rhs);
}
