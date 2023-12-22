#include "ExpressionTokenizer.hpp"

// libcalc
#include <Number.hpp>

// 307lib
#include <TermAPI.hpp>				//< for console helpers
#include <color-sync.hpp>			//< for sync
#include <opt3.hpp>					//< for ArgManager
#include <envpath.hpp>				//< for PATH
#include <hasPendingDataSTDIN.h>	//< for checking piped input

color::sync csync{};

struct print_help {
	const std::string _executableName;

	constexpr print_help(const std::string& executableName) : _executableName{ executableName } {}

	friend std::ostream& operator<<(std::ostream& os, const print_help& h)
	{
		return os
			<< "calc " /* << calc_VERSION_EXTENDED */ << ' ' /* << calc_COPYRIGHT */ << '\n'
			<< "  Commandline calculator.\n"
			<< '\n'
			<< "USAGE:\n"
			<< "  " << h._executableName << " [OPTIONS] [--] <EXPRESSION>" << '\n'
			<< '\n'
			<< "  NOTE: Negative numbers are interpreted as flags because they start with a dash. To prevent this," << '\n'
			<< "         specify \"--\" before it on the commandline to disable parsing for all args that follow." << '\n'
			<< '\n'
			<< "OPTIONS:\n"
			<< "  -h, --help               Shows this help display, then exits." << '\n'
			<< "  -v, --version            Prints the current version number, then exits." << '\n'
			;
	}
};
$DefineExcept(print_help_exception);

static const std::map<calc::expr::LexemeType, std::string> LexemeTypeNames{
	{ calc::expr::LexemeType::Unknown, "Unknown" },
	{ calc::expr::LexemeType::Escape, "Escape" },
	{ calc::expr::LexemeType::Equal, "Equal" },
	{ calc::expr::LexemeType::Colon, "Colon" },
	{ calc::expr::LexemeType::Semicolon, "Semicolon" },
	{ calc::expr::LexemeType::Operator, "Operator" },
	{ calc::expr::LexemeType::Alpha, "Alpha" },
	{ calc::expr::LexemeType::IntNumber, "IntNumber" },
	{ calc::expr::LexemeType::RealNumber, "RealNumber" },
	{ calc::expr::LexemeType::BinaryNumber, "BinaryNumber" },
	{ calc::expr::LexemeType::OctalNumber, "OctalNumber" },
	{ calc::expr::LexemeType::HexNumber, "HexNumber" },
	{ calc::expr::LexemeType::Period, "Period" },
	{ calc::expr::LexemeType::Comma, "Comma" },
	{ calc::expr::LexemeType::Macro, "Macro" },
	{ calc::expr::LexemeType::AngleBracketOpen, "AngleBracketOpen" },
	{ calc::expr::LexemeType::AngleBracketClose, "AngleBracketClose" },
	{ calc::expr::LexemeType::SquareBracketOpen, "SquareBracketOpen" },
	{ calc::expr::LexemeType::SquareBracketClose, "SquareBracketClose" },
	{ calc::expr::LexemeType::ParenthesisOpen, "ParenthesisOpen" },
	{ calc::expr::LexemeType::ParenthesisClose, "ParenthesisClose" },
	{ calc::expr::LexemeType::BraceOpen, "BraceOpen" },
	{ calc::expr::LexemeType::BraceClose, "BraceClose" },
	{ calc::expr::LexemeType::_EOF, "_EOF" },
};
static const std::map<calc::expr::PrimitiveTokenType, std::string> PrimitiveTypeNames{
	{ calc::expr::PrimitiveTokenType::Unknown, "Unknown" },
	{ calc::expr::PrimitiveTokenType::Variable, "Variable" },
	{ calc::expr::PrimitiveTokenType::ExpressionOpen, "ExpressionOpen" },
	{ calc::expr::PrimitiveTokenType::ExpressionClose, "ExpressionClose" },
	{ calc::expr::PrimitiveTokenType::FunctionName, "FunctionName" },
	{ calc::expr::PrimitiveTokenType::FunctionParamsOpen, "FunctionParamsOpen" },
	{ calc::expr::PrimitiveTokenType::FunctionParamsClose, "FunctionParamsClose" },
	{ calc::expr::PrimitiveTokenType::IntNumber, "IntNumber" },
	{ calc::expr::PrimitiveTokenType::RealNumber, "RealNumber" },
	{ calc::expr::PrimitiveTokenType::BinaryNumber, "BinaryNumber" },
	{ calc::expr::PrimitiveTokenType::OctalNumber, "OctalNumber" },
	{ calc::expr::PrimitiveTokenType::HexNumber, "HexNumber" },
	{ calc::expr::PrimitiveTokenType::Add, "Add" },
	{ calc::expr::PrimitiveTokenType::Subtract, "Subtract" },
	{ calc::expr::PrimitiveTokenType::Multiply, "Multiply" },
	{ calc::expr::PrimitiveTokenType::Divide, "Divide" },
	{ calc::expr::PrimitiveTokenType::Modulo, "Modulo" },
	{ calc::expr::PrimitiveTokenType::Exponent, "Exponent" },
	{ calc::expr::PrimitiveTokenType::Factorial, "Factorial" },
	{ calc::expr::PrimitiveTokenType::LeftShift, "LeftShift" },
	{ calc::expr::PrimitiveTokenType::RightShift, "RightShift" },
	{ calc::expr::PrimitiveTokenType::BitOR, "BitOR" },
	{ calc::expr::PrimitiveTokenType::BitAND, "BitAND" },
	{ calc::expr::PrimitiveTokenType::BitXOR, "BitXOR" },
	{ calc::expr::PrimitiveTokenType::BitNOT, "BitNOT" },
	{ calc::expr::PrimitiveTokenType::Equal, "Equal" },
};

