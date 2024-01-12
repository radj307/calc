#pragma once
#include "tokenizer/types/PrimitiveTokenType.h"
#include "operators.hpp"

#include <map>

namespace calc::expr {
	/**
	 * @brief	Defines the precedence of operators and provides functions for
	 *			 getting the precedence of a specified operator.
	 */
	struct OperatorPrecedence {
		/// @brief	Operator precedence map using [FEDMAS]:
		static const std::map<PrimitiveTokenType, uint8_t> map;

		/**
		 * @brief			Gets the precedence of the specified operator type.
		 * @param opType  -	The type of the operator to get the precedence of.
		 * @param def	  -	Default value when opType doesn't have a defined precedence.
		 * @returns			The precedence of opType when successful; otherwise, the default.
		 */
		static uint8_t Get(PrimitiveTokenType const& opType, uint8_t def = 0)
		{
			if (const auto& it{ map.find(opType) }; it != map.end())
				return it->second;
			return def;
		}

		/**
		 * @brief			Determines if the specified tknType is an operator.
		 * @param tknType -	Primitive token type to check.
		 * @returns			true when tknType is an operator; otherwise, false.
		 */
		static bool IsOperator(PrimitiveTokenType const& tknType)
		{
			return map.contains(tknType);
		}

	private:
		virtual ~OperatorPrecedence() = 0;
	};
	// higher values have higher precedence
	inline const std::map<PrimitiveTokenType, uint8_t> OperatorPrecedence::map{
		// [F] Functions
		std::make_pair(PrimitiveTokenType::Factorial, 6),
		std::make_pair(PrimitiveTokenType::FunctionName, 6),
		// [E] Exponents
		std::make_pair(PrimitiveTokenType::Exponent, 5),
		// Negate
		std::make_pair(PrimitiveTokenType::Negate, 4),
		// [DM] Mult/Div
		std::make_pair(PrimitiveTokenType::Multiply, 3),
		std::make_pair(PrimitiveTokenType::Divide, 3),
		std::make_pair(PrimitiveTokenType::Modulo, 3),
		// [AS] Add/Sub
		std::make_pair(PrimitiveTokenType::Add, 2),
		std::make_pair(PrimitiveTokenType::Subtract, 2),

		// Bitwise Shift
		std::make_pair(PrimitiveTokenType::BitshiftLeft, 3),
		std::make_pair(PrimitiveTokenType::BitshiftRight, 3),
		// Bitwise NOT
		std::make_pair(PrimitiveTokenType::BitNOT, 2),
		// Bitwise AND/OR/XOR
		std::make_pair(PrimitiveTokenType::BitAND, 1),
		std::make_pair(PrimitiveTokenType::BitOR, 1),
		std::make_pair(PrimitiveTokenType::BitXOR, 1),

		// Boolean Not
		std::make_pair(PrimitiveTokenType::LogicalNOT, 2),
		// Comparison operators
		std::make_pair(PrimitiveTokenType::Equal, 1),
		std::make_pair(PrimitiveTokenType::NotEqual, 1),
		std::make_pair(PrimitiveTokenType::LessThan, 1),
		std::make_pair(PrimitiveTokenType::LessOrEqual, 1),
		std::make_pair(PrimitiveTokenType::GreaterThan, 1),
		std::make_pair(PrimitiveTokenType::GreaterOrEqual, 1),
		// Boolean operators
		std::make_pair(PrimitiveTokenType::LogicalOR, 0),
		std::make_pair(PrimitiveTokenType::LogicalAND, 0),
	};
}
