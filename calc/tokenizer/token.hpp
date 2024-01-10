#pragma once
#include "types/LexemeType.h"
#include "types/PrimitiveTokenType.h"

#include <strcore.hpp>			//< for stringify()
#include <make_exception.hpp>	//< for make_exception()
#include <indentor.hpp>			//< for indent()

#include <sstream>

namespace calc::expr::tkn {

	/// @brief	Requires type T to be a valid token type enum.
	template<typename T> concept token_type = var::any_same<T, LexemeType, PrimitiveTokenType>;

	/// @brief	Gets the friendly name of the specified token type value.
	template<token_type T> std::string get_name(T const& tokenType)
	{
		if constexpr (std::same_as<T, LexemeType>)
			return LexemeTypeNames[static_cast<int>(tokenType)];
		else if constexpr (std::same_as<T, PrimitiveTokenType>)
			return PrimitiveTokenTypeNames[static_cast<int>(tokenType)];
		else static_assert(token_type<T>, "get_name() does not handle all token types!");
	}

	/**
	 * @brief				A basic token object with the specified token type.
	 * @tparam TTokenType -	The type of token contained by this instance.
	 */
	template<typename TTokenType>
	struct basic_token {
		using type_t = TTokenType;

		/// @brief	The type of this token.
		type_t type;
		/// @brief	The starting position of the underlying value in the input stream.
		std::streamoff pos;
		/// @brief	The underlying string value that the token represents.
		std::string text;

	#pragma region Constructors
		/// @brief	Creates a new token instance with undefined values.
		constexpr basic_token() = default;
		/// @brief	Creates a new token instance with the specified type.
		constexpr basic_token(const type_t& type) : type{ type } {}
		/**
		 * @brief				Creates a new token instance with the specified type, starting position, and underlying text.
		 * @param type		  - The type of token associated with the string value.
		 * @param position	  - The starting position of the string value in the input stream.
		 * @param text		  - The underlying text that the new token represents.
		 */
		constexpr basic_token(const type_t& type, const auto position, const std::string& text) : type{ type }, pos{ static_cast<std::streamoff>(position) }, text{ text } {}
		/**
		 * @brief				Creates a new token instance with the specified type, starting position, and underlying text.
		 * @param type		  - The type of token associated with the string value.
		 * @param position	  - The starting position of the string value in the input stream.
		 * @param text		  - The underlying text that the new token represents.
		 */
		constexpr basic_token(const type_t& type, const auto position, const char text) : type{ type }, pos{ static_cast<std::streamoff>(position) }, text{ text } {}
		/**
		 * @brief				Creates a new token instance with the specified type, but otherwise with the same values as the specified otherToken.
		 * @tparam T		  - The token type of the other token.
		 * @param type		  - The type of this token.
		 * @param otherToken  -	Another basic_token to copy the position and text values from.
		 */
		template<token_type T>
		constexpr basic_token(const type_t& type, basic_token<T> const& otherToken) : type{ type }, pos{ otherToken.pos }, text{ otherToken.text } {}
	#pragma endregion Constructors

	#pragma region Get End Position
		/**
		 * @brief		Gets the (exclusive) ending position of this token.
		 * @returns		When the text size is greater than 0, the index of directly after this token in the underlying text.
		 */
		constexpr size_t getEndPos() const noexcept
		{
			return pos + text.size();
		}
		/**
		 * @brief		Gets the (inclusive) ending index of this token.
		 * @returns		The index of the last character represented by this token when the underlying text isn't empty; otherwise, the token's position.
		 */
		constexpr size_t getEndIndex() const noexcept
		{
			if (text.size() == 0) return pos;
			else return pos + text.size() - 1;
		}
	#pragma endregion Get End Position

	#pragma region isAdjacentTo
		/**
		 * @brief				Checks if this token instance is directly adjacent to the specified position.
		 * @param position	  -	An index to compare with this token's position.
		 * @returns				true when this token comes immediately after, or immediately before, the specified position.
		 */
		constexpr bool isAdjacentTo(const std::streamoff position) const noexcept
		{
			return position + 1 == pos
				|| getEndPos() == position;
		}
		/**
		 * @brief				Checks if this token instance is directly adjacent to the specified other token.
		 * @tparam T		  -	The token type of the other token instance.
		 * @param other		  -	Another token instance to compare the positions of.
		 * @returns				true when this token comes immediately after, or immediately before, the specified position.
		 */
		template<token_type T>
		constexpr bool isAdjacentTo(const basic_token<T>& other) const noexcept
		{
			return other.getEndPos() == pos
				|| getEndPos() == other.pos;
		}
	#pragma endregion isAdjacentTo

