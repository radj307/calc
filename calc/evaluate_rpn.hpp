#pragma once
// calc
#include "tokenizer/token.hpp"
#include "OperatorPrecedence.hpp"
#include "FunctionMap.hpp"
#include "VarMap.hpp"
#include "stack_helpers.h"

// libcalc
#include <Number.hpp>	//< for calc::Number

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
			return str::tonumber<Number::int_t>(primitive.text.starts_with("0b")
												? primitive.text.substr(2)
												: primitive.text, 2);
		case PrimitiveTokenType::OctalNumber:
			// Octal Integral
			return str::tonumber<Number::int_t>(primitive.text, 8);
		case PrimitiveTokenType::HexNumber:
			// Hexadecimal Integral
			return str::tonumber<Number::int_t>(primitive.text.starts_with("0x")
												? primitive.text.substr(2)
												: primitive.text, 16);
		case PrimitiveTokenType::IntNumber:
			// Decimal Integral
			return str::tonumber<Number::int_t>(stripWhitespace(primitive.text), 10);
		case PrimitiveTokenType::RealNumber:
			// Decimal Floating-Point
			return str::tonumber<Number::real_t>(stripWhitespace(primitive.text), std::chars_format::fixed);
		default:
			// Unsupported
			throw make_exception("primitiveToNumber() does not support converting type \"", PrimitiveTokenTypeNames[(int)primitive.type], "\" to Number!");
		}
	}

	inline Number evaluate_rpn(std::vector<tkn::primitive> const& rpn_expression, FunctionMap const& fnMap, VarMap& vars)
	{
		std::stack<Number> operands;
		bool atLeastOneOperation{ false };

		for (auto it{ rpn_expression.begin() }, it_end{ rpn_expression.end() };
			 it != it_end;
			 ++it) {
			const auto tkn{ *it };
			if (tkn.type == PrimitiveTokenType::FunctionName && fnMap.isFunction(tkn.text)) {
				// Function Type
				const auto* const func{ fnMap.get(tkn.text) };
				if (func == nullptr) // this *shouldn't* be possible, since fnMap.isFunction checks if it exists
					throw make_exception("evaluate_rpn() failed to retrieve a valid function pointer for \"", tkn.text, "\"; this is a bug, please report it!");
				const auto paramsCount{ func->getParamsCount() };

				// pop the required number of parameters from the stack
				std::vector<Number> params;
				params.reserve(paramsCount);
				for (auto i{ 0 }; !operands.empty() && i < paramsCount; ++i) {
					params.emplace_back(operands.top());
					operands.pop();
				}
				if (params.size() < paramsCount) {
					throw make_exception("Function \"", tkn.text, "\" takes ", paramsCount, " operands, but only ", params.size(), (params.size() == 1 ? " was" : " were"), " provided!", str::stringify_if([&params]() { return !params.empty(); }, " (\"", str::stringify_join(params.begin(), params.end(), "\", \""), "\")"));
				}
				// reverse the operands so they're in the correct order for the function call
				std::reverse(params.begin(), params.end());

				// get the result of the function
				Number result;
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

				// push the function result to the operand stack
				operands.push(result);
				atLeastOneOperation = true;
			}
			else {
				// evaluate result
				Number result{};
				/*/
				 * NOTE
				 * ====
				 * Pop the right-most operand first!
				/*/
				switch (tkn.type) {
				case PrimitiveTokenType::BinaryNumber:
				case PrimitiveTokenType::OctalNumber:
				case PrimitiveTokenType::HexNumber:
				case PrimitiveTokenType::IntNumber:
				case PrimitiveTokenType::RealNumber:
					result = primitiveToNumber(tkn);
					break;
				case PrimitiveTokenType::Variable:
					if (vars.isDefined(tkn.text))
						result = vars[tkn.text];
					else throw make_exception("Variable \"", tkn.text, "\" is undefined!");
					break;
				case PrimitiveTokenType::Add: {
					const auto right{ pop_off(operands) };
					result = pop_off(operands) + right;
					break;
				}
				case PrimitiveTokenType::Subtract: {
					const auto right{ pop_off(operands) };
					result = pop_off(operands) - right;
					break;
				}
				case PrimitiveTokenType::Multiply: {
					const auto right{ pop_off(operands) };
					result = pop_off(operands) * right;
					break;
				}
				case PrimitiveTokenType::Divide: {
					const auto right{ pop_off(operands) };
					result = pop_off(operands) / right;
					break;
				}
				case PrimitiveTokenType::Modulo: {
					const auto right{ pop_off(operands) };
					result = pop_off(operands) % right;
					break;
				}
				case PrimitiveTokenType::Exponent: {
					// doesn't work (?):
					const auto right{ pop_off(operands) };
					result = (*fnMap.get("pow"))(pop_off(operands), right);
					break;
				}
				case PrimitiveTokenType::BitNOT: {
					result = ~pop_off(operands);
					break;
				}
				case PrimitiveTokenType::BitOR: {
					const auto right{ pop_off(operands) };
					result = pop_off(operands) | right;
					break;
				}
				case PrimitiveTokenType::BitAND: {
					const auto right{ pop_off(operands) };
					result = pop_off(operands) & right;
					break;
				}
				case PrimitiveTokenType::BitXOR: {
					const auto right{ pop_off(operands) };
					result = pop_off(operands) ^ right;
					break;
				}
				default:
					throw make_exception("Operator \"", PrimitiveTokenTypeNames[(int)tkn.type], "\" is not currently supported.");
				}

				operands.push(result);
				atLeastOneOperation = true;
			}
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
