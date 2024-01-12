#pragma once
#include <cstdint>

namespace calc {
	/// @brief	Integral form of the pow (exponential power) function.
	int64_t ipow(int64_t base, uint8_t exp)
	{
		static const uint8_t highest_bit_set[] = {
			0, 1, 2, 2, 3, 3, 3, 3,
			4, 4, 4, 4, 4, 4, 4, 4,
			5, 5, 5, 5, 5, 5, 5, 5,
			5, 5, 5, 5, 5, 5, 5, 5,
			6, 6, 6, 6, 6, 6, 6, 6,
			6, 6, 6, 6, 6, 6, 6, 6,
			6, 6, 6, 6, 6, 6, 6, 6,
			6, 6, 6, 6, 6, 6, 6, 255, // anything past 63 is a guaranteed overflow with base > 1
			255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255,
		};

		int64_t result = 1;

		switch (highest_bit_set[exp]) {
		case 255: // we use 255 as an overflow marker and return 0 on overflow/underflow
			switch (base) {
			case 1:
				return 1;
			case -1:
				return 1 - 2 * (exp & 1);
			default:
				return 0;
			}

			/*if (base == 1) {
				return 1;
			}

			if (base == -1) {
				return 1 - 2 * (exp & 1);
			}

			return 0;*/
		case 6:
			if (exp & 1) result *= base;
			exp >>= 1;
			base *= base;
		case 5:
			if (exp & 1) result *= base;
			exp >>= 1;
			base *= base;
		case 4:
			if (exp & 1) result *= base;
			exp >>= 1;
			base *= base;
		case 3:
			if (exp & 1) result *= base;
			exp >>= 1;
			base *= base;
		case 2:
			if (exp & 1) result *= base;
			exp >>= 1;
			base *= base;
		case 1:
			if (exp & 1) result *= base;
		default:
			return result;
		}
	}

	int64_t imax(int64_t a, int64_t b)
	{
		return a < b
			? b
			: a;
	}
	int64_t imin(int64_t a, int64_t b)
	{
		return a < b
			? a
			: b;
	}


	template<std::integral T>
	T factorial(T const n)
	{
		if constexpr (std::signed_integral<T>) {
			if (n < 0) throw make_exception("Factorial operation expects a positive integer; received ", n, '!');
		}

		T result{ 1 };
		for (auto i{ n }; i > 1; --i) {
			result *= i;
		}
		return result;
	}
}
