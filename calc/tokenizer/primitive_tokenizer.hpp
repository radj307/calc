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

	protected:
		const std::vector<lexeme> lexemes;
		const_iterator begin;
		const_iterator end;
		const_iterator current;
		FunctionMap const* const functionMap{ nullptr };

		template<size_t INDENT = 10, var::streamable... Ts>
		std::string make_error_message_from(const_iterator const& iterator, Ts&&... message)
		{
			std::stringstream ss;

			const auto expr_str{ stringify_tokens(lexemes.begin(), lexemes.back().type == LexemeType::_EOF ? lexemes.end() - 1 : lexemes.end()) };
			ss << expr_str << '\n';
			ss << indent(INDENT) << csync(color::dark_red) << indent(iterator->pos, 0, '~') << csync(color::red) << '^' << csync(color::dark_red) << indent(expr_str.size(), iterator->getEndPos(), '~') << csync() << '\n';

			if (sizeof...(Ts) > 0) {
				ss << indent(INDENT);
				(ss << ... << message);
				ss << '\n';
			}

			return ss.str();
		}

		/// @returns	true when functionName is a function in the functionMap; otherwise, false.
		bool isFunctionName(std::string const& functionName) const noexcept
		{
			return functionMap != nullptr
				&& functionMap->isFunction(functionName);
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
						&& (previousPrimitive == nullptr || !evaluates_to_number(previousPrimitive->type))
						&& evaluates_to_number(next->type)) {
						// is negate operator
						return{ { PrimitiveTokenType::Negate, lex } };
					}
					else // is subtract operator
						return{ { PrimitiveTokenType::Subtract, lex } };

					//if (const auto& next{ (iterator + 1) };
					//	next != end && (previousPrimitive == nullptr
					//					|| previousPrimitive->type == PrimitiveTokenType::TermSeparator
					//					|| OperatorPrecedence::IsOperator(previousPrimitive->type))) {
					//	// handle negative numbers
					//	switch (next->type) {
					//	case LexemeType::IntNumber:
					//		++iterator;
					//		return{ combine_tokens(PrimitiveTokenType::IntNumber, lex, *next) };
					//	case LexemeType::RealNumber:
					//		++iterator;
					//		return{ combine_tokens(PrimitiveTokenType::RealNumber, lex, *next) };
					//	}
					//}
					//return{ { PrimitiveTokenType::Subtract, lex } };
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
			case LexemeType::Alpha:
			{ // function, variable, or boolean literal
				const auto& nextNonAlpha{ findFirstNonAdjacentOrNotOfType(iterator, LexemeType::Alpha, LexemeType::Underscore) };
				if (nextNonAlpha != end) {
					// stringify adjacent alpha tokens
					const auto alphaStr{ stringify_tokens<false>(iterator, nextNonAlpha) };

					if (alphaStr == "true" || alphaStr == "false") {
						// is boolean literal

						iterator = nextNonAlpha - 1; //< update the iterator

						return{ { PrimitiveTokenType::Boolean, lex.pos, alphaStr } };
					}
					else if (nextNonAlpha->type == LexemeType::ParenthesisOpen && isFunctionName(alphaStr)) {
						// is function name
						std::vector<primitive> functionSegments{
							combine_tokens(PrimitiveTokenType::FunctionName, getRange(iterator, nextNonAlpha))
						};

						// add the function param open token
						functionSegments.emplace_back(primitive{ PrimitiveTokenType::ExpressionOpen, *nextNonAlpha });

						// get pointer for function param close
						const auto& paramEndBracket{ findEndBracket(nextNonAlpha, LexemeType::ParenthesisOpen, LexemeType::ParenthesisClose) };

						if (paramEndBracket == end)
							throw make_exception(make_error_message_from(nextNonAlpha, "Syntax Error: Function \"", functionSegments.front().text, "\" has unmatched opening bracket!"));

						// get the lexemes inside of the brackets (if there are any)
						if (std::distance(nextNonAlpha, paramEndBracket) > 1) {
							// Recursively tokenize the inner lexemes
							const auto inner{ primitive_tokenizer{ getRange(nextNonAlpha + 1, paramEndBracket), functionMap }.tokenize() }; //< RECURSE
							functionSegments.insert(functionSegments.end(), inner.begin(), inner.end());
						}

						functionSegments.emplace_back(primitive{ PrimitiveTokenType::ExpressionClose, *paramEndBracket });
						iterator = paramEndBracket; //< update the iterator

						return functionSegments;
					}
				}

				// is a variable
				std::vector<primitive> variables;
				variables.reserve(std::distance(iterator, nextNonAlpha));

				for (; current != nextNonAlpha; ++current) {
					variables.emplace_back(primitive{ PrimitiveTokenType::Variable, *current });
				}

				--current;

				// no need to shrink since the returned vector is temporary
				return std::move(variables);
			}
			case LexemeType::ParenthesisOpen:
			{
				std::vector<primitive> tokens{};

				const auto closeBracket{ findEndBracket(iterator, LexemeType::ParenthesisOpen, LexemeType::ParenthesisClose) };

				if (closeBracket == end)
					throw make_exception(make_error_message_from(iterator, "Syntax Error: Unmatched opening bracket!"));

				tokens.emplace_back(primitive{ PrimitiveTokenType::ExpressionOpen, *iterator });

				const auto inner{ primitive_tokenizer{ getRange(iterator + 1, closeBracket), functionMap }.tokenize() };

				tokens.insert(tokens.end(), inner.begin(), inner.end());

				tokens.emplace_back(primitive{ PrimitiveTokenType::ExpressionClose, *closeBracket });

				iterator = closeBracket; //< update the iterator

				return tokens;
			}
			case LexemeType::ParenthesisClose:
				throw make_exception(make_error_message_from(iterator, "Syntax Error: Unmatched closing bracket!"));
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

			// reserve enough space for 1:1 token count. Allow reallocations on exceeding this limit tho
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
