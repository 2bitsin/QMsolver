#pragma once

#include <algorithm>
#include <optional>
#include <experimental/generator>

#include "utils.hpp"

template <typename T>
struct term
{
	using value_type = T;
	using term_type = term<T>;
	static constexpr inline auto length = sizeof (T) * 8u;
	static constexpr inline auto wlc = '*';

	constexpr explicit term (std::string_view dgt)
		: term ()
	{
		const auto len = std::min (dgt.size (), length);
		for (auto i = 0u; i < len; ++i)
		{
			const auto m = value_type (1) << i;
			switch (dgt [dgt.size () - i - 1u])
			{
			case '1': bits |= m; break;
			case '0': bits &= ~m; break;
			case wlc: mask &= ~m; break;
			}
		}
	}

	constexpr term
	(	value_type bits = value_type (0),
		value_type mask = ~value_type (0))
	: bits (bits& mask),
		mask (mask)
	{}

	constexpr auto cardinality () const
	{
		auto _one = value_type (1);
		return _one << cardlog2 ();
	}

	constexpr auto inv_mask () const
	{
		return value_type (~mask);
	}

	constexpr auto cardlog2 () const
	{
		return popcount (inv_mask ());
	}

	constexpr bool contains (value_type b) const
	{
		return and_(b, mask) == bits;
	}

	constexpr bool contains (const term<value_type>& b) const
	{
		return (*this) == b || superset_of(b); 
	}
	
	constexpr bool subset_of (const term<value_type>& b) const
	{
		return b.superset_of(*this);
	}

	constexpr bool superset_of (const term<T>& b) const
	{
		const auto carddiff = cardlog2 () - b.cardlog2 ();
		if (carddiff <= 0)
			return false;
		const auto maskdiff = popcount (xor_(inv_mask (), b.inv_mask ()));
		if (maskdiff != carddiff)
			return false;
		return (bits & mask) == (b.bits & mask);
	}

	auto to_string () const
	{
		constexpr auto len = length;
		std::string buff;
		buff.reserve (len);
		for (auto i = len; i > 0u; --i)
		{
			const auto m = value_type (1) << (i - 1u);
			const auto c = mask & m ? (bits & m ? '1' : '0') : wlc;
			buff.push_back (c);
		}
		return buff;
	}

	auto explode () const
		-> std::experimental::generator<value_type>
	{
		const auto o = value_type (1);
		const auto n = cardinality ();
		std::vector<int> xs;
		xs.reserve (length);

		for (value_type i = 0u; i < length; ++i)
		{
			const auto m = o << i;
			if (mask & m)
				continue;
			const auto v = value_type (i - xs.size ());
			xs.push_back (v);
		}

		auto v = bits;
		auto m = o;
		for (value_type i = 0u; i < n; ++i)
		{
			v &= mask;
			auto j = 0u;
			while (j < xs.size ())
			{
				m = i & (o << j);
				v |= (m << xs [j++]);
			}
			co_yield v;
		}
		co_return;
	}

	friend constexpr auto operator == (const term_type& lhs, const term_type& rhs)
	{
		return lhs.mask == rhs.mask && lhs.bits == rhs.bits;
	}

	friend constexpr auto operator != (const term_type& lhs, const term_type& rhs)
	{
		return !(lhs == rhs);
	}

	friend constexpr auto operator < (const term_type& lhs, const term_type& rhs)
	{
		return lhs.subset_of(rhs);
	}

	friend constexpr auto operator > (const term_type& lhs, const term_type& rhs)
	{
		return lhs.superset_of(rhs);
	}

	friend constexpr auto operator <= (const term_type& lhs, const term_type& rhs)
	{
		return lhs == rhs || lhs < rhs;
	}

	friend constexpr auto operator >= (const term_type& lhs, const term_type& rhs)
	{
		return lhs == rhs || lhs > rhs;
	}


	value_type mask;
	value_type bits;
};

template <typename value_type>
inline constexpr auto distance (const term<value_type>& a, const term<value_type>& b)
	-> std::optional<int>
{
	if (a.mask != b.mask)
		return popcount(xor_
		(	and_(a.bits, a.mask),
			and_(b.bits, b.mask)));
	return {};
}

template <typename value_type>
inline constexpr auto combine (const term<value_type>& a, const term<value_type>& b)
	-> std::optional<term<value_type>>
{
	const auto aa = and_(a.bits, a.mask);
	const auto bb = and_(b.bits, b.mask);
	auto m0 = xor_(aa, bb);
	auto mm = and_(a.mask, not_(m0));

	if (a.mask == b.mask && popcount(m0) == 1)
		return term<value_type>{a.bits, mm};
	return std::nullopt;
}


template <typename value_type>
inline constexpr auto absorb (const term<value_type>& a, const term<value_type>& b)
	-> std::optional<term<value_type>>
{
	if (a.contains(b))
		return a;
	if (b.contains(a))
		return b;
	return std::nullopt;
}

auto operator ""_t8 (const char* dgt, std::size_t len)
{
	return term<std::uint8_t>{ {dgt, len}};
}

auto operator ""_t16 (const char* dgt, std::size_t len)
{
	return term<std::uint16_t>{ {dgt, len}};
}

auto operator ""_t32 (const char* dgt, std::size_t len)
{
	return term<std::uint32_t>{ {dgt, len}};
}

auto operator ""_t64 (const char* dgt, std::size_t len)
{
	return term<std::uint64_t>{ {dgt, len}};
}

namespace std
{
	template <typename T>
	struct hash<term<T>>
	{
		auto operator () (const term<T>& t) const 
		{
			static constexpr const std::hash<T> h;
			return h(t.mask)^h(t.bits);
		}
	};
}