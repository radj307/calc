#pragma once
#include "token.hpp"
#include "../OperatorPrecedence.hpp"
#include "../settings.h"
#include "../FunctionMap.hpp"

#include <var.hpp>				//< for concepts
#include <charcompare.hpp>		//< for stdpred noexcept
#include <strcore.hpp>			//< for stringify
#include <make_exception.hpp>	//< for make_exception


namespace calc::expr::tkn {
	/// @brief	 Tokenizer that converts lexemes into primitive tokens, and performs syntax analysis to verify that the expression uses valid syntax.
	class primitive_tokenizer {
		using const_iterator = typename std::vector<lexeme>::const_iterator;
		using const_reverse_iterator = typename std::vector<lexeme>::const_reverse_iterator;

	protected:
		const std::vector<lexeme> lexemes;
		const_iterator begin;
		const_iterator end;
		const_iterator current;
		FunctionMap const* const functionMap{ nullptr };

		/// @brief	Gets the specified range of the expression as a string.
		template<bool INCLUDE_WS = false>
		std::string get_expr_as_string(const_iterator const& begin, const_iterator const& end) const
		{
			return stringify_tokens<INCLUDE_WS>(begin, end);
		}
		/// @brief	Gets the entire expression as a string.
		std::string get_expr_as_string() const
		{
			return get_expr_as_string<true>(lexemes.begin(), lexemes.back().type == LexemeType::_EOF ? lexemes.end() - 1 : lexemes.end());
		}

		/**
		 * @brief				Creates an formatted error message that indicates the token that caused the error.
		 * @param beginTkn	  -	Begin iterator of the error range. (the '~' chars start here)
		 * @param endTkn	  -	(Exclusive) end iterator of the error range. (the '~' chars stop just before here)
		 * @param errorTkn	  -	Iterator for the token that caused the error. (the '^' char is shown here)
		 * @param message	  -	The error message to display.
		 * @param indent_sz	  -	The length of the indent to put at the beginning of each line after the first one.
		 * @returns				
		 */
		std::string make_error_msg(const_iterator const& beginTkn, const_iterator const& endTkn, const_iterator const& errorTkn, std::string const& message, size_t const indent_sz = 10) const
		{
			std::stringstream ss;

			const auto expr_str{ get_expr_as_string() };

			size_t endPos{ expr_str.size() };
			if (endTkn != end) endPos = endTkn->pos;

			// print the entire expression
			ss << expr_str << '\n';

			// print the error indicator
			ss
				<< indent(indent_sz + beginTkn->pos)
				<< csync(color::dark_red) << indent(errorTkn->pos, beginTkn->pos, '~')
				<< csync(color::red) << indent(errorTkn->getEndPos(), errorTkn->pos, '^')
				<< csync(color::dark_red) << indent(endPos, errorTkn->getEndPos(), '~')
				<< csync() << '\n'
				;

			// print the error message
			if (!message.empty())
				ss << indent(indent_sz) << message << '\n';

			return ss.str();
		}

		bool isFunctionName(std::string const& functionName) const noexcept
		{
			return functionMap != nullptr
				&& functionMap->isFunction(functionName);
		}

		/**
		 * @brief			Gets the specified range of tokens as a vector.
		 * @param begin   -	The beginning of the range.
		 * @param end	  -	The (exclusive) end of the range.
		 * @returns			A std::vector containing the specified range of lexemes.
		 */
		std::vector<lexeme> getRange(const_iterator const& begin, const_iterator const& end)
		{
			const auto distance{ std::distance(begin, end) };

			if (distance <= 0)
				return{};
			if (distance == 1)
				return{ *begin };

			std::vector<lexeme> vec;
			vec.reserve(distance);

			for (auto it{ begin }; it != end; ++it) {
				vec.emplace_back(*it);
			}

			return std::move(vec);
		}

		std::pair<const_iterator, size_t> findPairClose(const_iterator const& startAt, const_iterator const& stopBefore, const LexemeType openTknType, const LexemeType closeTknType, size_t targetDepth = 0) const
		{
			size_t depth{ 0 };

			for (auto it{ startAt }; it != stopBefore; ++it) {
				if (it->type == openTknType) {
					++depth;
				}
				else if (it->type == closeTknType) {
					if (--depth == targetDepth)
						return std::make_pair(it, depth);
				}
			}

			return std::make_pair(stopBefore, depth);
		}
		std::pair<const_iterator, size_t> findPairOpen(const_iterator const& startAt, const_iterator const& stopBefore, const LexemeType openTknType, const LexemeType closeTknType, size_t targetDepth = 0) const
		{
			size_t depth{ 0 };

			for (auto it{ startAt }; it != stopBefore; ++it) {
				if (it->type == openTknType) {
					if (depth++ == targetDepth)
						return std::make_pair(it, depth);
				}
				else if (it->type == closeTknType) {
					if (--depth == 0)
						break;
				}
			}

			return std::make_pair(stopBefore, depth);
		}

