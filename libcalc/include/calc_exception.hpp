#pragma once
// 307lib
#include <make_exception.hpp>	//< for ex::except
#include <indentor.hpp>			//< for indent

// STL
#include <map>			//< for std::map
#include <utility>		//< for std::make_pair
#include <string>		//< for std::string
#include <sstream>		//< for std::stringstream

namespace calc {
#define SUGGESTED_FIX			\
X(0, None, "") \
X(1, SmallerNumbers, "Try using smaller numbers in your expression.")	\
X(2, UnsafeCast, "Disable unsafe cast exceptions by specifying the '-E|--castex' option.")	\
X(4, RoundFloat, "Convert the floating-point to an integer with round(), trunc(), ceil(), or floor().")	\
X(8, EncloseExprInQuotes, "Enclose the expression with double-quotes (\").") \
X(16, IncludeArgTerminator, "Include an argument terminator (--) prior to the expression.") \


	// define the enum
#define X(value, name, desc) name = value,
	/// @brief	Defines possible fixes for various issues.
	enum class SuggestedFix {
		SUGGESTED_FIX
	};
#undef X
	inline SuggestedFix operator|(SuggestedFix const l, SuggestedFix const r)
	{
		return static_cast<SuggestedFix>(static_cast<int>(l) | static_cast<int>(r));
	}
	inline int operator&(SuggestedFix const l, SuggestedFix const r)
	{
		return static_cast<int>(l) & static_cast<int>(r);
	}

	// define the map
#define X(value, name, desc) std::make_pair(SuggestedFix::name, desc),
	static const std::map<SuggestedFix, std::string> suggested_fixes{
		SUGGESTED_FIX
	};
#undef X

#define X(value, name, desc) \
	if (suggestedFixes & SuggestedFix::name) { \
		ss << indent(10) << "- " << suggested_fixes.at(SuggestedFix::name) << '\n';	\
	}
	inline std::string make_suggested_fix_message(SuggestedFix const suggestedFixes)
	{
		if (suggestedFixes == SuggestedFix::None)
			return{};

		std::stringstream ss;
		ss << indent(10) << "Suggested Fixes:\n";

		SUGGESTED_FIX;

		return ss.str();
	}
#undef X

#undef SUGGESTED_FIX

	struct calc_exception : ex::except {
		calc_exception(std::string const& message, SuggestedFix const suggestedFixes) :
			ex::except(str::stringify(message, '\n', make_suggested_fix_message(suggestedFixes)))
		{}
	};
}
