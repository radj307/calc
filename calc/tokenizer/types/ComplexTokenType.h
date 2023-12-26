#pragma once
#include <cstdint>

namespace calc::expr {
#define COMPLEX_TOKEN_TYPE	\
X(Unknown)					\
X(SubExpression)			\
X(Function)					\
X(Operation)				\

#define X(name) name,
	enum class ComplexTokenType : std::uint8_t {
		COMPLEX_TOKEN_TYPE
	};
#undef X

#define X(name) #name,
	char const* const ComplexTokenTypeNames[]{
		COMPLEX_TOKEN_TYPE
	};
#undef X
}
