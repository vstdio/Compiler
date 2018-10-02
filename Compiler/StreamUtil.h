#pragma once
#include <ostream>

namespace StreamUtil
{
template <typename Iterable>
void PrintIterable(
	std::ostream& output,
	const Iterable& iterable,
	const std::string& separator = " ",
	const std::string& prefix = "",
	const std::string& suffix = "",
	bool newline = true)
{
	output << prefix;
	for (auto it = iterable.cbegin(); it != iterable.cend(); ++it)
	{
		output << *it;
		if (std::next(it) != iterable.cend())
		{
			output << separator;
		}
	}
	output << suffix;
	if (newline)
	{
		output << "\n";
	}
}
}
