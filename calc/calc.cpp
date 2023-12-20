#include "ExpressionTokenizer.hpp"
#include "Number.hpp"

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

static const std::map<calc::expr::LexemeType, std::string> LexemeTypeNames{
	{ calc::expr::LexemeType::Unknown, "Unknown" },
	{ calc::expr::LexemeType::Escape, "Escape" },
	{ calc::expr::LexemeType::Equal, "Equal" },
	{ calc::expr::LexemeType::Add, "Add" },
	{ calc::expr::LexemeType::Subtract, "Subtract" },
	{ calc::expr::LexemeType::Multiply, "Multiply" },
	{ calc::expr::LexemeType::Divide, "Divide" },
	{ calc::expr::LexemeType::Percent, "Percent" },
	{ calc::expr::LexemeType::Alpha, "Alpha" },
	{ calc::expr::LexemeType::IntNumber, "IntNumber" },
	{ calc::expr::LexemeType::RealNumber, "RealNumber" },
	{ calc::expr::LexemeType::BinaryNumber, "BinaryNumber" },
	{ calc::expr::LexemeType::OctalNumber, "OctalNumber" },
	{ calc::expr::LexemeType::HexNumber, "HexNumber" },
	{ calc::expr::LexemeType::Period, "Period" },
	{ calc::expr::LexemeType::Comma, "Comma" },
	{ calc::expr::LexemeType::AngleBracketOpen, "AngleBracketOpen" },
	{ calc::expr::LexemeType::AngleBracketClose, "AngleBracketClose" },
	{ calc::expr::LexemeType::BracketOpen, "BracketOpen" },
	{ calc::expr::LexemeType::BracketClose, "BracketClose" },
	{ calc::expr::LexemeType::_EOF, "_EOF" },
};

int main(const int argc, char** argv)
{
	try {
		opt3::ArgManager args{ argc, argv,
			// argument defs:
		};

		const auto& [procPath, procName] { env::PATH{}.resolve_split(argv[0]) };


		using namespace calc::expr;

		const auto expr{ "(5 / 0.56 )(0b1110 * 0xFF0)" };
		const auto lexemes = token::lexer{ expr }.get_lexemes(false);

		std::cout << "Input Expression:" << '\"' << expr << '\"' << '\n' << '\n';

		const size_t COLSZ_0{ 4 };
		const size_t COLSZ_1{ 12 };
		const size_t COLSZ_2{ 8 };
		const size_t COLSZ_3{ 20 };

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
			<< indent(20, 0, '-') << '\n'
			;

		int i = 0;
		for (const auto& it : lexemes) {
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
			str = LexemeTypeNames.find(it.type)->second;
			std::cout << str << indent(COLSZ_3, str.size()) << "| ";

			// print token value
			std::cout << it.value << '\n';
		}
		return 0;


		const auto pipedInput{ hasPendingDataSTDIN() };

		if ((args.empty() && !pipedInput) || args.check_any<opt3::Flag, opt3::Option>('h', "help")) {
			std::cout << print_help(procName.generic_string());
			return 0;
		}
		else if (args.check_any<opt3::Flag, opt3::Option>('v', "version")) {
			std::cout /* << calc_VERSION_EXTENDED */;
			return 0;
		}

		// create a stringstream for the entire expression buffer
		std::stringstream exprBuff;

		// if there is piped input move it into the expression buffer
		if (pipedInput) {
			exprBuff << std::cin.rdbuf();
		}

		return 0;
	} catch (const std::exception& ex) {
		std::cerr << csync.get_fatal() << ex.what() << std::endl;
		return 1;
	}
}
