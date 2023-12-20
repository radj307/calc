#pragma once
#include <var.hpp>				//< for concepts
#include <charcompare.hpp>		//< for stdpred noexcept
#include <make_exception.hpp>	//< for make_exception

#include <cstdint>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <optional>

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
		// ([A-Za-z]) An upper or lower-case letter.
		Alpha,
		// ([1-9]+) An integral number.
		IntNumber,
		// ([1-9][0-9]+.[0-9]+) A floating-point number.
		RealNumber,
		// ("0b[01]+") A binary number. Binary numbers always start with "0b".
		BinaryNumber,
		// (0[0-7]+) An octal number. Octal numbers always start with "0".
		OctalNumber,
		// (0x[0-9A-Fa-f]+) A hexadecimal number. Hex numbers always start with "0x".
		HexNumber,
		// ('.') A period character
		Period,
		// (',') A comma character
		Comma,
		// ('<') An opening angle bracket / less than symbol
		AngleBracketOpen,
		// ('>') A closing angle bracket / greater than symbol
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

		// A number literal.
		Number,
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
		Unknown,


		// find median

		// stats functions
	};

	// tokenizers:

	namespace token {
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
			std::string value;

			constexpr basic_token() = default;
			/**
			 * @brief				Creates a new token instance with the specified type, starting position, and string value.
			 * @param type		  - The type of token associated with the string value.
			 * @param position	  - The starting position of the string value in the input stream.
			 * @param strValue	  - The string value of the token.
			 */
			constexpr basic_token(const type_t& type, const auto position, const std::string& strValue) : type{ type }, pos{ static_cast<std::streamoff>(position) }, value{ strValue } {}
			constexpr basic_token(const type_t& type, const auto position, const char charValue) : type{ type }, pos{ static_cast<std::streamoff>(position) }, value{ charValue } {}

			/// @brief	Gets the (exlusive) ending position of this token.
			constexpr auto getEndPos() const noexcept { return pos + value.size(); }

			constexpr bool isAdjacentTo(const std::streamoff position) const noexcept
			{
				return position + 1 == pos
					|| getEndPos() == position;
			}
			constexpr bool isAdjacentTo(const basic_token<TTokenType>& other) const noexcept
			{
				return other.getEndPos() == pos
					|| getEndPos() == other.pos;
			}

			friend constexpr bool operator==(const basic_token<TTokenType>& l, const basic_token<TTokenType>& r)
			{
				return l.pos == r.pos && l.type == r.type && l.value == r.value;
			}
			friend constexpr bool operator!=(const basic_token<TTokenType>& l, const basic_token<TTokenType>& r)
			{
				return l.pos != r.pos || l.type != r.type || l.value != r.value;
			}

			friend std::ostream& operator<<(std::ostream& os, const basic_token<type_t>& tkn)
			{
				return os << tkn.value;
			}
		};

		/**
		 * @brief					Combines the specified tokens to create a single new token instance with the specified type.
		 * @tparam FILL_CHAR	  -	The character to use as a separator when combining token string values.
		 *							 Defaults to a space ' '.
		 * @tparam TResultType	  -	The token type of the resulting token.
		 * @tparam TResultToken	  -	The type of the resulting token.
		 * @param type			  -	The token type of the resulting token instance.
		 * @param ...tokens		  -	At least one basic_token to combine into a new token.
		 * @returns					A new token of the specified type.
		 */
		template<char FILL_CHAR, typename TResultType, std::derived_from<basic_token<TResultType>> TResultToken = basic_token<TResultType>>
		constexpr TResultToken combine_tokens(TResultType type, auto&&... tokens)
		{
			size_t startPos{ static_cast<size_t>(-1) };
			std::stringstream buf;
			size_t endPos{ startPos };

			([&]() {
				using TToken = std::decay_t<decltype(tokens)>;
			static_assert(var::derived_from_templated<TToken, basic_token>, "create_token does not accept inputs that are not tokens!");

			auto pos{ std::forward<TToken>(tokens).pos };

			// use the smallest pos value as the beginning of the resulting token
			if (pos < startPos)
				startPos = std::forward<TToken>(tokens).pos;

			// pad out the gap between tokens with spaces
			while (pos > endPos) {
				buf << ' ';
				++endPos;
			}

			buf << std::forward<TToken>(tokens);

			endPos = std::forward<TToken>(tokens).getEndPos();
			}(), ...);

			return{ type, startPos, buf.str() };
		}
		template<typename TResultType, std::derived_from<basic_token<TResultType>> TResultToken = basic_token<TResultType>>
		constexpr TResultToken combine_tokens(TResultType type, auto&&... tokens) { return combine_tokens<' '>(type, $fwd(tokens)...); }

		/// @brief	A lexeme token, the most basic kind of token subtype.
		using lexeme = basic_token<LexemeType>;
		/// @brief	A primitive token, one step up from a lexeme but not a full token.
		using primitive = basic_token<PrimitiveTokenType>;
		/// @brief	A token.
		using token = basic_token<TokenType>;

		/// @brief	Lexical tokenizer that converts text input into lexemes.
		class lexer {
			/// @brief	A stream position class that is loosely interchangeable with integers.
			using pos_t = std::streampos;
			/// @brief	A stream offset value. (int64 / long long)
			using off_t = std::streamoff;

		public:
			bool throwOnUnknownLexeme{ false };
		protected:
			// The underlying buffer stream.
			std::stringstream sbuf;
			// The previous position of the getter.
			pos_t lastPos{ 0 };

			/// @brief			Sets lastPos to the specified position.
			/// @param pos	  - The target position relative to the beginning of the stream.
			/// @returns		The previous lastPos value.
			inline auto setLastPos(const pos_t pos)
			{
				const auto previousValue{ lastPos };
				lastPos = pos;
				return previousValue;
			}
			/// @brief			Sets lastPos to the current position of the getter.
			/// @returns		The previous lastPos value.
			inline auto setLastPosHere()
			{
				return setLastPos(getPos());
			}
			/// @brief	Sets the current getter position to the lastPos.
			inline auto restoreLastPos()
			{
				setPos(lastPos);
			}

			/**
			 * @brief		Checks if the buffer is in a good state and more data is available.
			 * @returns		true when no error bits are set; otherwise, false.
			 */
			[[nodiscard]] inline bool good() const { return sbuf.good(); }
			/**
			 * @brief		Checks if the buffer has suffered from an unrecoverable loss of integrity.
			 *				If the badbit is set, further operations should not be attempted on the buffer.
			 * @returns		true when the badbit is set; otherwise, false.
			 */
			[[nodiscard]] inline bool bad() const { return sbuf.bad(); }
			/**
			 * @brief		Checks if an error has occurred in the buffer.
			 *				The failbit indicates a non-critical I/O error; it can be unset using unsetf(std::ios::failbit).
			 * @returns		true when the failbit is set; otherwise, false.
			 */
			[[nodiscard]] inline bool fail() const { return sbuf.fail(); }
			/**
			 * @brief		Checks if the getter position has reached the end of the buffer's available data.
			 * @returns		true when the eofbit is set; otherwise, false.
			 */
			[[nodiscard]] inline bool eof() const { return sbuf.eof(); }

			/// @brief		Sets the specified flag(s) in the buffer stream.
			/// @returns	The previous flags.
			inline auto setf(std::ios_base::fmtflags const& formatFlags)
			{
				return sbuf.setf(formatFlags);
			}
			/// @brief		Unsets the specified flag(s) in the buffer stream.
			inline void unsetf(std::ios_base::fmtflags const& formatFlags)
			{
				sbuf.unsetf(formatFlags);
			}

			/// @brief			Sets the getter position to the specified position.
			/// @param pos    - The target position relative to the beginning of the stream.
			inline void setPos(const pos_t pos)
			{
				sbuf.seekg(pos);
			}
			/// @brief				Moves the getter position by the specified relative offset.
			/// @param offset     - The number of positions to move by.
			/// @param relativeTo - The anchor point that the offset is relative to.
			///						 (std::ios::cur) is the current position, negative goes back, positive goes forward;
			///						 (std::ios::beg) is the beginning of the stream;
			///						 (std::ios::end) is the end of the stream.
			inline void movePos(const pos_t offset, const std::ios_base::seekdir relativeTo = std::ios::cur)
			{
				sbuf.seekg(offset, relativeTo);
			}
			/// @brief		Returns the current getter position.
			/// @returns	The current getter position relative to the beginning of the stream.
			inline pos_t getPos()
			{
				return sbuf.tellg();
			}
			/// @brief		Returns the current getter position as an actual integer type rather than a streampos.
			/// @returns	The current getter position relative to the beginning of the stream.
			inline off_t getOff()
			{
				return static_cast<off_t>(getPos());
			}

			/// @brief				Gets the next char in the stream without changing the getter position.
			/// @param setLastPos - When true, this is the same as calling setLastPosHere(). Defaults to false.
			[[nodiscard]] char peekNextChar(const bool setLastPos = false)
			{
				if (setLastPos) setLastPosHere();

				return static_cast<char>(sbuf.peek());
			}
			/// @brief				Gets the next char in the stream.
			/// @param setLastPos - When true, sets lastPos prior to moving to the target position. Defaults to false.
			[[nodiscard]] char getNextChar(const bool setLastPos = false)
			{
				if (setLastPos) setLastPosHere();

				char c;
				sbuf.get(c);
				return c;
			}

			/// @brief				Returns the char at the specified position in the stream without changing the getter position.
			/// @param pos		  - The target position relative to the beginning of the stream.
			/// @param setLastPos - When true, this is the same as calling setLastPosHere(). Defaults to false.
			[[nodiscard]] char peekCharAt(const pos_t pos, const  bool setLastPos = false)
			{
				if (setLastPos) setLastPosHere();

				const pos_t currentPos{ getPos() };

				// move to target pos
				setPos(pos);

				// get target char
				const auto c{ peekNextChar() };

				// restore pos
				setPos(currentPos);

				return c;
			}
			/// @brief				Returns the char at the specified position in the stream.
			/// @param pos		  -	The target position relative to the beginning of the stream.
			/// @param setLastPos - When true, sets lastPos prior to moving to the target position. Defaults to false.
			[[nodiscard]] char getCharAt(const pos_t pos, const bool setLastPos = false)
			{
				if (setLastPos) setLastPosHere();

				// move to target pos
				setPos(pos);

				return getNextChar();
			}

			/// @brief				Returns the char at the specified offset, relative to the current getter position.
			/// @param pos		  - The offset from the current getter position. Negative goes backwards, positive goes forwards.
			/// @param setLastPos - When true, this is the same as calling setLastPosHere(). Defaults to false.
			[[nodiscard]] char peekCharOff(const off_t offset, bool setLastPos = false)
			{
				return peekCharAt(getPos() + offset, setLastPos);
			}
			/// @brief				Returns the char at the specified offset, relative to the current getter position.
			/// @param pos		  - The offset from the current getter position. Negative goes backwards, positive goes forwards.
			/// @param setLastPos - When true, sets lastPos prior to moving to the target position. Defaults to false.
			[[nodiscard]] char getCharOff(const off_t offset, bool setLastPos = false)
			{
				return getCharAt(getPos() + offset, setLastPos);
			}

			/// @brief				Returns the specified predicate's result for the next char in the stream without changing the getter position.
			/// @returns			true when the predicate returned true for the next char; otherwise, false.
			[[nodiscard]] bool nextCharIs(const std::function<bool(char)>& predicate)
			{
				return predicate(peekNextChar());
			}
			/// @brief				Checks if the next char in the stream matches any one of the specified predicates, without changing the getter position.
			/// @returns			true when any predicate returned true for the next char; otherwise, false.
			template<std::same_as<std::function<bool(char)>>... Ts>
			[[nodiscard]] bool nextCharIsAny(Ts&&... predicates)
			{
				const auto next{ peekNextChar() };
				return var::variadic_or(predicates(next)...);
			}

			/// @brief				Eats the next char in the stream. Useful when it was already retrieved with peekNextChar().
			/// @param setLastPos - When true, sets lastPos prior to moving to the target position. Defaults to false.
			void skipNextChar(const bool setLastPos = false)
			{
				(void)getNextChar(setLastPos);
			}

			/// @brief				Undoes the last call to getNextChar().
			void ungetChar() { sbuf.unget(); }

			std::string getWhile(const std::function<bool(char)>& predicate, const bool includeEof = false, const bool setLastPos = false)
			{
				if (setLastPos) setLastPosHere();

				std::string buf{};

				for (char c{}; true;) {
					c = getNextChar();

					if (const bool atEof{ eof() };
						(!includeEof && atEof) || !predicate(c)) {
						// predicate failed, put this char back in the stream and return
						ungetChar();
						break;
					}
					// if we've reached eof and the predicate explicitly allows it, include it in return
					else if (atEof) break;

					buf += c;
				}

				return buf;
			}

		public:
			/// @brief	Creates a new lexer instance without any buffered data.
			lexer() = default;
			/// @brief	Creates a new lexer instance with the specified data.
			lexer(std::stringstream&& ss, const bool throwOnUnknownLexeme = false) : throwOnUnknownLexeme{ throwOnUnknownLexeme }, sbuf{ std::move(ss) } {}
			lexer(std::string&& str, const bool throwOnUnknownLexeme = false) : throwOnUnknownLexeme{ throwOnUnknownLexeme }, sbuf{ std::move(str) } {}

			lexeme getNextLexeme()
			{
			GET_NEXT_LEXEME:
				const auto pos{ getOff() };
				char c{ getNextChar(true) };

				if (eof() || c == EOF)
					return{ LexemeType::_EOF, pos, c };
				else if (str::stdpred::isalpha(c)) {
					// alpha; continue getting until the first non-alpha char
					std::string buf{};
					buf += c;
					buf += getWhile(str::stdpred::isalpha);
					return{ LexemeType::Alpha, pos, buf };
				}
				// parse number types
				else if (str::stdpred::isdigit(c)) {
					// digit; parse as a number
					std::string buf{};
					buf += c;

					c = peekNextChar();

					if (!eof()) {
						// handle binary numbers
						if (c == 'b') {
							// increment the getter position past the 'b' (c is already set to the result of getNextChar() since we peeked)
							buf += getNextChar();
						GET_NEXT_BINARY_SEGMENT:
							buf += getWhile(str::isbinarydigit);
							if ((c = peekNextChar()) == '_') {
								if (const auto nextNextChar{ peekCharOff(1) }; nextNextChar == '0' || nextNextChar == '1') {
									// add underscore to the buffer and increment the getter position
									buf += c;
									skipNextChar();
									goto GET_NEXT_BINARY_SEGMENT;
								}
							}

							return{ LexemeType::BinaryNumber, pos, buf };
						}
						// handle hexadecimal numbers
						else if (c == 'x') {
							// increment the getter position past the 'b' (c is already set to the result of getNextChar() since we peeked)
							buf += getNextChar();
							buf += getWhile(str::ishexdigit);

							return{ LexemeType::HexNumber, pos, buf };
						}
						// handle octal, integral, and real numbers
						if (bool hasDecimalPoint{ c == '.' }, has8DigitOrHigher{ c == '8' || c == '9' };
							hasDecimalPoint || has8DigitOrHigher || (c >= '0' && c <= '7')) {
							// increment the getter position past the current char (c is already set to the result of getNextChar() since we peeked)
							buf += getNextChar();

							while (nextCharIs([](auto&& ch) -> bool { return $fwd(ch) == '.' || $fwd(ch) == ',' || str::stdpred::isdigit($fwd(ch)); })) {
								c = getNextChar();

								if (c == '.')
									hasDecimalPoint = true;
								else if (c == '8' || c == '9')
									has8DigitOrHigher = true;

								buf += c;
							}

							if (hasDecimalPoint)
								return{ LexemeType::RealNumber, pos, buf }; //< floating-point
							else if (buf.at(0) == '0' && !has8DigitOrHigher)
								return{ LexemeType::OctalNumber, pos, buf }; //< octal
							else
								return{ LexemeType::IntNumber, pos, buf }; //< integer								
						}
						// else fall through
					}
					return{ (c == '0' ? LexemeType::OctalNumber : LexemeType::IntNumber), pos, buf };
				}

				switch (c) {
				case '\\':
					return{ LexemeType::Escape, pos, std::string{ c, getNextChar() } };
				case '=':
					return{ LexemeType::Equal, pos, c };
				case '+':
					return{ LexemeType::Add, pos, c };
				case '-':
					return{ LexemeType::Subtract, pos, c };
				case '*':
					return{ LexemeType::Multiply, pos, c };
				case '/':
					return{ LexemeType::Divide, pos, c };
				case '%':
					return{ LexemeType::Percent, pos, c };
				case ' ': case '\t': case '\v': case '\r': case '\n':
					goto GET_NEXT_LEXEME; //< skip whitespace characters
				case '.':
					return{ LexemeType::Period, pos, c };
				case ',':
					return{ LexemeType::Comma, pos, c };
				case '<':
					return{ LexemeType::AngleBracketOpen, pos, c };
				case '>':
					return{ LexemeType::AngleBracketClose, pos, c };
				case '(':
					return{ LexemeType::BracketOpen, pos, c };
				case ')':
					return{ LexemeType::BracketClose, pos, c };
				default:
					if (throwOnUnknownLexeme)
						throw make_exception("lexer::getNextLexeme(): Character \"", c, "\" at pos (", pos, ") is not a recognized lexeme!");
					return{ LexemeType::Unknown, pos, c };
				}
			}

			std::vector<lexeme> get_lexemes(bool includeEofLexeme = true, bool startFromBeginning = false)
			{
				std::vector<lexeme> lexemes;

				// move to the beginning of the stream, if specified
				if (startFromBeginning)
					movePos(0, std::ios::beg);
				// short circuit and return empty vector if stream is at eof
				else if (eof())
					return lexemes;

				// get the remaining length of the stream
				const auto pos{ getPos() };
				movePos(0, std::ios::end);
				const auto len{ getPos() };
				setPos(pos);

				// reserve enough space for every following char to be a lexeme
				lexemes.reserve(len - pos);

				// get all remaining lexemes
				while (!eof()) {
					lexemes.emplace_back(getNextLexeme());
				}

				// remove the EOF token if specified
				if (lexemes.size() > 0 && !includeEofLexeme && lexemes.back().type == LexemeType::_EOF) {
					lexemes.pop_back();
				}

				// shrink the final vector to fit actual contents
				lexemes.shrink_to_fit();
				return lexemes;
			}

			/// @brief	Reads data from the input stream into the buffer.
			friend std::istream& operator>>(std::istream& is, lexer& tokenizer)
			{
				tokenizer.sbuf << is.rdbuf();
				return is;
			}
			/// @brief	Writes data from the buffer into the output stream.
			friend std::ostream& operator<<(std::ostream& os, const lexer& tokenizer)
			{
				return os << tokenizer.sbuf.rdbuf();
			}
		};

		/// @brief	Primitive tokenizer that converts lexemes into primitive tokens.
		class primitive_tokenizer {
			std::vector<lexeme> lexemes;
			size_t lastPos{ 0 };

		public:
			primitive_tokenizer(std::vector<lexeme>&& lexemes) : lexemes{ std::move(lexemes) } {}

			/// @brief	Tokenizes the lexeme buffer into a vector of primitive tokens.
			std::vector<primitive> tokenize()
			{
				std::vector<primitive> vec{};

				// ...

				vec.shrink_to_fit();
				return vec;
			}
		};
	}
}
