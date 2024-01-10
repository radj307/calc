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
	inline std::vector<tkn::primitive> to_rpn(std::vector<tkn::primitive> const& primitives)
	{
		std::vector<tkn::primitive> result; //< queue
		result.reserve(primitives.size());

		std::stack<tkn::primitive> operators;

		for (auto const& tkn : primitives) {
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
