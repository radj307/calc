#pragma once
#include "tokenizer/token.hpp"
#include "OperatorPrecedence.hpp"
#include "FunctionMap.hpp"

// libcalc
#include <Number.hpp>	//< for calc::Number

// 307lib
#include <strcore.hpp>	//< for str::tonumber

// stl
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

	inline Number evaluate_rpn(std::vector<tkn::primitive> const& rpn_expression, FunctionMap const& fnMap)
	{
		std::stack<Number> operands;
		bool atLeastOneOperation{ false };

		for (auto const& tkn : rpn_expression) {
			if (primitiveTypeIsNumber(tkn.type)) {
				// Number Type
				operands.push(primitiveToNumber(tkn));
			}
			else if (tkn.type == PrimitiveTokenType::FunctionName && fnMap.isFunction(tkn.text)) {
				// Function Type
				const auto* const func{ fnMap.get(tkn.text) };
				if (func == nullptr) // this *shouldn't* be possible, since fnMap.isFunction checks if it exists
					throw make_exception("evaluate_rpn() failed to retrieve a function pointer for function name \"", tkn.text, "\"");
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
			else if (OperatorPrecedence::IsOperator(tkn.type)) {
				// Operator Type
				// get right operand
				if (operands.empty())
					throw make_exception("Operator ", PrimitiveTokenTypeNames[(int)tkn.type], " (\"", tkn.text, "\") takes 2 operands; 0 were provided.");
				const Number right{ operands.top() };
				operands.pop();

				// get left operand
				if (operands.empty())
					throw make_exception("Operator ", PrimitiveTokenTypeNames[(int)tkn.type], " (\"", tkn.text, "\") takes 2 operands; 1 was provided.");
				const Number left{ operands.top() };
				operands.pop();

				// evaluate result
				Number result{};
				switch (tkn.type) {
				case PrimitiveTokenType::Add:
					result = (left + right);
					break;
				case PrimitiveTokenType::Subtract:
					result = (left - right);
					break;
				case PrimitiveTokenType::Multiply:
					result = (left * right);
					break;
				case PrimitiveTokenType::Divide:
					result = (left / right);
					break;
				case PrimitiveTokenType::Modulo:
					result = (left % right);
					break;
				case PrimitiveTokenType::Exponent:
					// doesn't work (?):
					result = (*fnMap.get("pow"))(left, right);
					break;
				case PrimitiveTokenType::BitNOT:
					// Unary operator (?)
					break;
				case PrimitiveTokenType::BitOR:
					result = left | right;
					break;
				case PrimitiveTokenType::BitAND:
					result = left & right;
					break;
				case PrimitiveTokenType::BitXOR:
					result = left ^ right;
					break;
					// TODO: implement remaining operators
				default:
					throw make_exception("Operator \"", PrimitiveTokenTypeNames[(int)tkn.type], "\" is not currently supported.");
				}

				operands.push(result);
				atLeastOneOperation = true;
			}
			else throw make_exception("Token type \"", PrimitiveTokenTypeNames[(int)tkn.type], "\" is not currently supported!");
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
