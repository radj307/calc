#pragma once
#include <cstdint>

namespace calc::expr {
	// X-macro defined PrimitiveTokenType enum values
#define PRIMITIVE_TOKEN_TYPE \
X(Unknown,) \
X(Variable,) \
X(ExpressionOpen,) \
X(ExpressionClose,) \
X(FunctionName,) \
X(FunctionParamsOpen,) \
X(FunctionParamsClose,) \
X(IntNumber,) \
X(RealNumber,) \
X(BinaryNumber,) \
X(OctalNumber,) \
X(HexNumber,) \
X(Add,) \
X(Subtract,) \
X(Multiply,) \
X(Divide,) \
X(Modulo,) \
X(Exponent,) \
X(Factorial,) \
X(LeftShift,) \
X(RightShift,) \
X(BitOR,) \
X(BitAND,) \
X(BitXOR,) \
X(BitNOT,) \
X(ShiftLeft,) \
X(ShiftRight,) \
X(Equal,) \
X(LessThan,) \
X(GreaterThan,) \

#define X(name, value) name value,
	/**
	 * @brief	Intermediate unit in a tokenized math expression.
	 *			These are one step above a lexeme, and simpler than a full token.
	 */
	enum class PrimitiveTokenType : std::uint8_t {
		PRIMITIVE_TOKEN_TYPE
	};
#undef X

#define X(name, value) #name,
	/// @brief	Specifies the text names of primitive token types.
	char const* const PrimitiveTokenTypeNames[] {
		PRIMITIVE_TOKEN_TYPE
	};
#undef X
}
