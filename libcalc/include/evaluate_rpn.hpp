#pragma once
// calc
#include "tokenizer/token.hpp"
#include "OperatorPrecedence.hpp"
#include "FunctionMap.hpp"
#include "VarMap.hpp"
#include "helpers/stack_helpers.h"	//< for pop_off

// libcalc
#include <Number.hpp>	//< for calc::Number
#include <baseconv.h>	//< for calc::from_base

// 307lib
#include <strcore.hpp>	//< for str::tonumber

// STL
#include <stack>		//< for std::stack

namespace calc::expr {
	inline std::string stripWhitespace(std::string str)
	{
		str.erase(std::remove_if(str.begin(), str.end(), str::stdpred::isspace), str.end());
		return str;
	}

	/**
	 * @brief				Converts the specified primitive token to a Number.
	 * @param primitive	  -	A primitive token containing a supported numeric type.
	 * @returns				The equivalent Number.
	 */
	inline Number primitiveToNumber(tkn::primitive const& primitive)
	{
		std::string text{ primitive.text };
		text.erase(std::remove_if(text.begin(), text.end(), [](auto&& c) { return str::stdpred::isspace(c) || c == '_'; }), text.end());
		switch (primitive.type) {
		case PrimitiveTokenType::BinaryNumber:
			// Binary Integral
			return from_base(text.starts_with("0b")
							 ? text.substr(2)
							 : text, 2);
		case PrimitiveTokenType::OctalNumber:
			// Octal Integral
			return from_base(text, 8);
		case PrimitiveTokenType::HexNumber:
			// Hexadecimal Integral
			return from_base(text.starts_with("0x")
							 ? text.substr(2)
							 : text, 16);
		case PrimitiveTokenType::IntNumber:
		case PrimitiveTokenType::RealNumber:
			// Decimal Integral/Floating-Point
			return from_base(text, 10);
		default:
			// Unsupported
			throw make_exception("primitiveToNumber() does not support converting type \"", PrimitiveTokenTypeNames[(int)primitive.type], "\" to Number!");
		}
	}

