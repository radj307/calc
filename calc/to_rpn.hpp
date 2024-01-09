#pragma once
// calc
#include "tokenizer/token.hpp"
#include "OperatorPrecedence.hpp"

// STL
#include <stack>
#include <map>

namespace calc::expr {

	/**
	 * @brief				Converts the specified primitives to an expression in reverse polish notation.
	 * @param primitives  -	Vector of primitive tokens to convert to an expression in RPN.
	 * @returns				A vector of primitives in RPN, with unnecessary tokens discarded.
	 */
	inline std::vector<tkn::primitive> to_rpn(std::vector<tkn::primitive> const& primitives)
	{
		std::vector<tkn::primitive> result;
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
					result.emplace_back(operators.top());
					operators.pop();

					if (operators.empty())
						throw make_exception("Missing opening bracket!");
				}

				if (operators.empty())
					throw make_exception("Invalid brackets!");

				// pop the ExpressionOpen token that terminated the while loop
				operators.pop();

				// if this was a function, push the function name
				if (!operators.empty() && operators.top().type == PrimitiveTokenType::FunctionName) {
					result.emplace_back(operators.top());
					operators.pop();
				}
				break;
			case PrimitiveTokenType::TermSeparator:
				break; //< don't add term separators (',') to the result
			default: {
				// operators:
				const auto tknPrecedence{ OperatorPrecedence::Get(tkn.type) };

				// pop all of the lower-precedence operators into the result (if any)
				while (!operators.empty() && OperatorPrecedence::Get(operators.top().type) >= tknPrecedence) {
					result.emplace_back(operators.top());
					operators.pop();
				}
				// push the higher-precedence operator
				operators.push(tkn);
				break;
			}
			}
		}

		while (!operators.empty()) {
			result.emplace_back(operators.top());
			operators.pop();
		}

		return result;
	}
}
