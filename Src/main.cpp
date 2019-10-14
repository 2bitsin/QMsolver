#include <cassert>
#include <cstdio>
#include <vector>
#include <string_view>
#include <bitset>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <string>

#include "popcount.hpp"
#include "term.hpp"
#include "solver.hpp"

void test_term ();

auto parse_arguments(std::vector<std::uint8_t>& input, std::istream& cin)
{
	std::istream_iterator<std::string> ib{cin}, ie;
	std::back_insert_iterator<std::vector<std::uint8_t>> bii{input};
	auto parse_interger = [](auto&& in) -> std::uint8_t
	{
		const auto pfx = in.substr(0, 2);
		auto base = 10;
		if (pfx == "0b")
			base = 2;
		else if (pfx == "0o")
			base = 8;
		else if (pfx == "0x")
			base = 16;	
		return (std::uint8_t)std::stoul
		(	base != 10 ? in.substr(2) : in, 
			nullptr, 
			base);
	};

	std::transform(ib, ie, bii, parse_interger);
}

using args_t = std::vector<std::string_view>;
int main (int argc, char** argv)
{
	args_t args{argv, argv+argc};

	std::vector<std::uint8_t> input;
	parse_arguments(input, std::cin);	
	solver<std::uint8_t> s{input};

	s.solve ();

	return 0;
}

void test_term ()
{
	auto a = "*101*101"_t8;
	auto b = "110*110*"_t8;
	auto c = "*10*110*"_t8;
	auto d = "*11*110*"_t8;
	auto e = "**0*110*"_t8;
	auto f = "10011100"_t8;
	auto g = "****0000"_t8;
	auto h = "*0*0*0*0"_t8;


	assert (a.to_string () == "*101*101");
	assert (b.to_string () == "110*110*");
	assert (c.to_string () == "*10*110*");
	assert (d.to_string () == "*11*110*");
	assert (e.to_string () == "**0*110*");
	assert (f.to_string () == "10011100");

	assert (a.cardlog2() == 2);
	assert (b.cardlog2() == 2);
	assert (c.cardlog2() == 3);
	assert (d.cardlog2() == 3);
	assert (e.cardlog2() == 4);
	assert (f.cardlog2() == 0);

	assert (a.contains (b) == false);
	assert (b.contains (a) == false);

	assert (b.contains (c) == false);
	assert (c.contains (b) == true);
	assert (d.contains (b) == false);
	assert (e.contains (b) == true);
	assert (e.contains (f) == true);
	assert (d.contains (f) == false);

}
