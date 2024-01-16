#pragma once
#include "func.hpp"

// libcalc
#include <intmath.hpp>		//< for integral math functions

// 307lib
#include <var.hpp>			//< for concepts
#include <print_table.hpp>	//< for term::print_table

// stl
#include <cmath>			//< for math functions
#include <cstdint>			//< for std::uint8_t (for clarity)
#include <map>				//< for std::map
#include <string>			//< for std::string
#include <functional>		//< for std::function

namespace calc {
	class FunctionMap {
		std::map<std::string, std::tuple<base_func*, std::string>> map{
			/// Trigonometric Functions
			std::make_pair("cos", std::make_tuple(new func(cosl), "Compute cosine")),
			std::make_pair("sin", std::make_tuple(new func(sinl), "Compute sine")),
			std::make_pair("tan", std::make_tuple(new func(tanl), "Compute tangent")),
			std::make_pair("acos", std::make_tuple(new func(acosl), "Compute arc cosine")),
			std::make_pair("asin", std::make_tuple(new func(asinl), "Compute arc sine")),
			std::make_pair("atan", std::make_tuple(new func(atanl), "Compute arc tangent")),
			std::make_pair("atan2", std::make_tuple(new func(atan2l), "Compute arc tangent with two parameters")),

			/// Hyperbolic Functions
			std::make_pair("cosh", std::make_tuple(new func(coshl), "Compute hyperbolic cosine")),
			std::make_pair("sinh", std::make_tuple(new func(sinhl), "Compute hyperbolic sine")),
			std::make_pair("tanh", std::make_tuple(new func(tanhl), "Compute hyperbolic tangent")),
			std::make_pair("acosh", std::make_tuple(new func(acoshl), "Compute area hyperbolic cosine")),
			std::make_pair("asinh", std::make_tuple(new func(asinhl), "Compute area hyperbolic sine")),
			std::make_pair("atanh", std::make_tuple(new func(atanhl), "Compute area hyperbolic tangent")),

			/// Exponential and Logarithmic Functions
			std::make_pair("exp", std::make_tuple(new func(expl), "Compute exponential function")),
			//std::make_pair("frexp", std::make_tuple(new func( frexpl ), "Get significand and exponent")), //< uses pointers, must be adapted
			std::make_pair("ldexp", std::make_tuple(new func(ldexpl), "Generate value from significand and exponent")),
			std::make_pair("log", std::make_tuple(new func(logl), "Compute natural logarithm")),
			std::make_pair("log10", std::make_tuple(new func(log10l), "Compute common logarithm")),
			//std::make_pair("modf", std::make_tuple(new func( modfl ), "Break into fractional and integral parts")), //< uses pointers, must be adapted
			std::make_pair("exp2", std::make_tuple(new func(exp2l), "Compute binary exponential function")),
			std::make_pair("expm1", std::make_tuple(new func(expm1l), "Compute exponential minus one")),
			std::make_pair("ilogb", std::make_tuple(new func(ilogbl), "Integer binary logarithm")),
			std::make_pair("log1p", std::make_tuple(new func(log1pl), "Compute logarithm plus one")),
			std::make_pair("log2", std::make_tuple(new func(log2l), "Compute binary logarithm")),
			std::make_pair("logb", std::make_tuple(new func(logbl), "Compute floating-point base logarithm")),
			std::make_pair("scalbn", std::make_tuple(new func(scalbnl), "Scale significand using floating-point base exponent")),
			std::make_pair("scalbln", std::make_tuple(new func(scalblnl), "Scale significand using floating-point base exponent (long)")),

			/// Power Functions
			std::make_pair("pow", std::make_tuple(new func(std::function{ [](Number const& base, Number const& exp) -> Number {
				if (base.has_integral_value() && exp.has_integral_value()) {
					return ipow(base.cast_to<int64_t>(), exp.cast_to<int64_t>());
				}
				else return powl(base.cast_to<long double>(), exp.cast_to<long double>());
			} }), "Raise to power")),
			std::make_pair("sqrt", std::make_tuple(new func(sqrtl), "Compute square root")),
			std::make_pair("cbrt", std::make_tuple(new func(cbrtl), "Compute cubic root")),
			std::make_pair("hypot", std::make_tuple(new func(hypotl), "Compute hypotenuse")),

			/// Error & Gamma Functions
			std::make_pair("erf", std::make_tuple(new func(erfl), "Compute error function")),
			std::make_pair("erfc", std::make_tuple(new func(erfcl), "Compute complementary error function")),
			std::make_pair("tgamma", std::make_tuple(new func(tgammal), "Compute gamma function")),
			std::make_pair("lgamma", std::make_tuple(new func(lgammal), "Compute log-gamma function")),

			/// Rounding & Remainder Functions
			std::make_pair("ceil", std::make_tuple(new func(ceill), "Round up value")),
			std::make_pair("floor", std::make_tuple(new func(floorl), "Round down value")),
			std::make_pair("fmod", std::make_tuple(new func(std::function{ [](Number const& numer, Number const& denom) -> Number { return numer % denom; } }), "Compute remainder of division")),
			std::make_pair("trunc", std::make_tuple(new func(truncl), "Truncate value")),
			std::make_pair("round", std::make_tuple(new func(roundl), "Round to nearest")),
			std::make_pair("nearbyint", std::make_tuple(new func(nearbyintl), "Round to nearby integral value")),
			std::make_pair("remainder", std::make_tuple(new func(remainderl), "Compute remainder (IEC 60559)")),

			/// Floating-point Manipulation Functions
			std::make_pair("copysign", std::make_tuple(new func(copysignl), "Copy sign")),
			std::make_pair("nextafter", std::make_tuple(new func(nextafterl), "Next representable value")),
			std::make_pair("nexttoward", std::make_tuple(new func(nexttowardl), "Next representable value toward precise value")),

			/// Minimum, Maximum, & Difference Functions
			std::make_pair("dim", std::make_tuple(new func(fdiml), "Positive difference")),
			std::make_pair("max", std::make_tuple(new func(std::function{ [](Number const& a, Number const& b) -> Number {
				if (a.has_integral_value() && b.has_integral_value()) {
					return imax(a.cast_to<int64_t>(), b.cast_to<int64_t>());
				}
				else return fmaxl(a.cast_to<long double>(), b.cast_to<long double>());
			} }), "Get larger value")),
			std::make_pair("min", std::make_tuple(new func(std::function{ [](Number const& a, Number const& b) -> Number {
				if (a.has_integral_value() && b.has_integral_value()) {
					return imin(a.cast_to<int64_t>(), b.cast_to<int64_t>());
				}
				else return fminl(a.cast_to<long double>(), b.cast_to<long double>());
			} }), "Get smaller value")),

			/// Other
			std::make_pair("abs", std::make_tuple(new func(std::function{ [](Number const& n) -> Number {
				if (n.has_integral_value()) {
					return llabs(n.cast_to<long long>());
				}
				else return fabsl(n.cast_to<long double>());
			} }), "Get Absolute Value")),
			std::make_pair("fma", std::make_tuple(new func(fmal), "Multiply-add")),
		};

