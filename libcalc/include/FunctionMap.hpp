#pragma once
#include "func.hpp"

// libcalc
#include <intmath.hpp>		//< for integral math functions

// 307lib
#include <var.hpp>			//< for concepts
#include <print_table.hpp>	//< for term::print_table

// boost
#include <boost/math_fwd.hpp>

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
			std::make_pair("cos", std::make_tuple(new func(cos), "Compute cosine")),
			std::make_pair("sin", std::make_tuple(new func(sin), "Compute sine")),
			std::make_pair("tan", std::make_tuple(new func(tan), "Compute tangent")),
			std::make_pair("acos", std::make_tuple(new func(acos), "Compute arc cosine")),
			std::make_pair("asin", std::make_tuple(new func(asin), "Compute arc sine")),
			std::make_pair("atan", std::make_tuple(new func(atan), "Compute arc tangent")),
			std::make_pair("atan2", std::make_tuple(new func(atan2), "Compute arc tangent with two parameters")),

			/// Hyperbolic Functions
			std::make_pair("cosh", std::make_tuple(new func(cosh), "Compute hyperbolic cosine")),
			std::make_pair("sinh", std::make_tuple(new func(sinh), "Compute hyperbolic sine")),
			std::make_pair("tanh", std::make_tuple(new func(tanh), "Compute hyperbolic tangent")),
			std::make_pair("acosh", std::make_tuple(new func(acosh), "Compute area hyperbolic cosine")),
			std::make_pair("asinh", std::make_tuple(new func(asinh), "Compute area hyperbolic sine")),
			std::make_pair("atanh", std::make_tuple(new func(atanh), "Compute area hyperbolic tangent")),

			/// Exponential and Logarithmic Functions
			std::make_pair("exp", std::make_tuple(new func(exp), "Compute exponential function")),
			//std::make_pair("frexp", std::make_tuple(new func( frexpl ), "Get significand and exponent")), //< uses pointers, must be adapted
			std::make_pair("ldexp", std::make_tuple(new func(ldexp), "Generate value from significand and exponent")),
			std::make_pair("log", std::make_tuple(new func(log), "Compute natural logarithm")),
			std::make_pair("log10", std::make_tuple(new func(log10), "Compute common logarithm")),
			//std::make_pair("modf", std::make_tuple(new func( modfl ), "Break into fractional and integral parts")), //< uses pointers, must be adapted
			std::make_pair("exp2", std::make_tuple(new func(exp2), "Compute binary exponential function")),
			std::make_pair("expm1", std::make_tuple(new func(expm1), "Compute exponential minus one")),
			std::make_pair("ilogb", std::make_tuple(new func(ilogb), "Integer binary logarithm")),
			std::make_pair("log1p", std::make_tuple(new func(log1p), "Compute logarithm plus one")),
			std::make_pair("log2", std::make_tuple(new func(log2), "Compute binary logarithm")),
			std::make_pair("logb", std::make_tuple(new func(logb), "Compute floating-point base logarithm")),
			std::make_pair("scalbn", std::make_tuple(new func(scalbn), "Scale significand using floating-point base exponent")),

			/// Power Functions
			std::make_pair("pow", std::make_tuple(new func(pow), "Raise to power")),
			std::make_pair("sqrt", std::make_tuple(new func(sqrt), "Compute square root")),
			std::make_pair("cbrt", std::make_tuple(new func(cbrt), "Compute cubic root")),
			std::make_pair("hypot", std::make_tuple(new func(hypot), "Compute hypotenuse")),

			/// Error & Gamma Functions
			std::make_pair("erf", std::make_tuple(new func(erf), "Compute error function")),
			std::make_pair("erfc", std::make_tuple(new func(erfc), "Compute complementary error function")),
			std::make_pair("tgamma", std::make_tuple(new func(tgamma), "Compute gamma function")),
			std::make_pair("lgamma", std::make_tuple(new func(lgamma), "Compute log-gamma function")),

			/// Rounding & Remainder Functions
			std::make_pair("ceil", std::make_tuple(new func(ceil), "Round up value")),
			std::make_pair("floor", std::make_tuple(new func(floor), "Round down value")),
			std::make_pair("fmod", std::make_tuple(new func(fmod), "Compute remainder of division")),
			std::make_pair("trunc", std::make_tuple(new func(trunc), "Truncate value")),
			std::make_pair("round", std::make_tuple(new func(round), "Round to nearest")),
			std::make_pair("nearbyint", std::make_tuple(new func(nearbyint), "Round to nearby integral value")),
			std::make_pair("remainder", std::make_tuple(new func(remainder), "Compute remainder of division")),

			/// Floating-point Manipulation Functions
			std::make_pair("copysign", std::make_tuple(new func(copysign), "Copy sign")),
			std::make_pair("nextafter", std::make_tuple(new func(nextafter), "Next representable value")),
			std::make_pair("nexttoward", std::make_tuple(new func(nexttoward), "Next representable value toward precise value")),

			/// Minimum, Maximum, & Difference Functions
			std::make_pair("fdim", std::make_tuple(new func(fdim), "Positive difference")),
			std::make_pair("max", std::make_tuple(new func(max), "Get larger value")),
			std::make_pair("min", std::make_tuple(new func(min), "Get smaller value")),

			/// Other
			std::make_pair("abs", std::make_tuple(new func(abs), "Get Absolute Value")),
			std::make_pair("fma", std::make_tuple(new func(fma), "Multiply-add")),
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
