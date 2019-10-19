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
	/*
	std::optional<std::vector<term_type>> find_solution (
		std::vector<value_type> term,
		std::vector<term_type> impl,
		std::size_t	u)
	{
		if (term.empty())
			return std::vector<term_type>{impl.begin(), impl.begin() + u};
		std::optional<std::vector<term_type>> best_solution;
		for(auto i = u; i < impl.size(); ++i)
		{
			std::vector<value_type> s_term;
			for (auto&& t : term)
				if (!impl[i].contains(t))
					s_term.emplace_back(t);
			auto s_impl = impl;
			std::swap(s_impl[u], s_impl[i]);					
			auto solution = find_solution(s_term, s_impl, u + 1u);
			if (!solution.has_value())
				continue;
			if (!best_solution.has_value() 
				|| solution->size() < best_solution->size())
				best_solution = std::move(solution);
		}
		return best_solution;
	}
	*/


	std::optional<std::vector<term_type>> find_solution (
		std::vector<value_type> terms,
		std::vector<term_type> implicants)
	{
		std::unordered_set<node, node_hash, node_equal> expanded;
		std::optional<node> best_solution;
		std::priority_queue<node, std::vector<node>, node_less> fringe;

		fringe.emplace (node
		{
			.impl = std::move(implicants),
			.term = std::move(terms),
			.used = 0u
		});		
		while (!fringe.empty())
		{
			auto curr = std::move(fringe.top());
			fringe.pop();
			if (!curr.term.empty())
			{
				const auto u = curr.used;
				if (expanded.count(curr))
					continue;
				expanded.emplace(curr);
				if (best_solution.has_value() 
					&& best_solution->used <= u)
					continue;
				const auto& c_impl = curr.impl;
				for(auto i = u; i < c_impl.size(); ++i)
				{
					auto impl = c_impl;
					std::vector<value_type> term;
					std::swap(impl[u], impl[i]);
					for (auto&& t : curr.term)
						if (!impl[u].contains(t))
							term.emplace_back(t);
					fringe.emplace(node 
					{
						.impl = std::move(impl),
						.term = std::move(term),
						.used = u+1u
					});
				}
				continue;
			}
			return std::vector<term_type>
			{
				curr.impl.begin(),
				curr.impl.begin()+curr.used 
			};

			//if (!best_solution.has_value() 
			//	||best_solution->used > curr.used)
			//	best_solution = curr;
		}
		//if (!best_solution.has_value())
		//	return {};
		//return std::vector<term_type>
		//{
		//	best_solution->impl.begin(), 
		//	best_solution->impl.begin() 
		//		+ best_solution->used 
		//};
	}
};
