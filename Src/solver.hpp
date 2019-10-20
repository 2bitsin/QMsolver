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
		: implicant_table{{remove_duplicates (std::move (minterms))}}
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


		std::map<term_type, std::vector<value_type>> implicant_to_values;
		auto value_to_implicant = coverage_table;

		for (auto&& implicants_by_cardinality : implicant_table)
		{
			for (auto&& implicant : implicants_by_cardinality)
			{
				for (auto& value : implicant.explode ())
				{
					if (coverage_table.count (value))
						implicant_to_values [implicant].emplace_back (value);
				}
			}
		}
		for (auto&& [implicant, value] : implicant_to_values)
		{
			std::cout << implicant.to_string () << " (" << value.size () << ")\n";
		}

				
		for (;;)
		{
			auto implicant = implicant_to_values.begin()->first;
			for (auto&& value : implicant.explode())
				value_to_implicant.erase(value);

			implicant_to_values.clear();
			for (auto&& [value, implicant_vector] : value_to_implicant)
			{
				for (auto&& implicant : implicant_vector)
					implicant_to_values[implicant].emplace_back(value);
			}

			for (auto&& [implicant, value_vector] : implicant_to_values)
			{
				std::cout << implicant.to_string () << " (" << value_vector.size () << ")\n";
			}
			__debugbreak();
		}

		return required;
	}

	auto populate_implicant_table ()
	{
		/* Populate implicant table */
		std::vector<term_type> out;
		while (implicant_table.size () <= term_type::length
			&& merge_pass (out, implicant_table.back ()))
			implicant_table.emplace_back (std::move (out));
	}

	void print_coverage (std::ostream& cout)
	{
		/* Print coverage */
		for (auto&& [key, tbl] : coverage_table)
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
		auto&& inputs = implicant_table.front ();
		for (auto&& mt : inputs)
			for (auto&& all_imps : reverse (implicant_table))
				for (auto&& imp : all_imps)
				{
					if (imp.cardinality () >= inputs.size ())
						return imp;
					assert (mt.cardlog2 () == 0u);
					if (imp.contains (mt))
						coverage_table [mt.bits].emplace_back (imp);
				}
		return std::nullopt;
	}

	auto pick_essential_implicants ()
	{
		std::vector<term_type> terms;
		/* Collecting essential implicants */
		for (auto&& [key, tbl] : coverage_table)
			if (tbl.size () == 1ul)
				terms.emplace_back (tbl.back ());
		/* Erase covered terms */
		for (auto&& t : terms)
			for (auto&& k : t.explode ())
				coverage_table.erase (k);
		return terms;
	}

private:
	std::vector<
		std::vector<term_type>>
		implicant_table;
	std::unordered_map<value_type,
		std::vector<term_type>>
		coverage_table;
};
