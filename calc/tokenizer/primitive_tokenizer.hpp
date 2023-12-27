#pragma once
#include "token.hpp"
#include "../settings.h"

#include <var.hpp>				//< for concepts
#include <charcompare.hpp>		//< for stdpred noexcept
#include <strcore.hpp>			//< for stringify
#include <make_exception.hpp>	//< for make_exception


namespace calc::expr::tkn {
	/// @brief	 Tokenizer that converts lexemes into primitive tokens, and performs syntax analysis to verify that the expression uses valid syntax.
	class primitive_tokenizer {
		using const_iterator = typename std::vector<lexeme>::const_iterator;

	protected:
		const std::vector<lexeme> lexemes;
		const_iterator begin;
		const_iterator end;
		const_iterator current;

		template<size_t INDENT = 10, var::streamable... Ts>
		std::string make_error_message_from(const_iterator const& iterator, std::string const& tokenErrorMessage, Ts&&... message)
		{
			std::stringstream ss;
			(ss << ... << message);

			if (sizeof...(Ts) > 0)
				ss << '\n';

			// previous token:
			if (iterator != begin)
				ss << indent(INDENT) << "Prev. Token: " << (iterator - 1)->get_debug_string() << '\n';
			else
				ss << indent(INDENT) << "Prev. Token: (None)" << '\n';

			// error token:
			ss << indent(INDENT) << "Token:       " << iterator->get_debug_string() << " <--- " << tokenErrorMessage << '\n';

			// next token:
			if (iterator != end && (iterator + 1) != end)
				ss << indent(INDENT) << "Next Token:  " << (iterator + 1)->get_debug_string();
			else
				ss << indent(INDENT) << "Next Token:  (None)";

			return ss.str();
		}

		bool isFunctionName(std::string const& functionName) const noexcept
		{
			// TODO: Implement this
			return true;
		}

		/// @brief					Checks if the iterator has reached the end of the available lexemes
		/// @param orEofLexeme	  -	When true, also checks if the iterator is at the _EOF lexeme.
		/// @returns				true when the iterator is at the end (or EOF lexeme); otherwise, false.
		bool at_end() const noexcept
		{
			return current == end;
		}
		bool hasNext() const noexcept
		{
			return !at_end() && std::distance(current, end) >= 1;
		}

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

