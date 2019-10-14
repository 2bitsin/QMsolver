#pragma once

#include <vector>
#include <array>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include "term.hpp"

template <typename _Qtype>
struct solver
{
	using value_type = _Qtype;
	using term_type = term<value_type>;

	solver (std::vector<value_type> minterms)
	: imp_table{std::vector<term_type>{
			minterms.begin (), 
			minterms.end ()
		}}
	{}

	auto merge_pass(std::vector<term_type>& dst, 
		const std::vector<term_type>& src)
	{
		std::unordered_set<term_type> prev;
		for (auto i = 0u; i < src.size() - 1u; ++i)
		for (auto j = i + 1u; j < src.size(); ++j)
		{
				auto ppatt = combine(src[i], src[j]);
				if (!ppatt.has_value())
					continue;
				if (prev.count(ppatt.value()))
					continue;
				dst.emplace_back(ppatt.value());
				prev.emplace(ppatt.value());
		}
		return dst.size();
	}

	auto solve ()
	{
		const auto xcl2 = term_type::length;
		auto cl2 = 0u;
		while(cl2 < xcl2 - 1u)
		{
			if (!merge_pass
			(	imp_table[cl2 + 1ull], 
				imp_table[cl2]))
				break;
			++cl2;
		}
	
		

		return 1;
	}

private:
	std::array
	<	std::vector<term_type>,
		term_type::length>
		imp_table;
	std::unordered_map<value_type, 
		std::vector<term_type*>>
		cov_table;
};