		/**
		 * @brief				Finds the next occurrence of the specified tokenType.
		 * @param startAt	  -	Iterator to begin searching at.
		 * @param tokenType   -	The type of token to search for.
		 * @returns				Iterator of the next token with the specified type
		 *						 when successful; otherwise, the end iterator.
		 */
		const_iterator findNext(const_iterator const& startAt, LexemeType const tokenType) const
		{
			for (auto it{ startAt }; it != end; ++it) {
				if (it->type == tokenType)
					return it;
			}
			return end;
		}
		const_iterator findPrev(const_iterator const& startAt, LexemeType const tokenType) const
		{
			for (auto it{ std::make_reverse_iterator(startAt) }, it_end{ lexemes.rend() }; it != it_end; ++it) {
				if (it->type == tokenType)
					return it.base();
			}
			return begin;
		}

		/**
		 * @brief							Finds the first lexeme that has a gap between it and the previous one.
		 * @param start					  - The position to start searching at.
		 * @param returnPreviousInstead	  - When true, returns the lexeme before the gap instead of the one after it.
		 * @returns							An iterator to the target lexeme when successful; otherwise, the end iterator.
		 */
		[[nodiscard]] const_iterator findFirstNonAdjacent(const_iterator const& start, const bool returnPreviousInstead = false) const
		{
			// short circuit if we're at the end
			if (start == end) return start;

			// cache the starting iterator
			const_iterator prev{ start };

			// begin looping from the next item
			for (auto it{ start + 1 }; it != end; ++it) {
				// check if not adjacent to previous
				if (!it->isAdjacentTo(*prev))
					return returnPreviousInstead ? prev : it;
				// otherwise, continue
				prev = it;
			}

			// nothing found
			return end;
		}
		/**
		 * @brief					Finds the first lexeme that doesn't have one of the specified types.
		 * @param start			  - The position to start searching at.
		 * @param ...lexemeTypes  - At least one lexeme type
		 * @returns					An iterator to the target lexeme when successful; otherwise, the end iterator.
		 */
		template<std::same_as<LexemeType>... Ts> requires (var::at_least_one<Ts...>)
			[[nodiscard]] const_iterator findFirstNotOfType(const_iterator const& start, Ts const&... lexemeTypes) const
		{
			// begin looping at the specified start
			for (auto it{ start }; it != end; ++it) {
				if (var::variadic_or(it->type == lexemeTypes...))
					continue;
				// else this lexeme is not of a specified type
				return it;
			}
			// nothing found
			return end;
		}
		template<std::same_as<LexemeType>... Ts> requires (var::at_least_one<Ts...>)
			[[nodiscard]] const_iterator findFirstNonAdjacentOrNotOfType(const_iterator const& start, Ts const&... lexemeTypes) const
		{
			// short circuit if starting at the end
			if (start == end) return start;

			// cache the starting iterator
			const_iterator prev{ start };

			// begin looping from the next item
			for (auto it{ start + 1 }; it != end; ++it) {
				// check if this lexeme's type wasn't specified, or if it isn't adjacent to previous
				if (!var::variadic_or(it->type == lexemeTypes...) || !it->isAdjacentTo(*prev)) {
					return it;
				}
				// otherwise, continue
				prev = it;
			}

			// nothing found
			return end;
		}