		const_iterator findEndBracket(const_iterator const& start, const LexemeType openBracketType, const LexemeType closeBracketType) const
		{
			size_t depth{ 0 };

			for (auto it{ start }; it != end; ++it) {
				if (it->type == openBracketType) {
					++depth;
				}
				else if (it->type == closeBracketType) {
					if (--depth == 0)
						return it;
				}
			}

			return end;
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

		std::vector<primitive> getNextPrimitivesFrom(const_iterator& iterator)
		{
			const auto lex{ *iterator };

			switch (lex.type) {
			case LexemeType::Semicolon:
				return{ { PrimitiveTokenType::Separator, lex } };
			case LexemeType::Colon: [[fallthrough]];
			case LexemeType::Equal:
				return{ { PrimitiveTokenType::Setter, lex } };
			case LexemeType::Comma:
				return{ { PrimitiveTokenType::TermSeparator, lex } };
			case LexemeType::Operator: // resolve operator:
				switch (lex.text.front()) {
				case '+':
					return{ { PrimitiveTokenType::Add, lex } };
				case '-':
					return{ { PrimitiveTokenType::Subtract, lex } };
				case '*':
					return{ { PrimitiveTokenType::Multiply, lex } };
				case '/':
					return{ { PrimitiveTokenType::Divide, lex } };
				case '%':
					return{ { PrimitiveTokenType::Modulo, lex } };
				case '!':
					return{ { PrimitiveTokenType::Factorial, lex } };
				case '|': {
					// TODO: Add absolute value handling ("|a+b|")
					//       Must be adjacent and have a closing char to be an abs

					//if (auto it{ iterator + 1 }; lex.isAdjacentTo(*it)) {
					//	long depth{ 0 };
					//	for (auto prev_it{ iterator }; it != end; ++it, ++prev_it) {
					//		if (!depth && !it->isAdjacentTo(*prev_it))
					//			break; //< there was non-enclosed whitespace
					//		switch (it->type) {
					//		case LexemeType::ParenthesisOpen:
					//			++depth;
					//			break;
					//		case LexemeType::ParenthesisClose:
					//			if (--depth < 0)
					//				goto BREAK;
					//			break;
					//		case LexemeType::Operator:
					//			if (it->text.front() == '|')
					//			break;
					//		}
					//	}
					//BREAK:
					//}
					return{ { PrimitiveTokenType::BitOR, lex } };
				}
				case '&':
					return{ { PrimitiveTokenType::BitAND, lex } };
				case '^':
					return{ { settings.enableBitwiseXOR ? PrimitiveTokenType::BitXOR : PrimitiveTokenType::Exponent, lex } };
				case '~':
					return{ { PrimitiveTokenType::BitNOT, lex } };
				default:
					throw make_exception("primitive_tokenizer::getNextPrimitive():  No implementation available for operator type \"", lex.text, '\"');
				}
			case LexemeType::AngleBracketOpen:
				// left bitshift or less than
				if (iterator != end && (iterator + 1)->type == LexemeType::AngleBracketOpen) {
					// left bitshift
					return{ combine_tokens(PrimitiveTokenType::LeftShift, lex, *++iterator) };
				}
				else {
					// less than
					return{ { PrimitiveTokenType::LessThan, lex } };
				}
			case LexemeType::AngleBracketClose:
				// right bitshift or greater than
				if (iterator != end && (iterator + 1)->type == LexemeType::AngleBracketClose) {
					// right bitshift
					return{ combine_tokens(PrimitiveTokenType::RightShift, lex, *++iterator) };
				}
				else {
					// greater than
					return{ { PrimitiveTokenType::GreaterThan, lex } };
				}
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
			case LexemeType::Alpha:
			{ // function or variable
				if (const auto& nextNonAlpha{ findFirstNonAdjacentOrNotOfType(iterator, LexemeType::Alpha) };
					nextNonAlpha != end && nextNonAlpha->type == LexemeType::ParenthesisOpen
					&& (nextNonAlpha == begin || nextNonAlpha->isAdjacentTo(*(nextNonAlpha - 1)))) {
					// is a function
					std::vector<primitive> functionSegments{
						combine_tokens(PrimitiveTokenType::FunctionName, getRange(iterator, nextNonAlpha))
					};

					// add the function param open token
					functionSegments.emplace_back(primitive{ PrimitiveTokenType::FunctionParamsOpen, *nextNonAlpha });

					// get pointer for function param close
					const auto& paramEndBracket{ findEndBracket(nextNonAlpha, LexemeType::ParenthesisOpen, LexemeType::ParenthesisClose) };

					if (paramEndBracket == end)
						throw make_exception(make_error_message_from(nextNonAlpha, "UNMATCHED", "Syntax Error: Function \"", functionSegments.front().text, "\" has unmatched opening bracket!"));

					// get the lexemes inside of the brackets (if there are any)
					if (std::distance(nextNonAlpha, paramEndBracket) > 1) {
						// Recursively tokenize the inner lexemes
						const auto inner{ primitive_tokenizer{ getRange(nextNonAlpha + 1, paramEndBracket) }.tokenize() }; //< RECURSE
						functionSegments.insert(functionSegments.end(), inner.begin(), inner.end());
					}

					functionSegments.emplace_back(primitive{ PrimitiveTokenType::FunctionParamsClose, *paramEndBracket });
					iterator = paramEndBracket; //< update the iterator

					return functionSegments;
				}
				else {
					// is a variable
					std::vector<primitive> variables;
					variables.reserve(std::distance(iterator, nextNonAlpha));

					for (; current != nextNonAlpha; ++current) {
						variables.emplace_back(primitive{ PrimitiveTokenType::Variable, *current });
					}
					// no need to shrink since the returned vector is temporary
					return std::move(variables);
				}
				break;
			}
			case LexemeType::ParenthesisOpen:
			{
				std::vector<primitive> tokens{};

				const auto closeBracket{ findEndBracket(iterator, LexemeType::ParenthesisOpen, LexemeType::ParenthesisClose) };

				if (closeBracket == end)
					throw make_exception(make_error_message_from(iterator, "UNMATCHED", "Syntax Error: Unmatched opening bracket!"));

				tokens.emplace_back(primitive{ PrimitiveTokenType::ExpressionOpen, *iterator });

				const auto inner{ primitive_tokenizer{ getRange(iterator + 1, closeBracket) }.tokenize() };

				tokens.insert(tokens.end(), inner.begin(), inner.end());

				tokens.emplace_back(primitive{ PrimitiveTokenType::ExpressionClose, *closeBracket });

				iterator = closeBracket; //< update the iterator

				return tokens;
			}
			case LexemeType::ParenthesisClose:
				throw make_exception(make_error_message_from(iterator, "UNMATCHED", "Syntax Error: Unmatched closing bracket!"));
			default:
				break;
			}
			return{ { PrimitiveTokenType::Unknown, lex } };
		}

	public:
		primitive_tokenizer(std::vector<lexeme> const& lexemes) : lexemes{ lexemes }, begin{ this->lexemes.begin() }, end{ this->lexemes.end() }, current{ this->lexemes.begin() } {}

		/// @brief	Tokenizes the lexeme buffer into a vector of primitive tokens.
		std::vector<primitive> tokenize()
		{
			std::vector<primitive> vec{};
			if (lexemes.empty()) return vec; //< if there aren't any lexemes, short circuit

			// reserve enough space for 1:1 token count. Allow reallocations on exceeding this limit tho
			vec.reserve(lexemes.size());

			// tokenize all of the lexemes into primitives
			for (; current != end; ++current) {
				if (current->type == LexemeType::_EOF) break;

				const auto tokens{ getNextPrimitivesFrom(current) };
				vec.insert(vec.end(), tokens.begin(), tokens.end());
			}

			// remove unused space & return
			vec.shrink_to_fit();
			return vec;
		}
	};
}
