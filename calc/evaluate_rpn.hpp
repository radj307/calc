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
	/**
	 * @brief				Converts the specified primitive token to a Number.
	 * @param primitive	  -	A primitive token containing a supported numeric type.
	 * @returns				The equivalent Number.
	 */
	inline Number primitiveToNumber(tkn::primitive const& primitive)
	{
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
			return str::tonumber<Number::int_t>(primitive.text, 10);
		case PrimitiveTokenType::RealNumber:
			// Decimal Floating-Point
			return str::tonumber<Number::real_t>(primitive.text, std::chars_format::fixed);
		default:
			// Unsupported
			throw make_exception("primitiveToNumber() does not support converting type \"", PrimitiveTokenTypeNames[(int)primitive.type], "\" to Number!");
		}
	}

	inline Number evaluate_rpn(std::vector<tkn::primitive> const& rpn_expression, FunctionMap const& fnMap)
	{
		std::stack<Number> operands;

		for (auto const& tkn : rpn_expression) {
			if (primitiveTypeIsNumber(tkn.type)) {
				operands.push(primitiveToNumber(tkn));
			}
			else if (tkn.type == PrimitiveTokenType::FunctionName && fnMap.isFunction(tkn.text)) {
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
			}
			else if (OperatorPrecedence::IsOperator(tkn.type)) {
				// get right operand
				if (operands.empty())
					throw make_exception("Out of operands!");
				const Number right{ operands.top() };
				operands.pop();

				// get left operand
				if (operands.empty())
					throw make_exception("Out of operands!");
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
					// TODO: implement remaining operators
				}

				operands.push(result);
			}
		}

		return operands.top();
	}
}