	#pragma region operator== & operator!=
		/// @brief	Checks if the specified tokens have the same values.
		friend constexpr bool operator==(const basic_token<type_t>& l, const basic_token<type_t>& r)
		{
			return l.pos == r.pos && l.type == r.type && l.text == r.text;
		}
		/// @brief	Checks if the specified tokens have different values.
		friend constexpr bool operator!=(const basic_token<type_t>& l, const basic_token<type_t>& r)
		{
			return l.pos != r.pos || l.type != r.type || l.text != r.text;
		}
	#pragma endregion operator== & operator!=

	#pragma region operator< & operator>
		/// @brief	Checks if the left-side token's end position is before the right-side token's start position.
		friend constexpr bool operator<(const basic_token<type_t>& l, const basic_token<type_t>& r)
		{
			return l.getEndIndex() < r.pos;
		}
		/// @brief	Checks if the left-side token's start position is after the right-side token's end position.
		friend constexpr bool operator>(const basic_token<type_t>& l, const basic_token<type_t>& r)
		{
			return l.pos > r.getEndIndex();
		}
	#pragma endregion operator< & operator>

		/// @brief	Gets a string that contains the starting index, ending index, the underlying text, and a label for each.
		constexpr std::string get_debug_string() const noexcept
		{
			return str::stringify("(Start Index: ", pos, ", End Index: ", getEndPos() - 1, " Text: \"", text, "\")");
		}

		template<typename Tr> requires (sizeof(Tr) == sizeof(type_t))
			constexpr basic_token<Tr> generic_token() const
		{
			return{ static_cast<Tr>(type), pos, text };
		}

		/// @brief	Prints the token's text to the output stream.
		friend std::ostream& operator<<(std::ostream& os, const basic_token<type_t>& tkn)
		{
			return os << tkn.text;
		}
	};

