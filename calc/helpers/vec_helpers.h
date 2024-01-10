#pragma once

// STL
#include <vector>
#include <utility>

template<typename T, class Predicate>
inline std::vector<std::vector<T>> split_vec(std::vector<T> const& vec, Predicate const& pred)
{
	std::vector<std::vector<T>> out;
	std::vector<T> buf;
	buf.reserve(vec.size());

	for (auto it{ vec.begin() }, it_end{ vec.end() }; it != it_end; ++it) {
		if (pred(*it)) {
			// move the buffer into the output vector
			out.emplace_back(std::move(buf));
			buf = {};
			buf.reserve(vec.size());
		}
		else buf.emplace_back(*it);
	}

	if (!buf.empty())
		out.emplace_back(std::move(buf));

	return out;
}
