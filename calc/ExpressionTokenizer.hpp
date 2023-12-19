#pragma once
#include <cstdint>
#include <sstream>
#include <vector>
#include <map>

namespace calc::expr {
	/**
	 * @brief	The most basic unit in a tokenized math expression
	 */
	enum class Lexeme : std::uint8_t {
		// Unrecognized character, or null.
		Unknown,
		// ('\\') An escape sequence starting with a backslash that consists of exactly 2 characters (the backslash and the escaped char)
		Escape,
		// ('=') An equals sign
		Equal,
		// ('+') Addition symbol
		Add,
		// ('-') Subtraction symbol
		Subtract,
		// ('*') Multiplication symbol
		Multiply,
		// ('/') Division symbol
		Divide,
		// ('%') Percent symbol
		Percent,
		// (A-Za-z) A letter.
		Alpha,
		// (0-9) A digit that isn't the start of a number.
		Digit,
		// ("0x") The start of a hexadecimal number.
		HexStart,
		// ('<') An opening angle bracket
		AngleBracketOpen,
		// ('>') A closing angle bracket
		AngleBracketClose,
		// ('(') An opening parenthesis
		BracketOpen,
		// (')') A closing parenthesis
		BracketClose,
		// (-1) End-of-file character. Indicates the end of an expression.
		_EOF,
	};

	/**
	 * @brief	Intermediate unit in a tokenized math expression.
	 *			These are one step above a lexeme, and simpler than a full token.
	 */
	enum class PrimitiveTokenType : std::uint8_t {
		// An unrecognized or null primitive token.
		Unknown,

		// OPERAND TYPES:

		// A number literal in binary (base-2)
		BinaryNumber,
		// A number literal in octal (base-8)
		OctalNumber,
		// A number literal in decimal (base-10).
		DecimalNumber,
		// A number literal in hexadecimal (base-16).
		HexNumber,
		// A variable name.
		VariableName,
		// A sub-expression, consisting of an opening bracket, operands, and an operator. Flow control.
		Expression,
		// A function expression.
		Function,

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
		// ("=") Setter/equal operator.
		Equal,
	};

	enum class TokenType : std::uint8_t {
		// find median

		// stats functions

		// 
	};

	// tokenizers:

	namespace token {
		template<typename TToken>
		struct basic_token {
			using token_t = TToken;

			const token_t type;
			const size_t pos;
			const std::string value;

			constexpr basic_token(const token_t& tokenType, const size_t tokenPosition, const std::string& tokenTextValue) : type{ tokenType }, pos{ tokenPosition }, value{ tokenTextValue } {}

			friend constexpr bool operator==(const basic_token<TToken>& l, const basic_token<TToken>& r)
			{
				return l.pos == r.pos && l.type == r.type && l.value == r.value;
			}
			friend constexpr bool operator!=(const basic_token<TToken>& l, const basic_token<TToken>& r)
			{
				return l.pos != r.pos || l.type != r.type || l.value != r.value;
			}
		};

		/// @brief	A lexeme token, the most basic kind of token subtype.
		using lexeme_token = basic_token<Lexeme>;
		/// @brief	A primitive token, one step up from a lexeme but not a full token.
		using primitive_token = basic_token<PrimitiveTokenType>;
		/// @brief	A token.
		using token = basic_token<TokenType>;

	}
}