		std::vector<primitive> getNextPrimitivesFrom(const_iterator& iterator, primitive const* const previousPrimitive)
		{
			const auto lex{ *iterator };

			switch (lex.type) {
			case LexemeType::Semicolon:
				return{ { PrimitiveTokenType::Separator, lex } };
			case LexemeType::Equal:
				if (iterator != end) {
					if (const auto& next{ iterator + 1 }; next->type == LexemeType::Equal && lex.isAdjacentTo(*next)) {
						// is Equal comparison operator
						return{ combine_tokens(PrimitiveTokenType::Equal, lex, *++iterator) };
					}
				}
				[[fallthrough]];
			case LexemeType::Colon:
				return{ { PrimitiveTokenType::Setter, lex } };
			case LexemeType::Comma:
				return{ { PrimitiveTokenType::TermSeparator, lex } };
			case LexemeType::Operator: // resolve operator:
				switch (lex.text.front()) {
				case '+':
					return{ { PrimitiveTokenType::Add, lex } };
				case '-': {
					// determine whether this is a subtact or negate operator

					if (const auto& next{ iterator + 1 }; next != end
						&& (previousPrimitive == nullptr || (!evaluates_to_number(previousPrimitive->type) || previousPrimitive->type == PrimitiveTokenType::ExpressionOpen))
						&& evaluates_to_number(next->type)) {
						// is negate operator
						return{ { PrimitiveTokenType::Negate, lex } };
					}
					else // is subtract operator
						return{ { PrimitiveTokenType::Subtract, lex } };
				}
				case '*':
					return{ { PrimitiveTokenType::Multiply, lex } };
				case '/':
					return{ { PrimitiveTokenType::Divide, lex } };
				case '%':
					return{ { PrimitiveTokenType::Modulo, lex } };
				case '!':
					if (iterator != end) {
						// if the next token is adjacent and an equal sign
						if (const auto next{ iterator + 1 }; lex.isAdjacentTo(*next) && next->type == LexemeType::Equal) {
							// is NotEqual comparison operator
							return{ combine_tokens(PrimitiveTokenType::NotEqual, lex, *++iterator) };
						}
					}
					// if the previous token is adjacent & an operand
					if (previousPrimitive != nullptr && lex.isAdjacentTo(*previousPrimitive) && evaluates_to_number(previousPrimitive->type)) {
						// is Factorial
						return{ { PrimitiveTokenType::Factorial, lex } };
					}
					else// is LogicalNOT
						return{ { PrimitiveTokenType::LogicalNOT, lex } };
				case '|':
					if (iterator != end) {
						// if the next token is adjacent and also a vertical bar
						if (const auto& next{ iterator + 1 }; lex.isAdjacentTo(*next) && next->type == LexemeType::Operator && next->text == "|") {
							// is LogicalOR
							return{ combine_tokens(PrimitiveTokenType::LogicalOR, lex, *++iterator) };
						}
					}
					return{ { PrimitiveTokenType::BitOR, lex } };
				case '&':
					if (iterator != end) {
						// if the next token is adjacent and also an ampersand
						if (const auto& next{ iterator + 1 }; lex.isAdjacentTo(*next) && next->type == LexemeType::Operator && next->text == "&") {
							// is LogicalAND
							return{ combine_tokens(PrimitiveTokenType::LogicalAND, lex, *++iterator) };
						}
					}
					return{ { PrimitiveTokenType::BitAND, lex } };
				case '^':
					return{ { settings.caretIsExponent ? PrimitiveTokenType::Exponent : PrimitiveTokenType::BitXOR, lex } };
				case '~':
					return{ { PrimitiveTokenType::BitNOT, lex } };
				default:
					throw make_exception("primitive_tokenizer::getNextPrimitive():  No implementation available for operator type \"", lex.text, '\"');
				}
			case LexemeType::AngleBracketOpen:
				if (iterator != end) {
					// if the next token is adjacent
					if (const auto& next{ iterator + 1 }; lex.isAdjacentTo(*next)) {
						if (next->type == LexemeType::AngleBracketOpen) {
							// left bitshift
							return{ combine_tokens(PrimitiveTokenType::BitshiftLeft, lex, *++iterator) };
						}
						else if (next->type == LexemeType::Equal) {
							// is less or equal
							return{ combine_tokens(PrimitiveTokenType::LessOrEqual, lex, *++iterator) };
						}
					}
				}
				// less than
				return{ { PrimitiveTokenType::LessThan, lex } };
			case LexemeType::AngleBracketClose:
				if (iterator != end) {
					// if the next token is adjacent
					if (const auto& next{ iterator + 1 }; lex.isAdjacentTo(*next)) {
						if (next->type == LexemeType::AngleBracketClose) {
							// right bitshift
							return{ combine_tokens(PrimitiveTokenType::BitshiftRight, lex, *++iterator) };
						}
						else if (next->type == LexemeType::Equal) {
							// is greater or equal
							return{ combine_tokens(PrimitiveTokenType::GreaterOrEqual, lex, *++iterator) };
						}
					}
				}
				// greater than
				return{ { PrimitiveTokenType::GreaterThan, lex } };
			case LexemeType::SquareBracketOpen:
				return{ { PrimitiveTokenType::ArrayOpen, lex } };
			case LexemeType::SquareBracketClose:
				return{ { PrimitiveTokenType::ArrayClose, lex } };
			case LexemeType::BinaryNumber:
				return{ { PrimitiveTokenType::BinaryNumber, lex } };
			case LexemeType::OctalNumber:
				return{ { PrimitiveTokenType::OctalNumber, lex } };
			case LexemeType::HexNumber:
				return{ { PrimitiveTokenType::HexNumber, lex } };
			case LexemeType::IntNumber:
				return{ { PrimitiveTokenType::IntNumber, lex } };
			case LexemeType::RealNumber:
				return{ { PrimitiveTokenType::RealNumber, lex } };
			case LexemeType::Alpha: { // function, variable, or boolean literal
				// get the next non-Alpha or non-adjacent token
				const auto& nextNonAlpha{ findFirstNonAdjacentOrNotOfType(iterator, LexemeType::Alpha, LexemeType::Underscore) };

				if (nextNonAlpha != end) {
					// stringify adjacent alpha tokens
					const auto alphaStr{ stringify_tokens<false>(iterator, nextNonAlpha) };

					if (nextNonAlpha->type == LexemeType::ParenthesisOpen && isFunctionName(alphaStr)) {
						// is function name
						const auto fn{ combine_tokens(PrimitiveTokenType::FunctionName, getRange(iterator, nextNonAlpha)) };

						// move the iterator to the last alpha pos
						iterator = nextNonAlpha - 1;

						return{ fn };
					}
				}

				// is a variable
				std::vector<primitive> variables;
				variables.reserve(std::distance(iterator, nextNonAlpha));

				for (; iterator != nextNonAlpha; ++iterator) {
					variables.emplace_back(primitive{ PrimitiveTokenType::Variable, *iterator });
				}

				--iterator; //< move the iterator to the last alpha token

				// no need to shrink since the returned vector is temporary
				return std::move(variables);
			}
			case LexemeType::ParenthesisOpen:
				return{ { PrimitiveTokenType::ExpressionOpen, lex } };
			case LexemeType::ParenthesisClose:
				return{ { PrimitiveTokenType::ExpressionClose, lex } };
			default:
				break;
			}
			return{ { PrimitiveTokenType::Unknown, lex } };
		}