	/// @brief	A lexeme token, the most basic kind of token subtype.
	using lexeme = basic_token<LexemeType>;
	/// @brief	A primitive token, one step up from a lexeme.
	using primitive = basic_token<PrimitiveTokenType>;

#pragma region combine_tokens
	/**
	 * @brief					Concatenates the specified tokens into a merged token with the specified type.
	 * @tparam TResultType	  -	The type of token type used by the resulting merged token.
	 * @tparam TokenIt		  -	The type of iterator for the tokens container.
	 * @tparam TResult		  -	The type of the resulting merged token.
	 * @param type			  -	The token type of the new merged token instance.
	 * @param begin			  -	The iterator to begin combining tokens at.
	 * @param end			  -	The (exclusive) iterator to stop combining tokens at.
	 * @returns					A single merged token with the specified token type.
	 */
	template<std::input_iterator TokenIt, token_type TResultType, std::derived_from<basic_token<TResultType>> TResult = basic_token<TResultType>>
	constexpr TResult combine_tokens(TResultType const& type, TokenIt const& begin, TokenIt const& end)
	{
		if (begin == end) return{ type }; //< short-circuit

		std::stringstream buf;
		std::streamoff startPos{ begin->pos };

		for (auto it{ begin }; it != end; ++it) {
			if (it != begin) {
				if (const auto prevEndPos{ (it - 1)->getEndPos() }; prevEndPos < it->pos) {
					// insert padding
					buf << indent(it->pos, prevEndPos);
				}
				else if (prevEndPos > it->pos)
					throw make_exception("combine_tokens():  The specified tokens are in an invalid order; expected ", it->get_debug_string(), " to come before ", it->get_debug_string());
				// else if equal, fallthrough (tokens are adjacent)
			}

			buf << *it;
		}

		return{ type, startPos, buf.str() };
	}
	/**
	 * @brief					Concatenates the specified tokens into a merged token with the specified type.
	 * @tparam TResultType	  -	The type of token type used by the resulting merged token.
	 * @tparam TSource		  -	The type of the tokens in the input vector.
	 * @tparam TResult		  -	The type of the resulting merged token.
	 * @param type			  -	The token type of the new merged token instance.
	 * @param tokens		  -	A vector of tokens to merge.
	 * @returns					A single merged token with the specified token type.
	 */
	template<var::derived_from_templated<basic_token> TSource, token_type TResultType, std::derived_from<basic_token<TResultType>> TResult = basic_token<TResultType>>
	constexpr TResult combine_tokens(TResultType const& type, const std::vector<TSource>& tokens)
	{
		if (tokens.empty()) return{ type }; //< short-circuit

		std::stringstream buf;
		std::streamoff startPos{ tokens.front().pos };

		for (auto it_begin{ tokens.begin() }, it{ it_begin }, it_end{ tokens.end() }; it != it_end; ++it) {
			if (it != it_begin) {
				if (const auto prevEndPos{ (it - 1)->getEndPos() }; prevEndPos < it->pos) {
					// insert padding
					buf << indent(it->pos, prevEndPos);
				}
				else if (prevEndPos > it->pos)
					throw make_exception("combine_tokens():  The specified tokens are in an invalid order; expected ", it->get_debug_string(), " to come before ", it->get_debug_string());
				// else if equal, fallthrough (tokens are adjacent)
			}

			buf << *it;
		}

		return{ type, startPos, buf.str() };
	}
	/**
	 * @brief					Combines any number of tokens into a single merged token.
	 * @tparam TResultType	  -	The type of token type used by the resulting merged token.
	 * @tparam TResult		  -	The type of the resulting merged token.
	 * @tparam ...TSource	  -	The type(s) of the tokens to merge.
	 * @param type			  -	The token type of the new merged token instance.
	 * @param ...tokens		  -	Any number of tokens to merge.
	 * @returns					A single merged token with the specified token type.
	 */
	template<token_type TResultType, std::derived_from<basic_token<TResultType>> TResult = basic_token<TResultType>, var::derived_from_templated<basic_token>... TSource>
	constexpr TResult combine_tokens(TResultType const& type, TSource const&... tokens) requires var::at_least_one<TSource...>
	{
		std::stringstream buf;
		std::streamoff startPos{ -1 }, prevEndPos{ startPos };

		([&](const auto& tkn) {
			const auto tknEndPos{ tkn.getEndPos() };

			if (startPos == -1)
				startPos = tkn.pos;
			else {
				if (prevEndPos < tkn.pos)
					buf << indent(tkn.pos - prevEndPos);
				else if (prevEndPos > tkn.pos)
					throw make_exception("combine_tokens():  The specified tokens are in an invalid order!");
				// else if equal, fallthrough (tokens are adjacent)
			}

			buf << tkn;
			prevEndPos = tknEndPos;
		 }(tokens), ...);

		return{ type, startPos, buf.str() };
	}
#pragma endregion combine_tokens

#pragma region stringify_tokens
	/**
	 * @brief				Creates a string that represents the specified tokens.
	 *						The tokens must be in the correct order.
	 * @tparam INCLUDE_WS -	When true, the returned string includes preceding whitespace before the first token. Defaults to true.
	 * @param tokens	  -	The tokens to include in the string.
	 * @returns				A string that includes the specified tokens.
	 */
	template<bool INCLUDE_WS = true>
	constexpr std::string stringify_tokens(var::derived_from_templated<basic_token> auto&&... tokens)
	{
		std::stringstream ss;
		std::streamoff startPos{ -1 }, prevEndPos{ startPos };

		([&](const auto& tkn) {
			const auto tknEndPos{ tkn.getEndPos() };

		if (startPos == -1) {
			startPos = tkn.pos;
			if (INCLUDE_WS && startPos > 0) // include preceding whitespace
				ss << indent(startPos);
		}
		else {
			if (prevEndPos < tkn.pos)
				ss << indent(tkn.pos, prevEndPos);
			else if (prevEndPos > tkn.pos)
				throw make_exception("combine_tokens():  The specified tokens are in an invalid order!");
			// else if equal, fallthrough (tokens are adjacent)
		}

		ss << tkn;
		prevEndPos = tknEndPos;
		 }(tokens), ...);

		return ss.str();
	}
	template<bool INCLUDE_WS = true, class TokenIt>
	constexpr std::string stringify_tokens(TokenIt const& begin, TokenIt const& end)
	{
		if (begin == end) return{};

		std::stringstream ss;

		// print the first token
		if (INCLUDE_WS && begin->pos > 0) {
			// include preceding whitespace
			ss << indent(begin->pos);
		}
		ss << *begin;
		size_t prevEndPos{ begin->getEndPos() };

		// print the rest of the tokens
		for (auto it{ begin + 1 }; it != end; ++it) {
			ss << indent(it->pos, prevEndPos) << *it;
			prevEndPos = it->getEndPos();
		}

		return ss.str();
	}
#pragma endregion stringify_tokens
}
