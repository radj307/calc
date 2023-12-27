#pragma once
#include <cstdint>
#include <cstdio>

namespace calc::expr {

#define LEXEME_TYPE			\
X(Unknown)					\
X(Escape)					\
X(Equal)					\
X(Colon)					\
X(Semicolon)				\
X(Operator)					\
X(Alpha)					\
X(IntNumber)				\
X(RealNumber)				\
X(BinaryNumber)				\
X(OctalNumber)				\
X(HexNumber)				\
X(Period)					\
X(Comma)					\
X(Macro)					\
X(AngleBracketOpen)			\
X(AngleBracketClose)		\
X(SquareBracketOpen)		\
X(SquareBracketClose)		\
X(ParenthesisOpen)			\
X(ParenthesisClose)			\
X(BraceOpen)				\
X(BraceClose)				\
X(_EOF)						\


#define X(name) name,
	/// @brief	Defines the available types for a lexeme token.
	enum class LexemeType : std::uint8_t {
		LEXEME_TYPE
	};
#undef X

#define X(name) #name,
	/// @brief	Specifies the text names of lexeme types.
	char const* const LexemeTypeNames[]{
		LEXEME_TYPE
	};
#undef X

	/// @brief	Determines whether the specified lexeme type is a number or not.
	inline bool lexemeTypeIsNumber(LexemeType const& lexType)
	{
		switch (lexType) {
		case LexemeType::BinaryNumber:
		case LexemeType::OctalNumber:
		case LexemeType::HexNumber:
		case LexemeType::IntNumber:
		case LexemeType::RealNumber:
			return true;
		default:
			return false;
		}
	}
}
