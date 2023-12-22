#pragma once
#include <cstdint>

namespace calc::expr {
	/**
	 * @brief	Intermediate unit in a tokenized math expression.
	 *			These are one step above a lexeme, and simpler than a full token.
	 */
	enum class PrimitiveTokenType : std::uint8_t {
		// An unrecognized or null primitive token.
		Unknown,

		// OPERAND TYPES:

		// A variable name.
		Variable,
		// The open bracket for an expression.
		ExpressionOpen,
		// The close bracket for an expression.
		ExpressionClose,
		// A function name.
		FunctionName,
		// The open bracket for a function parameter block.
		FunctionParamsOpen,
		// The close bracket for a function parameter block.
		FunctionParamsClose,
		// ([1-9]+) An integral number that starts with a non-zero digit. The underlying value of lexemes are not validated by the lexer.
		IntNumber,
		// ([1-9][0-9]+.[0-9]+) A floating-point number that contains at least one decimal point. The underlying value of lexemes are not validated by the lexer.
		RealNumber,
		// ("0b[01]+") A binary number. Binary numbers always start with "0b". The underlying value of lexemes are not validated by the lexer.
		BinaryNumber,
		// (0[0-7]+) An octal number. Octal numbers always start with "0". The underlying value of lexemes are not validated by the lexer.
		OctalNumber,
		// (0x[0-9A-Fa-f]+) A hexadecimal number. Hex numbers always start with "0x". The underlying value of lexemes are not validated by the lexer.
		HexNumber,

		// FUNCTIONS:


		// OPERATORS:

		// Addition operator. Corresponds to a plus ('+')
		Add,
		// Subtraction operator. Corresponds to a dash ('-')
		Subtract,
		// Multiplication operator. Corresponds to a star '*', a 'x', or whitespace between two numbers/variables.
		Multiply,
		// Division operator.
		Divide,
		// Modulo operator.
		Modulo,
		// Exponent operator.
		Exponent,
		// Factorial operator.
		Factorial,
		// ("<<") Indicates a left bitshift operation.
		LeftShift,
		// (">>") Indicates a right bitshift operation.
		RightShift,
		// ("|") Bitwise OR operator.
		BitOR,
		// ("&") Bitwise AND operator.
		BitAND,
		// ("^") Bitwise XOR operator.
		BitXOR,
		// ("~") Bitwise NOT operator.
		BitNOT,
		// ("<<") Left bitshift
		ShiftLeft,
		// (">>") Right bitshift
		ShiftRight,
		// ("=") Setter/equal operator.
		Equal,
		// ("<") Less than comparison operator.
		LessThan,
		// (">") Greater than comparison operator.
		GreaterThan,
	};
}
