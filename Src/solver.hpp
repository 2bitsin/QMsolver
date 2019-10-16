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

	static auto remove_duplicates (std::vector<value_type> in)
		-> std::vector<term_type>
	{
		std::sort (in.begin (), in.end ());
		auto ue = std::unique (in.begin (), in.end ());
		return {in.begin (), ue};
	}

	solver (std::vector<value_type> minterms)
	: imp_table{remove_duplicates (std::move (minterms))}
	{}

	auto merge_pass (std::vector<term_type>& dst,
		const std::vector<term_type>& src)
	{
		std::unordered_set<term_type> prev;
		for (auto i = 0u; i < src.size () - 1u; ++i)
			for (auto j = i + 1u; j < src.size (); ++j)
			{
				auto ppatt = combine (src [i], src [j]);
				if (!ppatt.has_value ())
					continue;
				if (prev.count (ppatt.value ()))
					continue;
				dst.emplace_back (ppatt.value ());
				prev.emplace (ppatt.value ());
			}
		return dst.size ();
	}

	auto solve () -> std::vector<term_type>
	{
		auto max_cardinality_log2 = populate_implicant_table ();
		auto cover_all = populate_coverate_table (max_cardinality_log2);
		if (cover_all.has_value())
			return { cover_all.value() };
		auto essentials = pick_essential_implicants ();
		print_coverage (std::cout);
		return {};
	}

	auto populate_implicant_table ()
	{
		/* Populate implicant table */
		auto cl2 = 0u;
		const auto xcl2 = term_type::length;
		while (cl2 < xcl2 - 1u)
		{
			auto& o = imp_table [cl2 + 1ull];
			const auto& i = imp_table [cl2];
			if (!merge_pass (o, i))
				break;
			++cl2;
		}
		return cl2;
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

	auto populate_coverate_table (unsigned int cl2)
		-> std::optional<term_type>
	{
		/* Populate coverage table */
		for (auto&& mt : imp_table [0])
		{
			for (auto tid = cl2; tid > 0; --tid)
				for (auto&& imp : imp_table [tid])
				{
					if (imp.cardinality () >= imp_table [0].size ())
						return imp ;
					if (imp.contains (mt))
						cov_table [mt].emplace_back (&imp);
				}
		}
		return std::nullopt;
	}

	
	auto pick_essential_implicants ()
	{
		std::vector<term_type> terms;
		/* Collecting essential implicants */
		for (auto&& [key, tbl] : cov_table)
		{
			if (tbl.size () != 1ul)
				continue;
			terms.emplace_back (*tbl.back ());
		}
		/* Erase covered terms */
		for (auto&& t : terms)
			for (auto&& k : t.explode ())
				cov_table.erase (k);
		return terms;
	}

private:
	std::array
	<	std::vector<term_type>,
		term_type::length>
		imp_table;
	std::unordered_map<term_type,
		std::vector<term_type*>>
		cov_table;
};
