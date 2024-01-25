#pragma once
#include <cstdint>

namespace calc {
	/**
	 * @brief		Performs a factorial operation.
	 * @tparam T	The type of number to use. It must support arithmetic operators.
	 * @param n		The number to get the factorial of.
	 * @returns		The result of the operation.
	 */
	template<typename T>
	T factorial(T const n)
	{
		if (n < 0) throw make_exception("Factorial operation expects a positive integer; received ", n, '!');

		T result{ 1 };
		for (auto i{ n }; i > 1; --i) {
			result *= i;
		}
		return result;
	}
}
