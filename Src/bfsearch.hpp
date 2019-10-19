#pragma once

#include <vector>
#include <algorithm>
#include <numeric>
#include <unordered_map>
#include <unordered_set>
#include <queue>

#include "term.hpp"

template <typename T>
struct bfsearch
{
	using value_type = T;
	using term_type = term<T>;


	//   [----selected implicants------|----unselected implicants---]
	//																	used				

	struct node
	{
		std::vector<term_type> impl{};
		std::vector<value_type> term{};
		std::size_t used{0};
	};

	struct node_hash
	{
		auto operator () (const node& lhs) const
		{
			static constexpr const std::hash<term_type> h{};
			std::size_t total = 0u;
			for (auto i = 0u; i < lhs.used; ++i)
				total ^= h (lhs.impl [i]);
			return total;
		}
	};

	struct node_equal
	{
		auto operator () (const node& lhs, const node& rhs) const
		{
			if (lhs.used != rhs.used)
				return false;
			const auto& i_lhs = lhs.impl;
			const auto& i_rhs = rhs.impl;
			for (auto i = 0u; i < lhs.used; ++i)
				if (i_lhs [i] != i_rhs [i])
					return false;
			return true;
		}
	};

	struct node_less
	{
		auto operator () (const node& lhs, const node& rhs) const
		{
			if (lhs.term.size () 
				== rhs.term.size ())
				return lhs.used < rhs.used;
			return lhs.term.size ()
					 < rhs.term.size ();
		}
	};

	std::optional<std::vector<term_type>> find_solution (
		std::vector<value_type> terms,
		std::vector<term_type> impl)
	{	
		return std::nullopt;
	}
};
