#pragma once
#include <cstdint>
#include <cstdio>

namespace calc::expr {
	/**
	 * @brief	The most basic unit in a tokenized math expression
	 */
	enum class LexemeType : std::uint8_t {
		// Unrecognized character, or null.
		Unknown,
		// ('\\') An escape sequence starting with a backslash that consists of exactly 2 characters (the backslash and the escaped char)
		Escape,
		// ('=') An equals sign
		Equal,
		// (':') A colon
		Colon,
		// (';') A semicolon
		Semicolon,
		// ([+-*/%^!~]) An arithmetic operator of some kind.
		Operator,
		// ([A-Za-z]) An upper or lower-case letter. Also includes underscores '_' that aren't in the middle of a number.
		Alpha,
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
		// ('.') A period character
		Period,
		// (',') A comma character
		Comma,
		// ([$@]) A macro paste symbol; corresponds to either a dollar sign '$', or an at symbol '@'.
		Macro,
		// ('<') An opening angle bracket / less than symbol
		AngleBracketOpen,
		// ('>') A closing angle bracket / greater than symbol
		AngleBracketClose,
		// ('[') An opening square bracket.
		SquareBracketOpen,
		// (']') A closing square bracket.
		SquareBracketClose,
		// ('(') An opening parenthesis
		ParenthesisOpen,
		// (')') A closing parenthesis
		ParenthesisClose,
		// ('{') An opening brace.
		BraceOpen,
		// ('}') A closing brace.
		BraceClose,
		// (-1) End-of-file character. Indicates the end of an expression.
		_EOF = static_cast<std::uint8_t>(EOF),
	};
}
