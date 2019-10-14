#pragma once

#include <vector>
#include <array>
#include <algorithm>
#include "term.hpp"

template <typename _Qtype>
struct solver
{
	using value_type = _Qtype;
	using term_type = term<value_type>;

	solver (std::vector<value_type> minterms)
	: minterms (std::move (minterms))
	{}

	auto solve ()
	{		
		std::array
		<	std::vector<term_type>,
			term_type::length> 
			grouped;

		grouped[0] = implicants;
		
		for(auto g = 0u; g < grouped.size(); ++g)
		{
			const auto len = grouped[g].size();
			if (len == 0)
				break;
			for(auto j = 0u; j < len - 1u; ++j)
			{
				const auto& lhs = implicants[j];
				for(auto i = j; i < len; ++i)
				{
					const auto& rhs = implicants[i];
					if (rhs.mask != lhs.mask)
						continue;
					const auto dist = distance(lhs, rhs);
					if (!dist.has_value() || dist.value() != 1)
						continue;					
					grouped[g+1].push_back(merge(lhs, rhs));
				}
			}
		}

			
		return 1;
	}

private:
	std::vector<value_type> minterms;
};
