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
#include <variant>

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

	/**
	 * @brief	Intermediate unit in a tokenized math expression.
	 *			These are one step above a lexeme, and simpler than a full token.
	 */
	enum class PrimitiveTokenType : std::uint8_t {
		// An unrecognized or null primitive token.
		Unknown,

		// OPERAND TYPES:

		// A variable name.
		VariableName,
		// A sub-expression, consisting of an opening bracket, operands, and an operator. Flow control.
		Expression,
		// A function expression.
		Function,
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
		// Factorial operator.
		Factorial,
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

		NumberLiteral,

		// find median

		// stats functions
	};

	// tokenizers:

	namespace tkn {
		/**
		 * @brief				A basic token object with the specified token type.
		 * @tparam TTokenType -	The type of token contained by this instance.
		 */
		template<var::any_same<LexemeType, PrimitiveTokenType, TokenType> TTokenType>
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
			template<var::any_same<LexemeType, PrimitiveTokenType, TokenType> T>
			constexpr basic_token(const type_t& type, basic_token<T> const& otherToken) : type{ type }, pos{ otherToken.pos }, text{ otherToken.text } {}

			/// @brief	Gets the (exlusive) ending position of this token.
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

			friend std::ostream& operator<<(std::ostream& os, const basic_token<type_t>& tkn) { return os << tkn.text; }
		};

	#pragma region combine_tokens
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
		template<char FILL_CHAR, typename TResultType, std::derived_from<basic_token<TResultType>> TResultToken = basic_token<TResultType>, var::streamable... Ts>
		constexpr TResultToken combine_tokens(TResultType const& type, Ts&&... tokens)
		{
			size_t startPos{ static_cast<size_t>(EOF) };
			size_t endPos{ 0ull };
			std::stringstream buf;

			([&]() {
				static_assert(var::derived_from_templated<decltype(tokens), basic_token>, "create_token does not accept inputs that are not tokens!");

			std::streamoff pos{ $fwd(tokens).pos };

			// use the smallest pos value as the beginning of the resulting token
			if (pos < startPos)
				startPos = $fwd(tokens).pos;

			// pad out the gap between tokens with spaces
			while (pos > endPos) {
				buf << ' ';
				++endPos;
			}

			buf << $fwd(tokens);

			endPos = $fwd(tokens).getEndPos();
			 }(), ...);

			return{ type, startPos, buf.str() };
		}
		/**
		 * @brief					Combines the specified tokens to create a single new token instance with the specified type.
		 * @tparam TResultType	  -	The token type of the resulting token.
		 * @tparam TResultToken	  -	The type of the resulting token.
		 * @param type			  -	The token type of the resulting token instance.
		 * @param ...tokens		  -	At least one basic_token to combine into a new token.
		 * @returns					A new token of the specified type.
		 */
		template<typename TResultType, std::derived_from<basic_token<TResultType>> TResultToken = basic_token<TResultType>, var::streamable... Ts>
		constexpr TResultToken combine_tokens(TResultType const& type, Ts&&... tokens) { return combine_tokens<' '>(type, $fwd(tokens)...); }

		template<var::derived_from_templated<basic_token> TSource, typename TResultType, std::derived_from<basic_token<TResultType>> TResult = basic_token<TResultType>>
		constexpr TResult combine_tokens(TResultType const& type, std::vector<TSource>&& vec)
		{
			size_t startPos{ static_cast<size_t>(EOF) };
			size_t endPos{ 0ull };
			std::stringstream buf;

			for (const auto& it : $fwd(vec)) {

				auto pos{ it.pos };

				// use the smallest pos value as the beginning of the resulting token
				if (pos < startPos)
					startPos = it.pos;

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

			for (const auto& it : $fwd(vec)) {

				auto pos{ it.pos };

				// use the smallest pos value as the beginning of the resulting token
				if (pos < startPos)
					startPos = it.pos;

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
	#pragma endregion combine_tokens

		/// @brief	A lexeme token, the most basic kind of token subtype.
		using lexeme = basic_token<LexemeType>;
		/// @brief	A primitive token, one step up from a lexeme but not a full token.
		using primitive = basic_token<PrimitiveTokenType>;
		/// @brief	A complex token, the most advanced kind of token subtype.
		using complex = basic_token<TokenType>;

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

		#pragma region Protected Methods

		#pragma region stream state
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
		#pragma endregion stream state

		#pragma region stream format flags
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
		#pragma endregion stream format flags

		#pragma region stream gpos
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
		#pragma endregion stream gpos

		#pragma region peek/get NextChar
			/// @brief				Gets the next char in the stream without changing the getter position.
			[[nodiscard]] char peekNextChar()
			{
				return static_cast<char>(sbuf.peek());
			}
			/// @brief		Gets the next char in the stream.
			/// @returns	The next char in the stream if available; otherwise, EOF.
			[[nodiscard]] char getNextChar()
			{
				if (!good()) return static_cast<char>(EOF);
				char c{};
				sbuf.get(c);
				return c;
			}
			/// @brief	Increments the getter position by one, skipping the next char.
			///          This is useful when you already have the next char from peekNextChar() and want to move the getter to the next char.
			void skipNextChar() { (void)getNextChar(); }
		#pragma endregion peek/get NextChar

		#pragma region peek/get CharAt
			/// @brief			Returns the char at the specified position in the stream without changing the getter position.
			/// @param pos	  -	The target position relative to the beginning of the stream.
			[[nodiscard]] char peekCharAt(const pos_t pos)
			{
				const pos_t currentPos{ getPos() };

				// move to target pos
				setPos(pos);

				// get target char
				const auto c{ peekNextChar() };

				// restore pos
				setPos(currentPos);

				return c;
			}
			/// @brief			Returns the char at the specified position in the stream.
			/// @param pos	  -	The target position relative to the beginning of the stream.
			[[nodiscard]] char getCharAt(const pos_t pos)
			{
				// move to target pos
				setPos(pos);

				return getNextChar();
			}
		#pragma endregion peek/get CharAt

		#pragma region peek/get CharOff
			/// @brief			Returns the char at the specified offset, relative to the current getter position.
			/// @param pos	  - The offset from the current getter position. Negative goes backwards, positive goes forwards.
			[[nodiscard]] char peekCharOff(const off_t offset)
			{
				return peekCharAt(getPos() + offset);
			}
			/// @brief			Returns the char at the specified offset, relative to the current getter position.
			/// @param pos	  - The offset from the current getter position. Negative goes backwards, positive goes forwards.
			[[nodiscard]] char getCharOff(const off_t offset)
			{
				return getCharAt(getPos() + offset);
			}
		#pragma endregion peek/get CharOff

		#pragma region nextCharIs
			/// @brief				Returns the specified predicate's result for the next char in the stream without changing the getter position.
			/// @param predicate  - A predicate function whose return value determines the result of this method.
			/// @returns			true when the predicate returned true for the next char; otherwise, false.
			[[nodiscard]] bool nextCharIs(const std::function<bool(char)>& predicate)
			{
				return predicate(peekNextChar());
			}
		#pragma endregion nextCharIs

		#pragma region ungetChar
			/// @brief	Decrements the getter position by one, making the last char to be extracted from the stream available again.
			void ungetChar() { sbuf.unget(); }
		#pragma endregion ungetChar

		#pragma region getWhile
			/// @brief				Continues getting characters from the stream while the specified predicate returns true.
			/// @param predicate  - A predicate function that accepts a char and returns true to continue or false to break from the loop.
			std::string getWhile(const std::function<bool(char)>& predicate, const std::string& skipOverChars)
			{
				std::string buf;

				// make sure the next char is actually wanted before spinning up the loop
				if (!predicate(peekNextChar())) return buf;

				// push chars allowed by the predicate to the buffer
				for (char c{}; sbuf >> c; ) {
					buf += c;

					// peek at the next char and break if the predicate returns false
					if (!predicate(peekNextChar())) break;
				}

				return buf;
			}
			/// @brief				Continues getting characters from the stream while the specified predicate returns true.
			/// @param predicate  - A predicate function that accepts a char and returns true to continue or false to break from the loop.
			std::string getWhile(const std::function<bool(char)>& predicate)
			{
				std::string buf;

				// make sure the next char is actually wanted before spinning up the loop
				if (!predicate(peekNextChar())) return buf;

				// push chars allowed by the predicate to the buffer
				for (char c{}; sbuf >> c; ) {
					buf += c;

					// peek at the next char and break if the predicate returns false
					if (!predicate(peekNextChar())) break;
				}

				return buf;
			}
		#pragma endregion getWhile

		#pragma endregion Protected Methods

		public:
			/// @brief	Creates a new lexer instance without any buffered data.
			lexer() = default;
			/// @brief	Creates a new lexer instance with the specified data.
			lexer(std::stringstream&& ss, const bool throwOnUnknownLexeme = false) : throwOnUnknownLexeme{ throwOnUnknownLexeme }, sbuf{ std::move(ss) } {}
			lexer(std::string&& str, const bool throwOnUnknownLexeme = false) : throwOnUnknownLexeme{ throwOnUnknownLexeme }, sbuf{ std::move(str) } {}

			lexeme getNextLexeme()
			{
			GET_NEXT_LEXEME:

				const auto pos{ getPos() };
				char c{ getNextChar() };

				// end-of-file (end-of-stream)
				if (eof() || c == EOF)
					return{ LexemeType::_EOF, pos, c };
				// alpha
				else if (str::stdpred::isalpha(c) || c == '_')
					return{ LexemeType::Alpha, pos, c }; //< don't coalesce alpha; we'll do that during expression resolution
				// numbers
				else if (str::stdpred::isdigit(c)) {
				PARSE_NUMBER:
					// digit; parse as a number
					std::string buf{};
					buf += c;

					const bool startsWithZero{ c == '0' };

					// peek at the next char
					c = peekNextChar();

					if (good()) {
						// Only numbers that start with zero can be binary or hex; see "0b"/"0x"; 
						if (startsWithZero) {
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
							//v fallthrough
						}
						// handle octal, integral, and real numbers
						if (bool hasDecimalPoint{ c == '.' || buf.at(0) == '.' }, has8DigitOrHigher{ c == '8' || c == '9' };
							hasDecimalPoint || has8DigitOrHigher || (c >= '0' && c <= '7')) {
							// increment the getter position past the current char (c is already set to the result of getNextChar() since we peeked)
							buf += getNextChar();

							// keep getting characters until we reach the end of the number
							while (nextCharIs([](auto&& ch) -> bool { return $fwd(ch) == '.' || $fwd(ch) == ',' || $fwd(ch) == '_' || str::stdpred::isdigit($fwd(ch)); })) {
								c = getNextChar();

								if (c == '.') {
									if (!hasDecimalPoint)
										hasDecimalPoint = true;
									else {
										// we already found a decimal point ; this one is FAKE
										ungetChar();
										break;
									}
								}
								else if (hasDecimalPoint && c == ',') {
									// we already found a decimal point, so why is there a comma?
									//  Consider it a separator; put it back and stop reading more chars
									ungetChar();
									break;
								}
								else if (c == '8' || c == '9')
									has8DigitOrHigher = true;

								buf += c;
							}

							if (hasDecimalPoint)
								return{ LexemeType::RealNumber, pos, buf }; //< floating-point
							else if (startsWithZero && !has8DigitOrHigher)
								return{ LexemeType::OctalNumber, pos, buf }; //< octal
							else
								return{ LexemeType::IntNumber, pos, buf }; //< integer								
						}
						//v fallthrough
					}
					// return octal/integer number
					return{ (startsWithZero ? LexemeType::OctalNumber : LexemeType::IntNumber), pos, buf };
				}

				switch (c) {
				case '\\':
					return{ LexemeType::Escape, pos, std::string{ c, getNextChar() } };
				case '=':
					return{ LexemeType::Equal, pos, c };
				case ':':
					return{ LexemeType::Colon, pos, c };
				case ';':
					return{ LexemeType::Semicolon, pos, c };
				case '+': // add
				case '-': // subtract
				case '*': // multiply
				case '/': // divide
				case '%': // modulo
				case '!': // negate binary value
				case '|': // bitwise OR
				case '&': // bitwise AND
				case '^': // bitwise XOR
				case '~': // bitwise NOT
					return{ LexemeType::Operator, pos, c };
				case ' ': case '\t': case '\v': case '\r': case '\n':
					goto GET_NEXT_LEXEME; //< skip whitespace characters
				case '.':
					if (nextCharIs(str::stdpred::isdigit)) {
						goto PARSE_NUMBER;
					}
					else return{ LexemeType::Period, pos, c };
				case ',':
					return{ LexemeType::Comma, pos, c };
				case '$': //> (Note: this must be escaped in most shells, but it is the most recognizable character for this)
				case '@':
					return{ LexemeType::Macro, pos, c };
				case '<':
					return{ LexemeType::AngleBracketOpen, pos, c };
				case '>':
					return{ LexemeType::AngleBracketClose, pos, c };
				case '[':
					return{ LexemeType::SquareBracketOpen, pos, c };
				case ']':
					return{ LexemeType::SquareBracketClose, pos, c };
				case '(':
					return{ LexemeType::ParenthesisOpen, pos, c };
				case ')':
					return{ LexemeType::ParenthesisClose, pos, c };
				case '{':
					return{ LexemeType::BraceOpen, pos, c };
				case '}':
					return{ LexemeType::BraceClose, pos, c };
				default:
					if (throwOnUnknownLexeme)
						throw make_exception("lexer::getNextLexeme(): Character \"", c, "\" at pos (", pos, ") is not a recognized lexeme!");
					return{ LexemeType::Unknown, pos, c };
				}
			}

			/**
			 * @brief						Tokenizes the input stream into a vector of lexemes.
			 * @param startFromBeginning  - When true, always begins tokenizing lexemes at the stream beginning; otherwise, starts at the current stream getter pos.
			 * @returns						The resulting vector after tokenizing the buffer stream into lexemes.
			 */
			std::vector<lexeme> get_lexemes(bool resetToBeginning = false)
			{
				std::vector<lexeme> lexemes;

				// move to the beginning of the stream, if specified
				if (resetToBeginning)
					movePos(0, std::ios::beg);
				// short circuit and return empty vector if stream getter position is at the end
				else if (eof())
					return lexemes;

				// get the remaining length of the stream
				const auto pos{ getPos() };
				movePos(0, std::ios::end);
				const auto len{ getPos() - pos };
				setPos(pos);

				// reserve enough space for every following char to be a lexeme
				lexemes.reserve(len - pos);

				// get all remaining lexemes
				while (good()) {
					lexemes.emplace_back(getNextLexeme());
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
			using iterator_t = typename std::vector<lexeme>::iterator;
			using const_iterator_t = typename std::vector<lexeme>::const_iterator;

		protected:
			std::vector<lexeme> lexemes;
			const_iterator_t begin;
			const_iterator_t end;
			const_iterator_t current;

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

			bool moveToNext()
			{
				if (current == end) return false;
				++current;
				return true;
			}
			bool moveToPrev()
			{
				if (current == begin) return false;
				--current;
				return true;
			}

			/// @brief	Gets the current lexeme without moving the iterator.
			lexeme getCurrent() const
			{
				return *current;
			}

			lexeme peekNextLexeme() const
			{
				return *(current + 1);
			}
			lexeme getNextLexeme()
			{
				return *++current;
			}

			bool nextLexemeIs(const std::function<bool(lexeme)>& predicate) const
			{
				return hasNext() && predicate(peekNextLexeme());
			}
			bool nextLexemeIs(const LexemeType type) const
			{
				return hasNext() && peekNextLexeme().type == type;
			}

			std::vector<lexeme> peekWhile(const std::function<bool(lexeme)>& predicate, const size_t reserve_size = 8) const
			{
				std::vector<lexeme> vec;
				vec.reserve(reserve_size);

				for (auto it{ current }; hasNext(); ++it) {
					if (!predicate(*it))
						break;

					vec.emplace_back(*it);
				}

				vec.shrink_to_fit();
				return vec;
			}
			std::vector<lexeme> getWhile(const std::function<bool(lexeme)>& predicate, const size_t reserve_size = 8)
			{
				std::vector<lexeme> vec;
				vec.reserve(reserve_size);

				for (; hasNext(); ++current) {
					if (!predicate(*current))
						break;

					vec.emplace_back(*current);
				}

				vec.shrink_to_fit();
				return vec;
			}

		public:
			primitive_tokenizer(const std::vector<lexeme>& lexemes) : lexemes{ lexemes }, begin{ lexemes.begin() }, end{ lexemes.end() }, current{ lexemes.begin() }
			{
				// remove _EOF lexeme(s)
				this->lexemes.erase(std::remove_if(this->lexemes.begin(), this->lexemes.end(), [](auto&& lex) { return lex.type == LexemeType::_EOF; }), this->lexemes.end());
			}
			primitive_tokenizer(std::vector<lexeme>&& lexemes) : lexemes{ std::move(lexemes) }, begin{ lexemes.begin() }, end{ lexemes.end() }, current{ lexemes.begin() }
			{
				// remove _EOF lexeme(s)
				this->lexemes.erase(std::remove_if(this->lexemes.begin(), this->lexemes.end(), [](auto&& lex) { return lex.type == LexemeType::_EOF; }), this->lexemes.end());
			}

			primitive getNextPrimitive()
			{
				const auto lex{ getNextLexeme() };

				switch (lex.type) {
				case LexemeType::Operator:
					switch (lex.text.front()) {
					case '+':
						return{ PrimitiveTokenType::Add, lex };
					case '-':
						return{ PrimitiveTokenType::Subtract, lex };
					case '*':
						return{ PrimitiveTokenType::Multiply, lex };
					case '/':
						return{ PrimitiveTokenType::Divide, lex };
					case '%':
						return{ PrimitiveTokenType::Modulo, lex };
					case '!':
						return{ PrimitiveTokenType::Factorial, lex };
					case '|':
						return{ PrimitiveTokenType::BitOR, lex };
					case '&':
						return{ PrimitiveTokenType::BitAND, lex };
					case '^':
						return{ PrimitiveTokenType::BitXOR, lex };
					case '~':
						return{ PrimitiveTokenType::BitNOT, lex };
					default:
						throw make_exception("primitive_tokenizer::getNextPrimitive():  No implementation available for operator type \"", lex.text, '\"');
					}
					break;
				case LexemeType::BinaryNumber:
					return{ PrimitiveTokenType::BinaryNumber, lex };
				case LexemeType::OctalNumber:
					return{ PrimitiveTokenType::OctalNumber, lex };
				case LexemeType::HexNumber:
					return{ PrimitiveTokenType::HexNumber, lex };
				case LexemeType::IntNumber:
					return{ PrimitiveTokenType::IntNumber, lex };
				case LexemeType::RealNumber:
					return{ PrimitiveTokenType::RealNumber, lex };
				case LexemeType::Alpha:
					// peek ahead
					std::string functionName{};
					std::vector<lexeme> vec;
					vec.reserve(std::distance(current, end));

					size_t bracketDepth{ 0 };
					auto it{ current };
					for (; it != end; ++it) {
						switch (it->type) {
						case LexemeType::ParenthesisOpen:
							if (!isFunctionName(functionName))
								goto BREAK; //< this isn't a function
							++bracketDepth;
							break;
						case LexemeType::ParenthesisClose:
							// reached end of function
							if (--bracketDepth == 0) {
								current = it;
								return combine_tokens(PrimitiveTokenType::Function, vec);
							}
							break;
						case LexemeType::Alpha:
							if (!bracketDepth) // func bracket not reached; append to functionName
								functionName += it->text;
							break;
						default:
							if (!bracketDepth) // func bracket not reached & this lexeme isn't valid; break
								goto BREAK;
							break;
						}

						vec.emplace_back(*it);
					}
					// TODO: Improve this exception v:
					if (bracketDepth != 0)
						throw make_exception("Tokenization Error:  Expected an ending bracket for ");
					// TODO: Improve this exception ^:

				BREAK:
					break;
				}
			}

			/// @brief	Tokenizes the lexeme buffer into a vector of primitive tokens.
			std::vector<primitive> tokenize(bool resetToBeginning = false)
			{
				std::vector<primitive> vec{};
				if (lexemes.empty()) return vec; //< if there aren't any lexemes, short circuit

				// reserve enough space for 1:1 token count. Allow reallocations on exceeding this limit tho
				vec.reserve(lexemes.size());

				// move the iterator to the beginning if specified
				if (resetToBeginning)
					current = begin;

				// tokenize all of the lexemes into primitives
				while (hasNext()) {
					vec.emplace_back(getNextPrimitive());
				}

				// remove unused space & return
				vec.shrink_to_fit();
				return vec;
			}
		};

	}
}
