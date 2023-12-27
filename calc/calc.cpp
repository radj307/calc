﻿#include "version.h"
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

int main(const int argc, char** argv)
{
	using namespace calc;
	using namespace calc::expr;
	using namespace calc::expr::tkn;

	vtoken vt{};

	vt.isAnyType(PrimitiveTokenType::Add, PrimitiveTokenType::Subtract);


	FunctionMap fnmap;

	auto* pow = fnmap.get("pow");
	auto result_float = (*pow)(2.5, 4);
	auto result_int = (*pow)(2, 4);


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
