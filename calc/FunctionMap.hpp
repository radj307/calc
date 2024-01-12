#pragma once
#include "operators.hpp"

// libcalc
#include <intmath.hpp>		//< for integral math functions

// 307lib
#include <var.hpp>			//< for concepts

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
			std::make_pair("cos", std::make_tuple(new func(cosl), "Cosine")),
			std::make_pair("sin", std::make_tuple(new func(sinl), "Sine")),
			std::make_pair("tan", std::make_tuple(new func(tanl), "Tangent")),
			std::make_pair("acos", std::make_tuple(new func(acosl), "Inverse Cosine")),
			std::make_pair("asin", std::make_tuple(new func(asinl), "Inverse Sine")),
			std::make_pair("atan", std::make_tuple(new func(atanl), "Inverse Tangent")),
			std::make_pair("atan2", std::make_tuple(new func(atan2l), "Binary Tangent")),
			/// Hyperbolic Functions
			std::make_pair("cosh", std::make_tuple(new func(coshl), "Hyperbolic Cosine")),
			std::make_pair("sinh", std::make_tuple(new func(sinhl), "Hyperbolic Sine")),
			std::make_pair("tanh", std::make_tuple(new func(tanhl), "Hyperbolic Tangent")),
			std::make_pair("acosh", std::make_tuple(new func(acoshl), "Inverse Hyperbolic Cosine")),
			std::make_pair("asinh", std::make_tuple(new func(asinhl), "Inverse Hyperbolic Sine")),
			std::make_pair("atanh", std::make_tuple(new func(atanhl), "Inverse Hyperbolic Tangent")),
			/// Exponential and Logarithmic Functions
			std::make_pair("exp", std::make_tuple(new func(expl), "")),
			//std::make_pair("frexp", std::make_tuple(new func( frexpl ), "")), //< uses pointers, must be adapted
			std::make_pair("ldexp", std::make_tuple(new func(ldexpl), "")),
			std::make_pair("log", std::make_tuple(new func(logl), "")),
			std::make_pair("log10", std::make_tuple(new func(log10l), "")),
			//std::make_pair("modf", std::make_tuple(new func( modfl ), "")), //< uses pointers, must be adapted
			std::make_pair("exp2", std::make_tuple(new func(exp2l), "")),
			std::make_pair("expm1", std::make_tuple(new func(expm1l), "")),
			std::make_pair("ilogb", std::make_tuple(new func(ilogbl), "")),
			std::make_pair("log1p", std::make_tuple(new func(log1pl), "")),
			std::make_pair("log2", std::make_tuple(new func(log2l), "")),
			std::make_pair("logb", std::make_tuple(new func(logbl), "")),
			std::make_pair("scalbn", std::make_tuple(new func(scalbnl), "")),
			std::make_pair("scalbln", std::make_tuple(new func(scalblnl), "")),
			/// Power Functions
			std::make_pair("pow", std::make_tuple(new func(std::function{ [](Number const& base, Number const& exp) -> Number {
				if (base.has_integral_value() && exp.has_integral_value()) {
					return ipow(base.cast_to<int64_t>(), exp.cast_to<int64_t>());
				}
				else return powl(base.cast_to<long double>(), exp.cast_to<long double>());
			} }), "Calculates the result of an exponent.")),
			std::make_pair("sqrt", std::make_tuple(new func(sqrtl), "Calculates a square root.")),
			std::make_pair("cbrt", std::make_tuple(new func(cbrtl), "Calculates a cubic root.")),
			std::make_pair("hypot", std::make_tuple(new func(hypotl), "Calculate hypotenuse.")),
			/// Error & Gamma Functions
			std::make_pair("erf", std::make_tuple(new func(erfl), "")),
			std::make_pair("erfc", std::make_tuple(new func(erfcl), "")),
			std::make_pair("tgamma", std::make_tuple(new func(tgammal), "")),
			std::make_pair("lgamma", std::make_tuple(new func(lgammal), "")),
			/// Rounding & Remainder Functions
			std::make_pair("ceil", std::make_tuple(new func(ceill), "Raises a number to the nearest integral.")),
			std::make_pair("floor", std::make_tuple(new func(floorl), "Lowers a number to the nearest integral.")),
			std::make_pair("fmod", std::make_tuple(new func(fmodl), "")), //< implemented as '%' operator
			std::make_pair("trunc", std::make_tuple(new func(truncl), "Truncate a floating-point number.")),
			std::make_pair("round", std::make_tuple(new func(roundl), "Round a number to the nearest integral.")),
			std::make_pair("remainder", std::make_tuple(new func(remainderl), "Get remainder of division operation.")),
			/// Floating-point Manipulation Functions
			std::make_pair("copysign", std::make_tuple(new func(copysignl), "")),
			std::make_pair("nextafter", std::make_tuple(new func(nextafterl), "")),
			std::make_pair("nexttoward", std::make_tuple(new func(nexttowardl), "")),
			/// Minimum, Maximum, & Difference Functions
			//std::make_pair("dim", std::make_tuple(new func( fdiml ), "")), //< weird
			std::make_pair("max", std::make_tuple(new func(std::function{ [](Number const& a, Number const& b) -> Number {
				if (a.has_integral_value() && b.has_integral_value()) {
					return imax(a.cast_to<int64_t>(), b.cast_to<int64_t>());
				}
				else return fmaxl(a.cast_to<long double>(), b.cast_to<long double>());
			} }), "Get Larger Value")),
			std::make_pair("min", std::make_tuple(new func(std::function{ [](Number const& a, Number const& b) -> Number {
				if (a.has_integral_value() && b.has_integral_value()) {
					return imin(a.cast_to<int64_t>(), b.cast_to<int64_t>());
				}
				else return fminl(a.cast_to<long double>(), b.cast_to<long double>());
			} }), "Get Smaller Value")),
			/// Other
			std::make_pair("abs", std::make_tuple(new func(std::function{ [](Number const& n) -> Number {
				if (n.has_integral_value()) {
					return llabs(n.cast_to<long long>());
				}
				else return fabsl(n.cast_to<long double>());
			} }), "Get Absolute Value")),
			std::make_pair("fma", std::make_tuple(new func(fmal), "x * y + z.")),

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

		friend std::ostream& operator<<(std::ostream& os, const FunctionMap& fnMap)
		{
			// get the longest name length
			auto maxNameLen{ 0 };
			auto maxDescLen{ 0 };
			for (const auto& [name, val] : fnMap.map) {
				if (const auto nameSize{ name.size() }; nameSize > maxNameLen)
					maxNameLen = nameSize;
				if (const auto descSize{ std::get<1>(val).size() }; descSize > maxDescLen)
					maxDescLen = descSize;
			}
			// increment both max lengths to get column widths - 1
			if (++maxNameLen < 9)
				maxNameLen = 9;
			if (++maxDescLen < 11)
				maxDescLen = 11;

			os	// line 0:
				<< "| Function " << indent(maxNameLen, 9)
				<< "| Params "
				<< "| Description" << indent(maxDescLen, 11)
				<< '|' << '\n'
				// line 1:
				<< '|' << indent(maxNameLen + 1, 0, '-')
				<< '|' << indent(8, 0, '-')
				<< '|' << indent(maxDescLen + 1, 0, '-')
				<< '|' << '\n';

			for (const auto& [name, val] : fnMap.map) {
				const auto& [func, desc] { val };
				const auto paramsCountString{ str::stringify(func->getParamsCount()) };
				os	// line n:
					<< "| " << name << indent(maxNameLen, name.size())
					<< "| " << paramsCountString << indent(7, paramsCountString.size())
					<< "| " << desc << indent(maxDescLen, desc.size())
					<< '|' << '\n';
			}
			return os;
		}
	};
}
