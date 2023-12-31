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
	inline const std::map<PrimitiveTokenType, uint8_t> OperatorPrecedence::map{
		// [F] Functions
		std::make_pair(PrimitiveTokenType::Factorial, 4),
		std::make_pair(PrimitiveTokenType::FunctionName, 4),
		// [E] Exponents
		std::make_pair(PrimitiveTokenType::Exponent, 3),
		// [DM] Mult/Div
		std::make_pair(PrimitiveTokenType::Multiply, 2),
		std::make_pair(PrimitiveTokenType::Divide, 2),
		// [AS] Add/Sub
		std::make_pair(PrimitiveTokenType::Add, 1),
		std::make_pair(PrimitiveTokenType::Subtract, 1),
	};
}
