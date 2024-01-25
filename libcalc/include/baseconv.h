#pragma once
#include "Number.hpp"

#include <make_exception.hpp>

#include <bisc.hpp>

namespace calc {
	/**
	 * @brief			Converts the specified number to its string representation in the specified base.
	 * @param n		  -	The number to convert.
	 * @param base    -	The base to convert the number to.
	 * @returns			The string representation of the number.
	 * @exception		Throws ex::except when n is a floating-point.
	 */
	std::string to_base(Number const& n, unsigned const base)
	{
		if (!n.has_integral_value())
			throw make_exception("Cannot convert floating-point value ", n, " to base ", base, " (float to base conversions aren't supported).");
		else if (n.is_zero())
			return "0";

		return bisc::ntos(n.cast_to<Number::int_t>(), base);
	}
	/**
	 * @brief			Converts the string representation of a number in the specified base to a number.
	 * @param n		  -	The string representation of a number.
	 * @param base	  -	The base that the string representation is in.
	 * @returns			The number represented by the specified string.
	 */
	Number from_base(std::string const& n, unsigned const base)
	{
		if (base == 10) {
			if (n.find('.') != std::string::npos)
				return{ Number::real_t{ n } };
			else return{ Number::int_t{ n } };
		}
		else if (n.find('.') != std::string::npos)
			throw make_exception("Cannot convert floating-point ", n, " from base ", base, " which doesn't support floating-point values!");
		else return{ bisc::ston<Number::int_t>(n, base) };
	}
}