	public:
		primitive_tokenizer(std::vector<lexeme> const& lexemes, FunctionMap const* functionMap) : lexemes{ lexemes }, begin{ this->lexemes.begin() }, end{ this->lexemes.end() }, current{ this->lexemes.begin() }, functionMap{ functionMap } {}

		/// @brief	Tokenizes the lexeme buffer into a vector of primitive tokens.
		std::vector<primitive> tokenize()
		{
			std::vector<primitive> vec{};
			if (lexemes.empty()) return vec; //< if there aren't any lexemes, short circuit

			// validate bracket pairs
			std::vector<std::pair<const_iterator, const_iterator>> bracket_pairs;
			for (auto it{ begin }; it != end; ++it) {
				switch (it->type) {
				case LexemeType::ParenthesisOpen: {
					const auto& nextSeparator{ findNext(it, LexemeType::Semicolon) };
					const auto& [endBracket, depth] { findPairClose(it, nextSeparator, LexemeType::ParenthesisOpen, LexemeType::ParenthesisClose) };

					if (endBracket == nextSeparator) {
						// try to get the actual unmatched bracket
						const auto& [errorBracket, _] { findPairOpen(it, nextSeparator, LexemeType::ParenthesisOpen, LexemeType::ParenthesisClose, depth) };

						throw make_exception(make_error_msg(findPrev(it, LexemeType::Semicolon), nextSeparator, (errorBracket != nextSeparator ? errorBracket : it), "Syntax Error: Unmatched open bracket!"));
					}
					else bracket_pairs.emplace_back(std::make_pair(it, endBracket));

					break;
				}
				case LexemeType::ParenthesisClose:
					if (!std::any_of(bracket_pairs.begin(), bracket_pairs.end(), [&it](auto&& pr) { return pr.second == it; })) {
						throw make_exception(make_error_msg(findPrev(it, LexemeType::Semicolon), findNext(it, LexemeType::Semicolon), it, "Syntax Error: Unmatched close bracket!"));
					}
					break;
				}
			}

			// reserve enough space for 1:1 token count (worst case)
			vec.reserve(lexemes.size());

			// tokenize all of the lexemes into primitives
			for (; current != end; ++current) {
				if (current->type == LexemeType::_EOF) break;

				const auto tokens{ getNextPrimitivesFrom(current, vec.empty() ? nullptr : &vec.back()) };
				vec.insert(vec.end(), tokens.begin(), tokens.end());
			}

			// remove unused space & return
			vec.shrink_to_fit();
			return vec;
		}
	};
}
