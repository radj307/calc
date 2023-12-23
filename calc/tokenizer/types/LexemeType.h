#pragma once
#include <cstdint>
#include <cstdio>

namespace calc::expr {

#define LEXEME_TYPE							\
X(Unknown,)									\
X(Escape,)									\
X(Equal,)									\
X(Colon,)									\
X(Semicolon,)								\
X(Operator,)								\
X(Alpha,)									\
X(IntNumber,)								\
X(RealNumber,)								\
X(BinaryNumber,)							\
X(OctalNumber,)								\
X(HexNumber,)								\
X(Period,)									\
X(Comma,)									\
X(Macro,)									\
X(AngleBracketOpen,)						\
X(AngleBracketClose,)						\
X(SquareBracketOpen,)						\
X(SquareBracketClose,)						\
X(ParenthesisOpen,)							\
X(ParenthesisClose,)						\
X(BraceOpen,)								\
X(BraceClose,)								\
X(_EOF, =static_cast<std::uint8_t>(EOF))	\


#define X(name, value) name value,
	/// @brief	Defines the available types for a lexeme token.
	enum class LexemeType : std::uint8_t {
		LEXEME_TYPE
	};
#undef X

#define X(name, value) #name,
	/// @brief	Specifies the text names of lexeme types.
	char const* const LexemeTypeNames[]{
		LEXEME_TYPE
	};
#undef X
}
