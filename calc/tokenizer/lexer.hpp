#pragma once
#include "token.hpp"

#include <strcompare.hpp>		//< for str::stdpred
#include <make_exception.hpp>	//< for make_exception

#include <sstream>

namespace calc::expr::tkn {

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
		lexer(std::string const& str, const bool throwOnUnknownLexeme = false) : throwOnUnknownLexeme{ throwOnUnknownLexeme }, sbuf{ str } {}

		lexeme getNextLexeme()
		{
		GET_NEXT_LEXEME:

			const auto pos{ getPos() };
			char c{ getNextChar() };

			// end-of-file (end-of-stream)
			if (eof() || c == EOF)
				return{ LexemeType::_EOF, pos, c };
			// alpha
			else if (str::stdpred::isalpha(c))
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
					// Only numbers that start with zero can be binary or hex (see below for octal); see "0b"/"0x"; 
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
							else if (c == ',' && !str::stdpred::isdigit(peekNextChar())) {
								// this comma is at the end of the number, put it back & stop reading
								ungetChar();
								break;
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
			case '_':
				return{ LexemeType::Underscore, pos, c };
			case ' ': case '\t': case '\v': case '\r': case '\n':
				goto GET_NEXT_LEXEME; //< skip whitespace characters
			case '.':
				if (nextCharIs(str::stdpred::isdigit))
					goto PARSE_NUMBER;
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

}