	/**
	 * @brief					Evaluates the result of the specified rpn_expression.
	 * @param rpn_expression  -	A tokenized expression in reverse polish notation. (See calc::expr::to_rpn)
	 * @param fnMap			  -	FunctionMap to use for calling functions.
	 * @param vars			  -	VarMap instance to use when getting the values of variables.
	 * @returns					The result of the specified RPN expression.
	 */
	inline Number evaluate_rpn(std::vector<tkn::primitive> const& rpn_expression, FunctionMap const& fnMap, VarMap& vars)
	{
		std::stack<Number> operands;
		bool atLeastOneOperation{ false };

		for (auto it{ rpn_expression.begin() }, it_end{ rpn_expression.end() };
			 it != it_end;
			 ++it) {
			const auto tkn{ *it };
			// evaluate result
			Number result{};
			if (is_number(tkn.type)) {
				result = primitiveToNumber(tkn);
			}
			else if (tkn.type == PrimitiveTokenType::Variable) {
				if (vars.isDefined(tkn.text))
					result = vars[tkn.text];
				else throw make_exception("Variable \"", tkn.text, "\" is undefined!");
			}
			else {
				/*/
				 * NOTE
				 * ====
				 * Pop the right-most operand first!
				/*/
				switch (tkn.type) {
				case PrimitiveTokenType::FunctionName: {
					// Call the function
					auto const* const func{ fnMap.get(tkn.text) };
					const auto paramsCount{ func->getParamsCount() };

					// pop the required number of parameters from the stack
					std::vector<Number> params;
					params.reserve(paramsCount);
					for (auto i{ 0 }; !operands.empty() && i < paramsCount; ++i) {
						params.emplace_back(pop_off(operands));
					}
					if (params.size() < paramsCount) {
						throw make_exception("Function \"", tkn.text, "\" takes ", paramsCount, " operands, but only ", params.size(), (params.size() == 1 ? " was" : " were"), " provided!", str::stringify_if([&params]() { return !params.empty(); }, " (\"", str::stringify_join(params.begin(), params.end(), "\", \""), "\")"));
					}

					// reverse the operands so they're in the correct order for the function call
					std::reverse(params.begin(), params.end());

					// get the result of the function
					try {
						result = func->invoke(params);
					} catch (const std::exception& ex) {
						throw make_exception("An exception was thrown by function \"",
											 tkn.text,
											 "\" with params \"",
											 str::stringify_join(params.begin(), params.end(), "\", \""),
											 "\":\n",
											 indent(10),
											 ex.what());
					} catch (...) {
						throw make_exception("An undefined exception was thrown by function \"",
											 tkn.text,
											 "\" with params \"",
											 str::stringify_join(params.begin(), params.end(), "\", \""),
											 "\"");
					}
					break;
				}
				case PrimitiveTokenType::Factorial: {
					if (operands.size() < 1) throw make_exception("Not enough operands for unary operator ! (Factorial)");
					const auto operand{ pop_off(operands) };

					if (!operand.has_integral_value() || (!operand.is_positive() && !operand.is_zero()))
						throw make_exception("Operator ! (Factorial) requires a positive integer!");

					result = factorial(operand.cast_to<typename Number::int_t>());
					break;
				}
				case PrimitiveTokenType::Exponent: {
					if (operands.size() < 2) throw make_exception("Not enough operands for binary operator ^ (Exponent)");
					const auto right{ pop_off(operands) };
					result = fnMap.invoke("pow", pop_off(operands), right);
					break;
				}
				case PrimitiveTokenType::Negate: {
					if (operands.size() < 1) throw make_exception("Not enough operands for unary operator - (Negate)");
					result = -pop_off(operands);
					break;
				}
				case PrimitiveTokenType::Add: {
					if (operands.size() < 2) throw make_exception("Not enough operands for binary operator + (Add)");
					const auto right{ pop_off(operands) };
					result = pop_off(operands) + right;
					break;
				}
				case PrimitiveTokenType::Subtract: {
					if (operands.size() < 2) throw make_exception("Not enough operands for binary operator - (Subtract)");
					const auto right{ pop_off(operands) };
					result = pop_off(operands) - right;
					break;
				}
				case PrimitiveTokenType::Multiply: {
					if (operands.size() < 2) throw make_exception("Not enough operands for binary operator * (Multiply)");
					const auto right{ pop_off(operands) };
					result = pop_off(operands) * right;
					break;
				}
				case PrimitiveTokenType::Divide: {
					if (operands.size() < 2) throw make_exception("Not enough operands for binary operator / (Divide)");
					const auto right{ pop_off(operands) };
					if (right.is_zero()) throw make_exception("Cannot divide by zero!");
					result = pop_off(operands) / right;
					break;
				}
				case PrimitiveTokenType::Modulo: {
					if (operands.size() < 2) throw make_exception("Not enough operands for binary operator % (Modulo)");
					const auto right{ pop_off(operands) };
					if (right.is_zero()) throw make_exception("Cannot divide by zero!");
					result = pop_off(operands) % right;
					break;
				}
				case PrimitiveTokenType::BitNOT: {
					if (operands.size() < 1) throw make_exception("Not enough operands for unary operator ~ (BitwiseNOT)");
					result = ~pop_off(operands);
					break;
				}
				case PrimitiveTokenType::BitOR: {
					if (operands.size() < 2) throw make_exception("Not enough operands for binary operator | (BitwiseOR)");
					const auto right{ pop_off(operands) };
					result = pop_off(operands) | right;
					break;
				}
				case PrimitiveTokenType::BitAND: {
					if (operands.size() < 2) throw make_exception("Not enough operands for binary operator & (BitwiseAND)");
					const auto right{ pop_off(operands) };
					result = pop_off(operands) & right;
					break;
				}
				case PrimitiveTokenType::BitXOR: {
					if (operands.size() < 2) throw make_exception("Not enough operands for binary operator ^ (BitwiseXOR)");
					const auto right{ pop_off(operands) };
					result = pop_off(operands) ^ right;
					break;
				}
				case PrimitiveTokenType::BitshiftLeft: {
					if (operands.size() < 2) throw make_exception("Not enough operands for binary operator << (BitshiftLeft)");
					const auto right{ pop_off(operands) };
					result = pop_off(operands) << right;
					break;
				}
				case PrimitiveTokenType::BitshiftRight: {
					if (operands.size() < 2) throw make_exception("Not enough operands for binary operator >> (BitshiftRight)");
					const auto right{ pop_off(operands) };
					result = pop_off(operands) >> right;
					break;
				}
				case PrimitiveTokenType::Equal: {
					if (operands.size() < 2) throw make_exception("Not enough operands for binary operator == (", PrimitiveTokenTypeNames[(int)PrimitiveTokenType::Equal], ')');
					const auto right{ pop_off(operands) };
					result = pop_off(operands) == right;
					break;
				}
				case PrimitiveTokenType::NotEqual: {
					if (operands.size() < 2) throw make_exception("Not enough operands for binary operator != (", PrimitiveTokenTypeNames[(int)PrimitiveTokenType::NotEqual], ')');
					const auto right{ pop_off(operands) };
					result = pop_off(operands) != right;
					break;
				}
				case PrimitiveTokenType::LessThan: {
					if (operands.size() < 2) throw make_exception("Not enough operands for binary operator < (", PrimitiveTokenTypeNames[(int)PrimitiveTokenType::LessThan], ')');
					const auto right{ pop_off(operands) };
					result = pop_off(operands) < right;
					break;
				}
				case PrimitiveTokenType::LessOrEqual: {
					if (operands.size() < 2) throw make_exception("Not enough operands for binary operator <= (", PrimitiveTokenTypeNames[(int)PrimitiveTokenType::LessOrEqual], ')');
					const auto right{ pop_off(operands) };
					result = pop_off(operands) <= right;
					break;
				}
				case PrimitiveTokenType::GreaterThan: {
					if (operands.size() < 2) throw make_exception("Not enough operands for binary operator > (", PrimitiveTokenTypeNames[(int)PrimitiveTokenType::GreaterThan], ')');
					const auto right{ pop_off(operands) };
					result = pop_off(operands) > right;
					break;
				}
				case PrimitiveTokenType::GreaterOrEqual: {
					if (operands.size() < 2) throw make_exception("Not enough operands for binary operator >= (", PrimitiveTokenTypeNames[(int)PrimitiveTokenType::GreaterOrEqual], ')');
					const auto right{ pop_off(operands) };
					result = pop_off(operands) >= right;
					break;
				}
				case PrimitiveTokenType::LogicalNOT: {
					if (operands.size() < 1) throw make_exception("Not enough operands for unary operator ! (", PrimitiveTokenTypeNames[(int)PrimitiveTokenType::LogicalNOT], ')');
					result = !pop_off(operands);
					break;
				}
				case PrimitiveTokenType::LogicalOR: {
					if (operands.size() < 2) throw make_exception("Not enough operands for binary operator || (", PrimitiveTokenTypeNames[(int)PrimitiveTokenType::LogicalOR], ')');
					const auto right{ pop_off(operands) };
					result = pop_off(operands) || right;
					break;
				}
				case PrimitiveTokenType::LogicalAND: {
					if (operands.size() < 2) throw make_exception("Not enough operands for binary operator && (", PrimitiveTokenTypeNames[(int)PrimitiveTokenType::LogicalAND], ')');
					const auto right{ pop_off(operands) };
					result = pop_off(operands) && right;
					break;
				}
				default:
					throw make_exception("Operator \"", PrimitiveTokenTypeNames[(int)tkn.type], "\" is not implemented yet.");
				}

				atLeastOneOperation = true;
			}

			operands.push(result);
		}

		if (operands.size() > 1) {
			// too many operands; throw an exception
			if (atLeastOneOperation) {
				const calc::Number result{ operands.top() };
				operands.pop();
				const auto unmatchedCount{ operands.size() };
				std::stringstream ss;
				for (bool fst{ true }; !operands.empty(); operands.pop()) {
					if (fst) fst = false;
					else ss << "\", \"";
					ss << operands.top();
				}
				throw make_exception("Expression evaluated to \"", result, "\", but there were ", unmatchedCount, " unmatched operands: \"", ss.str(), "\"!");
			}
			else {
				const auto count{ operands.size() };
				std::stringstream ss;
				for (bool fst{ true }; !operands.empty(); operands.pop()) {
					if (fst) fst = false;
					else ss << "\', \'";
					ss << operands.top();
				}
				throw make_exception("No operators were specified, but the expression contained ", count, " operands: \'", ss.str(), "\'!");
			}
		}
		else if (operands.empty())
			throw make_exception("Invalid expression! (No operands)");
		// success
		return operands.top();
	}
}
