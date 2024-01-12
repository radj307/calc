#pragma once
#include <cstdint>

namespace calc::expr {
	// X-macro defined PrimitiveTokenType enum values
#define PRIMITIVE_TOKEN_TYPE	\
X(Unknown)						\
X(Variable)						\
X(Setter)						\
X(TermSeparator)				\
X(ExpressionOpen)				\
X(ExpressionClose)				\
X(FunctionName)					\
X(ArrayOpen)					\
X(ArrayClose)					\
X(Separator)					\
X(Boolean)						\
X(IntNumber)					\
X(RealNumber)					\
X(BinaryNumber)					\
X(OctalNumber)					\
X(HexNumber)					\
X(Add)							\
X(Subtract)						\
X(Negate)						\
X(Multiply)						\
X(Divide)						\
X(Modulo)						\
X(Exponent)						\
X(Factorial)					\
X(BitOR)						\
X(BitAND)						\
X(BitXOR)						\
X(BitNOT)						\
X(BitshiftLeft)					\
X(BitshiftRight)				\
X(Equal)						\
X(NotEqual)						\
X(LessThan)						\
X(LessOrEqual)					\
X(GreaterThan)					\
X(GreaterOrEqual)				\
X(LogicalNOT)					\
X(LogicalOR)					\
X(LogicalAND)					\

#define X(name) name,
	/**
	 * @brief	Intermediate unit in a tokenized math expression.
	 *			These are one step above a lexeme, and simpler than a full token.
	 */
	enum class PrimitiveTokenType : std::uint8_t {
		PRIMITIVE_TOKEN_TYPE
	};
#undef X

#define X(name) #name,
	/// @brief	Specifies the text names of primitive token types.
	char const* const PrimitiveTokenTypeNames[] {
		PRIMITIVE_TOKEN_TYPE
	};
#undef X

	/// @brief	Determines whether the specified PrimitiveTokenType represents a number.
	constexpr bool is_number(PrimitiveTokenType const tokenType) noexcept
	{
		switch (tokenType) {
		case PrimitiveTokenType::Boolean:
		case PrimitiveTokenType::BinaryNumber:
		case PrimitiveTokenType::OctalNumber:
		case PrimitiveTokenType::HexNumber:
		case PrimitiveTokenType::IntNumber:
		case PrimitiveTokenType::RealNumber:
			return true;
		default:
			return false;
		}
	}
	/// @brief	Determines whether the specified PrimitiveTokenType represents anything
	///          that evaluates to a number.
	constexpr bool evaluates_to_number(PrimitiveTokenType const tokenType) noexcept
	{
		if (is_number(tokenType)) return true;
		switch (tokenType) {
		case PrimitiveTokenType::Variable:
		case PrimitiveTokenType::ExpressionOpen:
		case PrimitiveTokenType::ExpressionClose:
		case PrimitiveTokenType::FunctionName:
			return true;
		default:
			return false;
		}
	}
}
