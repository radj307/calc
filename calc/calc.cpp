#include "version.h"
#include "copyright.h"

#include "tokenizer/lexer.hpp"
#include "tokenizer/primitive_tokenizer.hpp"
#include "tokenizer/expr_builder.hpp"

#include "FunctionMap.hpp" //< move this to libcalc
// libcalc
#include <Number.hpp>

// 307lib
#include <TermAPI.hpp>				//< for console helpers
#include <color-sync.hpp>			//< for sync
#include <opt3.hpp>					//< for ArgManager
#include <envpath.hpp>				//< for PATH
#include <hasPendingDataSTDIN.h>	//< for checking piped input
#include <print_tree.hpp>			//< for print_tree

#include <list>

#include "settings.h" //< for calc global settings

struct print_help {
	const std::string _executableName;
	const std::optional<std::string> _helpArg;

	print_help(const std::string& executableName, const std::optional<std::string>& helpArg) : _executableName{ executableName }, _helpArg{ helpArg } {}

	friend std::ostream& operator<<(std::ostream& os, const print_help& h)
	{
		// check if the user has provided an optional capture
		if (h._helpArg.has_value()) {
			// check if the optional capture is recognized
			if (h._helpArg.value() == "syntax") {
				/*
				* SYNTAX
				*/
				return os
					<< "SYNTAX" << '\n'
					<< '\n'
					<< "NUMBERS:\n"
					<< "  - Binary numbers must start with \'0b\'.   eg. 0b11001" << '\n'
					<< "  - Hexadecimal numbers must start with \'0x\'.   eg. 0xAB64" << '\n'
					<< "  - Octal numbers start with \'0\'.    eg. 056" << '\n'
					<< "  - Numbers without these prefixes are assumed to be Decimal." << '\n'
					<< "  - Numbers cannot have spaces. If you would like to space out large numbers you can use underscores" << '\n'
					<< "     to connect digits.    eg. 132_678 will be treated as 132678" << '\n'
					<< '\n'
					<< "VARIABLES:\n"
					<< "  - Variables are a single character only." << '\n'
					<< "  - Can be any lower or uppercase character, but are case sensitive." << '\n'
					<< "	eg. \'A\' is a different variable than \'a\'" << '\n'
					<< '\n'
					<< "FUNCTIONS:\n"
					<< "  - Functions can be any number of connected characters followed by parentheses containing the input." << '\n'
					<< "  - There cannot be a space between the function name and the parentheses." << '\n'
					<< "	 Valid: sqrt(25)		Not Valid: sqrt (25)" << '\n'
					<< '\n'
					;
			}
			else {
				return os
					<< "Help Subject " << h._helpArg.value() << " does not exist." << '\n'
					<< "USAGE: \n"
					<< "  -h, --help [SUBJECT]" << '\n'
					<< '\n'
					<< "SUBJECTS:" << '\n'
					<< "  syntax			Shows details about the syntax for usage of this program." << '\n'
					;
			}
		}
		return os
			<< "calc " << calc_VERSION_EXTENDED << ' ' << calc_COPYRIGHT << '\n'
			<< "  Commandline calculator.\n"
			<< '\n'
			<< "USAGE:\n"
			<< "  " << h._executableName << " [OPTIONS] [--] <EXPRESSION>" << '\n'
			<< '\n'
			<< "  NOTE: Negative numbers are interpreted as flags because they start with a dash. To prevent this," << '\n'
			<< "         specify \"--\" before it on the commandline to disable parsing for all args that follow." << '\n'
			<< '\n'
			<< "OPTIONS:\n"
			<< "  -h, --help [SUBJECT]     Shows this help display, or details about the specified subject, then exits." << '\n'
			<< "  -v, --version            Prints the current version number, then exits." << '\n'
			<< '\n'
			<< "  -d, --debug              Shows verbose output to help with debugging an expression." << '\n'
			;
	}
};


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
	using namespace calc;

	//std::vector<std::any> args{ 0.0l, 0.0l };

	//// this works:
	//auto res = std::apply(std::powl, unwrap<long double, long double>(args, std::index_sequence_for<long double, long double>()));

	//// this works:
	//auto fn = wrap_call<long double, long double>(std::powl);

	//FunctionMap fnmap1{};
	//const auto res1 = std::any_cast<long double>(fnmap1.invoke("pow", 0.0l, 0.0l));




	std::string _expr{ "5 + ( (5 + 7 * 23) / (2 + sqrt(9)) ) / 2" };
	auto _tokens{ expr::tkn::primitive_tokenizer{ expr::tkn::lexer{ _expr }.get_lexemes() }.tokenize() };


	std::cout
		<< _expr << std::endl
		<< print_tree<TreeNode<expr::tkn::vtoken>>(expr::tkn::expr_builder{ _tokens }.build(), [](auto&& node) -> std::vector<TreeNode<expr::tkn::vtoken>> { return node.children; }) << std::endl;


	try {
		opt3::ArgManager args{ argc, argv,
			// argument defs:
			opt3::make_template(opt3::CaptureStyle::Optional, opt3::ConflictStyle::Conflict, 'h', "help"),
		};

		const auto& [procPath, procName] { env::PATH{}.resolve_split(argv[0]) };

		if (args.empty() || args.check_any<opt3::Flag, opt3::Option>('h', "help")) {
			std::cout << print_help(procName.generic_string(), args.getv_any<opt3::Flag, opt3::Option>('h', "help"));
			return 0;
		}
		else if (args.check_any<opt3::Flag, opt3::Option>('v', "version")) {
			std::cout << calc_VERSION_EXTENDED << std::endl;
			return 0;
		}

		// create a stringstream for the entire expression buffer
		std::stringstream exprbuf;

		// if there is piped input move it into the expression buffer
		if (hasPendingDataSTDIN())
			exprbuf << std::cin.rdbuf() << ' ';
		for (const auto& param : args.getv_all<opt3::Parameter>())
			exprbuf << param << ' ';

		const auto expr_root{ expr::tkn::expr_builder{ expr::tkn::primitive_tokenizer{
			expr::tkn::lexer{ std::move(exprbuf) }.get_lexemes()
		}.tokenize() }.build() };

		std::cout << expr_root << std::endl;

		return 0;
	} catch (const std::exception& ex) {
		std::cerr << csync.get_fatal() << ex.what() << std::endl;
		return 1;
	}
}