int main(const int argc, char** argv)
{
	try {
		opt3::ArgManager args{ argc, argv,
			// argument defs:
		};

		const auto& [procPath, procName] { env::PATH{}.resolve_split(argv[0]) };


		static_assert(var::enumerable<std::string>, "ASSERTION FAILURE");

		
		static_assert(var::enumerable_of<std::vector<char>, char>, "ASSERTION FAILURE");
		static_assert(var::enumerable_of<const std::vector<char>, const char>, "ASSERTION FAILURE");
		static_assert(var::enumerable_of<const std::vector<char>, char>, "ASSERTION FAILURE");
		static_assert(var::enumerable_of<std::vector<char>, const char>, "ASSERTION FAILURE");
		//static_assert(var::enumerable_of<std::vector<std::stringstream>, char>, "ASSERTION FAILURE");


		// v REMOVE WHEN DONE v

		using namespace calc::expr;

		//const auto expr{ "(5 / 0.56, ..5_015 )(0b1110 * 0xFF0 *a 8 * 0ib ${ABCD})" };
		const auto expr{ "(09(-2^5 * 3 - 7) / (4a % 3) + (a - sqrt(25, 50 asdf())) + 4 * 7) * (sin(60) + cos(-45))) - (log(100) + 8 / (tan(30) * exp(2)))" };
		//const auto expr{ "(f() f(1) 2)" };
		//                01234567891111111111222222222233333333
		//                          0123456789012345678901234567
		const auto lexemes = tkn::lexer{ expr }.get_lexemes(false);
		const auto primitives = tkn::primitive_tokenizer{ lexemes }.tokenize();

		std::cout << "Input Expression: " << '\"' << expr << '\"' << '\n' << '\n';

		const size_t COLSZ_0{ 4 };
		const size_t COLSZ_1{ 12 };
		const size_t COLSZ_2{ 8 };
		const size_t COLSZ_3{ 20 };

		// print output table
		std::cout
			<< "Idx" << indent(COLSZ_0, 3) << "| "
			<< "Pos" << indent(COLSZ_1, 3) << "| "
			<< "Len" << indent(COLSZ_2, 3) << "| "
			<< "Type" << indent(COLSZ_3, 4) << "| "
			<< "Value" << '\n'
			<< indent(COLSZ_0, 0, '-') << '|'
			<< indent(COLSZ_1 + 1, 0, '-') << '|'
			<< indent(COLSZ_2 + 1, 0, '-') << '|'
			<< indent(COLSZ_3 + 1, 0, '-') << '|'
			<< indent(20, 0, '-') << '\n';
		int i = 0;
		for (const auto& it : primitives) {
			std::string str{ std::to_string(i++) };

			std::cout << str << indent(COLSZ_0, str.size()) << "| ";

			// get position string
			str = std::to_string(it.pos);
			if (const auto& endPos{ it.getEndPos() - 1 }; it.pos != endPos)
				str += "-" + std::to_string(endPos);
			std::cout << str << indent(COLSZ_1, str.size()) << "| ";
			// clear buffer

			// print token length
			str = str::stringify(it.getEndPos() - it.pos);
			std::cout << str << indent(COLSZ_2, str.size()) << "| ";

			// print token type
			str = PrimitiveTypeNames.find(it.type)->second;
			std::cout << str << indent(COLSZ_3, str.size()) << "| ";

			// print token value
			std::cout << it.text << '\n';
		}

		std::cout << "\n\n";

		const auto combined{ calc::expr::tkn::combine_tokens(calc::expr::PrimitiveTokenType::Unknown, std::move(lexemes)) };

		std::cout << '\"' << expr << '\"' << '\n';
		std::cout << '\"' << combined << '\"' << '\n';

		return 0;

		// ^ REMOVE WHEN DONE ^


		const auto pipedInput{ hasPendingDataSTDIN() };

		if (args.empty() || args.check_any<opt3::Flag, opt3::Option>('h', "help")) {
			std::cout << print_help(procName.generic_string());
			return 0;
		}
		else if (args.check_any<opt3::Flag, opt3::Option>('v', "version")) {
			std::cout /* << calc_VERSION_EXTENDED */;
			return 0;
		}

		// create a stringstream for the entire expression buffer
		std::stringstream exprbuf;

		// if there is piped input move it into the expression buffer
		if (pipedInput) {
			exprbuf << std::cin.rdbuf() << ' ';
		}

		for (const auto& param : args.getv_all<opt3::Parameter>()) {
			exprbuf << param << ' ';
		}

		return 0;
	} catch (const std::exception& ex) {
		std::cerr << csync.get_fatal() << ex.what() << std::endl;
		return 1;
	}
}
