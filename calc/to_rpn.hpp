#pragma once
// calc
#include "tokenizer/token.hpp"
#include "OperatorPrecedence.hpp"
#include "helpers/stack_helpers.h"	//< for pop_off

// STL
#include <stack>	//< for std::stack
#include <map>		//< for std::map

namespace calc::expr {

	/**
	 * @brief				Converts the specified primitives to an expression in reverse polish notation.
	 * @param primitives  -	Vector of primitive tokens to convert to an expression in RPN.
	 * @returns				A vector of primitives in RPN, with unnecessary tokens discarded.
	 */
	inline std::vector<tkn::primitive> to_rpn(std::vector<tkn::primitive> const& primitives, FunctionMap const& fnMap)
	{
		std::vector<tkn::primitive> result; //< queue
		result.reserve(primitives.size());

		std::stack<tkn::primitive> operators;

		for (auto it{ primitives.begin() }, it_end{ primitives.end() }; it != it_end; ++it) {
			const auto& tkn{ *it };
			switch (tkn.type) {
			case PrimitiveTokenType::BinaryNumber: //< BINARY INTEGRAL
			case PrimitiveTokenType::OctalNumber: //< OCTAL INTEGRAL
			case PrimitiveTokenType::HexNumber: //< HEX INTEGRAL
			case PrimitiveTokenType::IntNumber: //< INTEGRAL
			case PrimitiveTokenType::RealNumber: //< FLOATING-POINT
			case PrimitiveTokenType::Variable: //< VARIABLE
				result.emplace_back(tkn);
				break;
			case PrimitiveTokenType::ExpressionOpen: // '('
				operators.push(tkn);
				break;
			case PrimitiveTokenType::ExpressionClose: // ')'
				// pop operators off of the stack until the first ExpressionOpen token
				while (!operators.empty() && operators.top().type != PrimitiveTokenType::ExpressionOpen) {
					result.emplace_back(pop_off(operators));

					if (operators.empty())
						throw make_exception("Encountered an unmatched closing parenthesis!");
				}

				if (operators.empty())
					throw make_exception("Mismatched parentheses!");

				// pop the ExpressionOpen token that terminated the while loop
				operators.pop();

				// if this was a function, push the function name
				if (!operators.empty() && operators.top().type == PrimitiveTokenType::FunctionName) {
					result.emplace_back(pop_off(operators));
				}
				break;
			case PrimitiveTokenType::TermSeparator: {
				// pop operators off of the stack until the first ExpressionOpen token
				//  so we can correctly handle subexpressions within functions. Example:
				//        pow(1 + 2, 2)
				//            \___/  ^
				//              |    |
				//              0    1
				while (!operators.empty() && operators.top().type != PrimitiveTokenType::ExpressionOpen) {
					result.emplace_back(pop_off(operators));
				}
				// ensure an ExpressionOpen operator was reached:
				if (operators.empty() || operators.top().type != PrimitiveTokenType::ExpressionOpen) {
					throw make_exception("Mismatched parentheses or comma found outside function call!");
				}

				break;
			}
			case PrimitiveTokenType::FunctionName: {
				// verify function is called with the correct number of parameters
				const auto paramsCount{ fnMap.getParamsCount(tkn.text) };
				auto count{ 0u };

				// look ahead until the closing function bracket
				unsigned depth{ 0u };
				for (auto fwdit{ it }; fwdit != it_end; ++fwdit) {
					switch (fwdit->type) {
					case PrimitiveTokenType::ExpressionOpen:
						if (++depth == 2)
							++count; //< bracketed subexpression ( ex: "pow(2, (10 + 1))" )
						break;
					case PrimitiveTokenType::ExpressionClose:
						if (--depth == 0)
							goto BREAK_LOOKAHEAD;
						break;
					default:
						if (depth == 1 && evaluates_to_number(fwdit->type))
							++count;
						break;
					}
				}
			BREAK_LOOKAHEAD:

				if (count != paramsCount) // throw on parameter count mismatch:
					throw make_exception("Function \"", tkn.text, "\" expects ", paramsCount, " parameters but ", count, ' ', (count == 1 ? "was" : "were"), " provided!");

				[[fallthrough]];
			}
			default: {
				// operators:
				const auto tknPrecedence{ OperatorPrecedence::Get(tkn.type, -1) };

				if (tknPrecedence == (uint8_t)-1)
					throw make_exception("Token type \"", PrimitiveTokenTypeNames[(int)tkn.type], "\" is not a recognized operator!");

				// pop all of the lower-precedence operators into the result (if any)
				while (!operators.empty()
					   && operators.top().type != PrimitiveTokenType::ExpressionOpen
					   && OperatorPrecedence::Get(operators.top().type) >= tknPrecedence) {
					result.emplace_back(pop_off(operators));
				}
				// push the higher-precedence operator
				operators.push(tkn);
				break;
			}
			}
		}

		while (!operators.empty()) {
			result.emplace_back(pop_off(operators));
		}

		return result;
	}
}
