#pragma once
#include "types/LexemeType.h"
#include "types/PrimitiveTokenType.h"
#include "types/ComplexTokenType.h"

#include <strcore.hpp>	//< for stringify

#include <sstream>

namespace calc::expr::tkn {

	/// @brief	Requires type T to be a valid token type enum.
	template<typename T> concept token_type = var::any_same<T, LexemeType, PrimitiveTokenType, ComplexTokenType>;

	/**
	 * @brief				A basic token object with the specified token type.
	 * @tparam TTokenType -	The type of token contained by this instance.
	 */
	template<token_type TTokenType>
	struct basic_token {
		using type_t = TTokenType;

		/// @brief	The type of this token.
		type_t type;
		/// @brief	The starting position of the underlying value in the input stream.
		std::streamoff pos;
		/// @brief	The underlying string value that the token represents.
		std::string text;

		/// @brief	Creates a new empty token instance.
		constexpr basic_token() = default;
		constexpr basic_token(const type_t& type) : type{ type } {}
		/**
		 * @brief				Creates a new token instance with the specified type, starting position, and string value.
		 * @param type		  - The type of token associated with the string value.
		 * @param position	  - The starting position of the string value in the input stream.
		 * @param text		  - The text that the token represents.
		 */
		constexpr basic_token(const type_t& type, const auto position, const std::string& text) : type{ type }, pos{ static_cast<std::streamoff>(position) }, text{ text } {}
		/**
		 * @brief				Creates a new token instance with the specified type, starting position, and string value.
		 * @param type		  - The type of token associated with the string value.
		 * @param position	  - The starting position of the string value in the input stream.
		 * @param text		  - The text that the token represents.
		 */
		constexpr basic_token(const type_t& type, const auto position, const char text) : type{ type }, pos{ static_cast<std::streamoff>(position) }, text{ text } {}
		template<token_type T>
		constexpr basic_token(const type_t& type, basic_token<T> const& otherToken) : type{ type }, pos{ otherToken.pos }, text{ otherToken.text } {}

		/// @brief	Gets the (exclusive) ending position of this token.
		constexpr auto getEndPos() const noexcept { return pos + text.size(); }

		/// @brief	Checks if this token is directly adjacent to the specified position.
		constexpr bool isAdjacentTo(const std::streamoff position) const noexcept
		{
			return position + 1 == pos
				|| getEndPos() == position;
		}
		/// @brief	Checks if this token is directly adjacent to the specified other token.
		constexpr bool isAdjacentTo(const basic_token<TTokenType>& other) const noexcept
		{
			return other.getEndPos() == pos
				|| getEndPos() == other.pos;
		}

		friend constexpr bool operator==(const basic_token<TTokenType>& l, const basic_token<TTokenType>& r)
		{
			return l.pos == r.pos && l.type == r.type && l.str == r.str;
		}
		friend constexpr bool operator!=(const basic_token<TTokenType>& l, const basic_token<TTokenType>& r)
		{
			return l.pos != r.pos || l.type != r.type || l.str != r.str;
		}

		constexpr std::string get_debug_string() const noexcept
		{
			return str::stringify("(Start Index: ", pos, ", End Index: ", getEndPos() - 1, " Text: \"", text, "\")");
		}

		friend std::ostream& operator<<(std::ostream& os, const basic_token<type_t>& tkn) { return os << tkn.text; }
	};

#pragma region combine_tokens
	template<var::derived_from_templated<basic_token> TSource, typename TResultType, std::derived_from<basic_token<TResultType>> TResult = basic_token<TResultType>>
	constexpr TResult combine_tokens(TResultType const& type, std::vector<TSource>&& vec)
	{
		size_t startPos{ static_cast<size_t>(EOF) };
		size_t endPos{ 0ull };
		std::stringstream buf;

		for (const auto& it : vec) {
			if (it.pos < startPos)
				startPos = it.pos;
			if (const auto& tknEndPos{ it.getEndPos() }; tknEndPos > endPos)
				endPos = tknEndPos;
		}

		for (const auto& it : $fwd(vec)) {

			auto pos{ it.pos };

			// pad out the gap between tokens with spaces
			while (pos > endPos) {
				buf << ' ';
				++endPos;
			}

			buf << it;

			endPos = it.getEndPos();
		}

		return{ type, startPos, buf.str() };
	}
	template<var::derived_from_templated<basic_token> TSource, typename TResultType, std::derived_from<basic_token<TResultType>> TResult = basic_token<TResultType>>
	constexpr TResult combine_tokens(TResultType const& type, const std::vector<TSource>& vec)
	{
		size_t startPos{ static_cast<size_t>(EOF) };
		size_t endPos{ 0ull };
		std::stringstream buf;

		for (const auto& it : vec) {
			if (it.pos < startPos)
				startPos = it.pos;
			if (const auto& tknEndPos{ it.getEndPos() }; tknEndPos > endPos)
				endPos = tknEndPos;
		}

		for (const auto& it : $fwd(vec)) {

			auto pos{ it.pos };

			// pad out the gap between tokens with spaces
			while (pos > endPos) {
				buf << ' ';
				++endPos;
			}

			buf << it;

			endPos = it.getEndPos();
		}

		return{ type, startPos, buf.str() };
	}

	template<typename TResultType, std::derived_from<basic_token<TResultType>> TResult = basic_token<TResultType>, var::derived_from_templated<basic_token>... TSource>
	constexpr TResult combine_tokens(TResultType const& type, TSource const&... tokens)
	{
		size_t startPos{ static_cast<size_t>(EOF) };
		size_t endPos{ 0ull };

		([&]() {
			if (tokens.pos < startPos)
			startPos = tokens.pos;
		if (const auto& tknEndPos{ tokens.getEndPos() }; tknEndPos > endPos)
			endPos = tknEndPos;
		 }(), ...);

		std::stringstream buf;

		([&]() {
			auto pos{ tokens.pos };
		while (pos > endPos) {
			buf << ' ';
			++endPos;
		}

		buf << tokens;

		endPos = tokens.getEndPos();
		 }(), ...);

		return{ type, startPos, buf.str() };
	}
#pragma endregion combine_tokens

	/// @brief	A lexeme token, the most basic kind of token subtype.
	using lexeme = basic_token<LexemeType>;
	/// @brief	A primitive token, one step up from a lexeme but not a full token.
	using primitive = basic_token<PrimitiveTokenType>;
	/// @brief	A complex token, the most advanced kind of token subtype.
	using complex = basic_token<ComplexTokenType>;
}