	public:
		~FunctionMap()
		{
			for (auto& pr : map) {
				if (auto* ptr = std::get<0>(pr.second)) {
					delete ptr;
				}
			}
		}

		/// @brief	Gets the base_func pointer for the specified functionName.
		base_func const* const get(std::string const& functionName) const noexcept(false)
		{
			if (const auto& it{ map.find(functionName) }; it != map.end()) {
				return std::get<0>(it->second);
			}
			return nullptr;
		}

		/// @brief	Invokes the specified functionName with the specified args and returns the result.
		Number invoke(std::string const& functionName, std::vector<Number> const& args) const
		{
			return get(functionName)->invoke(args);
		}
		/// @brief	Invokes the specified functionName with the specified args and returns the result.
		template<typename... Args>
		Number invoke(std::string const& functionName, Args&&... args) const
		{
			return get(functionName)->invoke(std::vector<Number>{ static_cast<Number>(std::forward<Args>(args))... });
		}

		/// @brief	Determines whether the specified name refers to a function.
		inline bool isFunction(std::string const& name) const
		{
			return map.contains(name);
		}

		/// @brief	Gets the number of parameters required by the specified functionName.
		inline size_t getParamsCount(std::string const& functionName) const
		{
			return get(functionName)->getParamsCount();
		}

		friend std::ostream& operator<<(std::ostream& os, const FunctionMap& m)
		{
			return os << term::print_table(m.map.begin(), m.map.end(), {
				{ "Function", [](auto&& pr) { return pr.first; } },
										   { "Param#", [](auto&& pr) { return std::to_string(std::get<0>(pr.second)->getParamsCount()); }, term::HorizontalAlignment::Center },
										   { "Description", [](auto&& pr) { return std::get<1>(pr.second);  } },
										   });
		}
	};
}
