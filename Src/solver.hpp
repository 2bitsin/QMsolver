#pragma once

#include <vector>
#include <array>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include "term.hpp"


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
		: implicants_by_rank{{remove_duplicates (std::move (minterms))}}
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
		compute_final_solution (required);
		return required;
	}

	auto compute_final_solution (std::vector<term_type>& result)
	{
		std::unordered_set<value_type> ts;
		std::unordered_set<term_type> is;

		std::unordered_map<term_type, std::size_t> ni;
		std::vector<term_type> si;

		for (auto&& [t, iss] : implicants_by_term)
		{
			ts.emplace (t);
			for (auto&& i : iss)
				is.emplace (i);
		}

		for (;;)
		{
		#ifdef _DEBUG
			std::cout << "Left T's : " << ts.size() << "\n";
		#endif
			ni.clear();
			si.clear();

			for (auto&& i : is)
				for (auto&& t : i.explode ())
					if (ts.contains (t))
						++ni [i];

			for (auto&& [i, n] : ni)
			{
				si.emplace_back (i);
				std::inplace_merge
				(	si.rbegin (),
					std::next (si.rbegin ()),
					si.rend (),
					[&ni] (auto&& lhs, auto&& rhs)
					{
						return ni [lhs] < ni [rhs];
					}
				);
			}

		#ifdef _DEBUG
			std::cout << "-----\n";
			for (auto&& i : si)
				std::cout << i.to_string () << " -> " << ni [i] << "\n";
			std::cout << "-----\n";
		#endif

			auto im = si.front();
			result.push_back(im);
			for (auto&& t : im.explode())
				ts.erase(t);

			is.erase(im);
			if (ts.empty())
				break;
		}
	}

	auto populate_implicant_table ()
	{
		/* Populate implicant table */
		std::vector<term_type> out;
		while (implicants_by_rank.size () <= term_type::length
			&& merge_pass (out, implicants_by_rank.back ()))
			implicants_by_rank.emplace_back (std::move (out));
	}

	void print_coverage (std::ostream& cout)
	{
		/* Print coverage */
		for (auto&& [key, tbl] : implicants_by_term)
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
		auto&& inputs = implicants_by_rank.front ();
		for (auto&& mt : inputs)
			for (auto&& all_imps : reverse (implicants_by_rank))
				for (auto&& imp : all_imps)
				{
					if (imp.cardinality () >= inputs.size ())
						return imp;
					assert (mt.cardlog2 () == 0u);
					if (imp.contains (mt))
						implicants_by_term [mt.bits].emplace_back (imp);
				}
		return std::nullopt;
	}

	auto pick_essential_implicants ()
	{
		std::vector<term_type> terms;
		/* Collecting essential implicants */
		for (auto&& [key, tbl] : implicants_by_term)
			if (tbl.size () == 1ul)
				terms.emplace_back (tbl.back ());
		/* Erase covered terms */
		for (auto&& t : terms)
			for (auto&& k : t.explode ())
				implicants_by_term.erase (k);
		return terms;
	}

private:
	std::vector<
		std::vector<term_type>>
		implicants_by_rank;
	std::unordered_map<value_type,
		std::vector<term_type>>
		implicants_by_term;
};
