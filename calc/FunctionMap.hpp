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
	struct FunctionMap {
		// See https://cplusplus.com/reference/cmath/
		std::map<std::string, std::tuple<basic_operator*, std::string>> map{
			/// Trigonometric Functions
			std::make_pair("cos", std::make_tuple((basic_operator*)new singletype_operator{ cosl }, "Cosine")),
			std::make_pair("sin", std::make_tuple((basic_operator*)new singletype_operator{ sinl }, "Sine")),
			std::make_pair("tan", std::make_tuple((basic_operator*)new singletype_operator{ tanl }, "Tangent")),
			std::make_pair("acos", std::make_tuple((basic_operator*)new singletype_operator{ acosl }, "Inverse Cosine")),
			std::make_pair("asin", std::make_tuple((basic_operator*)new singletype_operator{ asinl }, "Inverse Sine")),
			std::make_pair("atan", std::make_tuple((basic_operator*)new singletype_operator{ atanl }, "Inverse Tangent")),
			std::make_pair("atan2", std::make_tuple((basic_operator*)new singletype_operator{ atan2l }, "Binary Tangent")),
			/// Hyperbolic Functions
			std::make_pair("cosh", std::make_tuple((basic_operator*)new singletype_operator{ coshl }, "Hyperbolic Cosine")),
			std::make_pair("sinh", std::make_tuple((basic_operator*)new singletype_operator{ sinhl }, "Hyperbolic Sine")),
			std::make_pair("tanh", std::make_tuple((basic_operator*)new singletype_operator{ tanhl }, "Hyperbolic Tangent")),
			std::make_pair("acosh", std::make_tuple((basic_operator*)new singletype_operator{ acoshl }, "Inverse Hyperbolic Cosine")),
			std::make_pair("asinh", std::make_tuple((basic_operator*)new singletype_operator{ asinhl }, "Inverse Hyperbolic Sine")),
			std::make_pair("atanh", std::make_tuple((basic_operator*)new singletype_operator{ atanhl }, "Inverse Hyperbolic Tangent")),
			/// Exponential and Logarithmic Functions
			std::make_pair("exp", std::make_tuple((basic_operator*)new singletype_operator{ expl }, "")),
			//std::make_pair("frexp", std::make_tuple((basic_operator*)new singletype_operator{ frexpl }, "")), //< uses pointers, must be adapted
			std::make_pair("ldexp", std::make_tuple((basic_operator*)new singletype_operator{ ldexpl }, "")),
			std::make_pair("log", std::make_tuple((basic_operator*)new singletype_operator{ logl }, "")),
			std::make_pair("log10", std::make_tuple((basic_operator*)new singletype_operator{ log10l }, "")),
			//std::make_pair("modf", std::make_tuple((basic_operator*)new singletype_operator{ modfl }, "")), //< uses pointers, must be adapted
			std::make_pair("exp2", std::make_tuple((basic_operator*)new singletype_operator{ exp2l }, "")),
			std::make_pair("expm1", std::make_tuple((basic_operator*)new singletype_operator{ expm1l }, "")),
			std::make_pair("ilogb", std::make_tuple((basic_operator*)new singletype_operator{ ilogbl }, "")),
			std::make_pair("log1p", std::make_tuple((basic_operator*)new singletype_operator{ log1pl }, "")),
			std::make_pair("log2", std::make_tuple((basic_operator*)new singletype_operator{ log2l }, "")),
			std::make_pair("logb", std::make_tuple((basic_operator*)new singletype_operator{ logbl }, "")),
			std::make_pair("scalbn", std::make_tuple((basic_operator*)new singletype_operator{ scalbnl }, "")),
			std::make_pair("scalbln", std::make_tuple((basic_operator*)new singletype_operator{ scalblnl }, "")),
			/// Power Functions
			std::make_pair("pow", std::make_tuple((basic_operator*)new singletype_operator{ powl }, "Calculates the result of an exponent.")),
			//std::make_pair("pow", std::make_tuple((basic_operator*)new dualtype_operator{ ipow, powl }, "Calculates the result of an exponent.")),
			std::make_pair("sqrt", std::make_tuple((basic_operator*)new singletype_operator{ sqrtl }, "Calculates a square root.")),
			std::make_pair("cbrt", std::make_tuple((basic_operator*)new singletype_operator{ cbrtl }, "Calculates a cubic root.")),
			std::make_pair("hypot", std::make_tuple((basic_operator*)new singletype_operator{ hypotl }, "Calculate hypotenuse.")),
			/// Error & Gamma Functions
			std::make_pair("erf", std::make_tuple((basic_operator*)new singletype_operator{ erfl }, "")),
			std::make_pair("erfc", std::make_tuple((basic_operator*)new singletype_operator{ erfcl }, "")),
			std::make_pair("tgamma", std::make_tuple((basic_operator*)new singletype_operator{ tgammal }, "")),
			std::make_pair("lgamma", std::make_tuple((basic_operator*)new singletype_operator{ lgammal }, "")),
			/// Rounding & Remainder Functions
			std::make_pair("ceil", std::make_tuple((basic_operator*)new singletype_operator{ ceill }, "Raises a number to the nearest integral.")),
			std::make_pair("floor", std::make_tuple((basic_operator*)new singletype_operator{ floorl }, "Lowers a number to the nearest integral.")),
			std::make_pair("fmod", std::make_tuple((basic_operator*)new singletype_operator{ fmodl }, "")), //< implemented as '%' operator
			std::make_pair("trunc", std::make_tuple((basic_operator*)new singletype_operator{ truncl }, "Truncate a floating-point number.")),
			std::make_pair("round", std::make_tuple((basic_operator*)new singletype_operator{ roundl }, "Round a number to the nearest integral.")),
			std::make_pair("remainder", std::make_tuple((basic_operator*)new singletype_operator{ remainderl }, "Get remainder of division operation.")),
			/// Floating-point Manipulation Functions
			std::make_pair("copysign", std::make_tuple((basic_operator*)new singletype_operator{ copysignl }, "")),
			//std::make_pair("nan", std::make_tuple((basic_operator*)new singletype_operator{ nanl }, "")), //< requires a char*
			std::make_pair("nextafter", std::make_tuple((basic_operator*)new singletype_operator{ nextafterl }, "")),
			std::make_pair("nexttoward", std::make_tuple((basic_operator*)new singletype_operator{ nexttowardl }, "")),
			/// Minimum, Maximum, & Difference Functions
			//std::make_pair("dim", std::make_tuple((basic_operator*)new singletype_operator{ fdiml }, "")), //< weird
			std::make_pair("max", std::make_tuple((basic_operator*)new dualtype_operator{ imax, fmaxl }, "Get Larger Value")),
			std::make_pair("min", std::make_tuple((basic_operator*)new dualtype_operator{ imin, fminl }, "Get Smaller Value")),
			/// Other
			std::make_pair("abs", std::make_tuple((basic_operator*)new dualtype_operator{ llabs, fabsl }, "Get Absolute Value")),
			std::make_pair("fma", std::make_tuple((basic_operator*)new singletype_operator{ fmal }, "x * y + z.")),
		};

		~FunctionMap()
		{
			// delete the pointers in the map
			for (auto& pr : map) {
				auto& [func, _] { pr.second };
				if (func != nullptr) {
					delete func;
					func = nullptr;
				}
			}
		}

		/**
		 * @brief					Gets a pointer to the function operator with the specified name.
		 * @param functionName	  -	The name of the function to retrieve.
		 * @returns					A const operator pointer when successful; otherwise, nullptr.
		 */
		basic_operator const* const get(std::string const& functionName) const noexcept
		{
			if (const auto& it{ map.find(functionName) }; it != map.end())
				return std::get<0>(it->second);
			return nullptr;
		}

		/// @brief	Determines whether the specified name is a function or not.
		inline bool isFunction(std::string const& name) const noexcept
		{
			return map.contains(name);
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

			os
				<< "| Function " << indent(maxNameLen, 9) << "| Params " << "| Description" << indent(maxDescLen, 11) << '|' << '\n'
				<< '|' << indent(maxNameLen + 1, 0, '-') << '|' << indent(8, 0, '-') << '|' << indent(maxDescLen + 1, 0, '-') << '|' << '\n';

			for (const auto& [name, val] : fnMap.map) {
				const auto& [func, desc] { val };
				const auto paramsCountString{ str::stringify(func->getParamsCount()) };
				os << "| " << name << indent(maxNameLen, name.size()) << "| " << paramsCountString << indent(7, paramsCountString.size()) << "| " << desc << indent(maxDescLen, desc.size()) << '|' << '\n';
			}
			return os;
		}
	};

	struct VarMap {
		/// @brief	The numeric value of a variable.
		using value_t = Number;

		/// @brief	Stores variable name-value mappings.
		std::map<std::string, value_t> map;

		/// @brief	Creates a new instance with no variables.
		VarMap() {}
		/// @brief	Creates a new instance with the specified variables.
		VarMap(std::initializer_list<std::pair<std::string, value_t>> variables) : map{ variables.begin(), variables.end() } {}
		/// @brief	Creates a new instance with the variables in the specified range.
		template<std::input_or_output_iterator Iter> VarMap(Iter const& begin, Iter const& end) : map{ begin, end } {}

		auto& operator[](std::string const& name)
		{
			return map[name];
		}

		inline bool isDefined(std::string const& name) const
		{
			return map.contains(name);
		}

		friend std::ostream& operator<<(std::ostream& os, VarMap const& vm)
		{

			return os;
		}
	};
}
