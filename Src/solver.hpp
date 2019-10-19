#pragma once

#include <vector>
#include <array>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include "term.hpp"
#include "bfsearch.hpp"

template <typename _Qtype>
struct solver
{
	using value_type = _Qtype;
	using term_type = term<value_type>;

	static auto remove_duplicates (std::vector<value_type> in)
		-> std::vector<term_type>
	{
		std::sort (in.begin (), in.end ());
		auto ue = std::unique (in.begin (), in.end ());
		return {in.begin (), ue};
	}

	solver (std::vector<value_type> minterms)
		: imp_table{{remove_duplicates (std::move (minterms))}}
	{}

	auto merge_pass (std::vector<term_type>& dst,
		const std::vector<term_type>& src)
	{
		std::unordered_set<term_type> unique;
		for (auto&& [lhs, rhs] :
			each_pair (src.begin (), src.end ()))
		{
			auto ppatt = combine (lhs, rhs);
			if (!ppatt.has_value ()
				|| unique.count (ppatt.value ()))
				continue;
			dst.emplace_back (ppatt.value ());
			unique.emplace (ppatt.value ());
		}
		return dst.size ();
	}

	auto solve () -> std::vector<term_type>
	{
		populate_implicant_table ();
		auto cover_all = populate_coverate_table ();
		if (cover_all.has_value ())
			return {cover_all.value ()};
		//print_coverage (std::cout);
		auto required = pick_essential_implicants ();

		std::vector<value_type> term;
		std::vector<term_type> impl;		
		for (auto&& [k, v] : cov_table)
			term.emplace_back(k.bits);
		for (auto&& all : imp_table)
			for (auto&& i : all)
				impl.emplace_back(i);
		std::sort(impl.begin(), impl.end(), [](auto&& lhs, auto&& rhs) { return rhs.cardlog2() < lhs.cardlog2(); } );
		for(auto&& t : impl)
			std::cout << t.to_string() << " (" << t.cardlog2() << ")" << "\n";
		bfsearch<value_type> bfs;
 		auto solution = bfs.find_solution(std::move(term), std::move(impl));
		if (!solution.has_value())
			return {};
		required.insert(required.end(), solution->begin(), solution->end());
		return required;
	}

	auto populate_implicant_table ()
	{
		/* Populate implicant table */
		std::vector<term_type> out;
		while (imp_table.size () <= term_type::length
			&& merge_pass (out, imp_table.back ()))
			imp_table.emplace_back (std::move (out));
	}

	void print_coverage (std::ostream& cout)
	{
		/* Print coverage */
		for (auto&& [key, tbl] : cov_table)
		{
			cout
				<< key.to_string ()
				<< " (" << tbl.size () << ") "
				<< "\n";
			for (auto* pimp : tbl)
			{
				cout
					<< " << "
					<< pimp->to_string ()
					<< " (" << pimp->cardlog2 () << ") "
					<< "\n";
			}
		}
	}

	auto populate_coverate_table ()
		-> std::optional<term_type>
	{
		/* Populate coverage table */
		auto&& inputs = imp_table.front ();
		for (auto&& mt : inputs)
			for (auto&& all_imps : reverse(imp_table))
				for (auto&& imp : all_imps)
				{
					if (imp.cardinality () >= inputs.size ())
						return imp;
					if (imp.contains (mt))
						cov_table [mt].emplace_back (&imp);
				}
		return std::nullopt;
	}

	auto pick_essential_implicants ()
	{
		std::vector<term_type> terms;
		/* Collecting essential implicants */
		for (auto&& [key, tbl] : cov_table)
			if (tbl.size () == 1ul)				
				terms.emplace_back (*tbl.back ());
		/* Erase covered terms */
		for (auto&& t : terms)
			for (auto&& k : t.explode ())
				cov_table.erase (k);
		return terms;
	}

private:
	std::vector<
		std::vector<term_type>>
		imp_table;
	std::unordered_map<term_type,
		std::vector<term_type*>>
		cov_table;
};
