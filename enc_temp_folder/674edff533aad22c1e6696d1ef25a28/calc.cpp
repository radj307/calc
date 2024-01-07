#include "version.h"
#include "copyright.h"

#include "tokenizer/lexer.hpp"
#include "tokenizer/primitive_tokenizer.hpp"
#include "to_rpn.hpp"
#include "evaluate_rpn.hpp"

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
			<< "  " << h._executableName << " [OPTIONS] [--] \"<EXPRESSION>\"" << '\n'
			<< '\n'
			<< "  NOTE: Wrap expressions that use brackets in quotes, or the brackets will be removed by the shell." << '\n'
			<< "  NOTE: Negative numbers are interpreted as flags because they start with a dash. To prevent this," << '\n'
			<< "         specify \"--\" before it on the commandline to disable parsing for all args that follow." << '\n'
			<< '\n'
			<< "OPTIONS:\n"
			<< "  -h, --help [SUBJECT]     Shows this help display, or details about the specified subject, then exits." << '\n'
			<< "  -v, --version            Prints the current version number, then exits." << '\n'
			<< '\n'
			<< "  -d, --debug              Shows verbose output to help with debugging an expression." << '\n'
			<< "      --functions          Displays a list of all of the functions supported by the current instance." << '\n'
			;
	}
};

template<typename T, class Predicate>
inline std::vector<std::vector<T>> split_vec(std::vector<T> const& vec, Predicate const& pred)
{
	std::vector<std::vector<T>> out;
	std::vector<T> buf;
	buf.reserve(vec.size());

	for (auto it{ vec.begin() }, it_end{ vec.end() }; it != it_end; ++it) {
		if (pred(*it)) {
			// move the buffer into the output vector
			out.emplace_back(std::move(buf));
			buf = {};
			buf.reserve(vec.size());
		}
		else buf.emplace_back(*it);
	}

	if (!buf.empty())
		out.emplace_back(std::move(buf));

	return out;
}

int main(const int argc, char** argv)
{
	using namespace calc;

	try {
		opt3::ArgManager args{ argc, argv,
			// argument defs:
			opt3::make_template(opt3::CaptureStyle::Optional, opt3::ConflictStyle::Conflict, 'h', "help"),
		};

		const auto& [procPath, procName] { env::PATH{}.resolve_split(argv[0]) };
		const bool hasPipedInput{ hasPendingDataSTDIN() };

		if ((!hasPipedInput && args.empty()) || args.check_any<opt3::Flag, opt3::Option>('h', "help")) {
			std::cout << print_help(procName.generic_string(), args.getv_any<opt3::Flag, opt3::Option>('h', "help"));
			return 0;
		}
		else if (args.check_any<opt3::Flag, opt3::Option>('v', "version")) {
			std::cout << calc_VERSION_EXTENDED << std::endl;
			return 0;
		}

		// load/build the function map
		FunctionMap fnmap;

		// functions ; show the table of functions & exit
		if (args.check<opt3::Option>("functions")) {
			std::cout << fnmap;
			return 0;
		}

		std::vector<expr::tkn::primitive> tokens;
		{ // read in the expression
			std::stringstream exprbuf;

			// get input from STDIN (if it exists)
			if (hasPipedInput)
				exprbuf << std::cin.rdbuf() << ' ';
			// get parameters
			for (const auto& param : args.getv_all<opt3::Parameter>())
				exprbuf << param << ' ';

			// tokenize the expression
			tokens = expr::tkn::primitive_tokenizer{
				expr::tkn::lexer{ std::move(exprbuf) }.get_lexemes(),
				&fnmap
			}.tokenize();
		}

		const bool debugExpressions{ args.check_any<opt3::Flag, opt3::Option>('d', "debug") };

		if (debugExpressions) {
			std::cout << "Args:\n";
			int i{ 0 };
			for (const auto& arg : args) {
				const auto indexStr{ std::to_string(i++) };
				std::cout << '[' << indexStr << ']' << indent(3, indexStr.size());
				if (arg.is_type<opt3::Option>()) std::cout << "(Option)" << indent(10, 9);
				else if (arg.is_type<opt3::Flag>()) std::cout << "(Flag)" << indent(10, 7);
				else if (arg.is_type<opt3::Parameter>()) std::cout << "(Param)" << indent(10, 8);
				std::cout << arg.name();
				if (arg.has_capture())
					std::cout << ' ' << arg.capture();
				std::cout << '\n';
			}

			std::cout << "Tokens:\n";
			i = 0;
			for (const auto& tkn : tokens) {
				const auto indexStr{ std::to_string(i++) };
				const std::string typeName{ expr::PrimitiveTokenTypeNames[(int)tkn.type] };
				std::cout << '[' << indexStr << ']' << indent(3, indexStr.size()) << typeName << indent(20, typeName.size()) << tkn << '\n';
			}
		}

		std::vector<std::vector<expr::tkn::primitive>> expressions{ split_vec(tokens, [](auto&& tkn) { return tkn.type == expr::PrimitiveTokenType::Separator; }) };

		if (expressions.empty() && !debugExpressions)
			throw make_exception("Nothing to do!");

		if (debugExpressions) {
			for (size_t i{ 0 }, i_max{ expressions.size() }; i < i_max; ++i) {
				for (const auto& rpnExpr : expressions) {
					std::cout << "Expression " << i++ << ":\n";
					int j{ 0 };
					for (const auto& tkn : rpnExpr) {
						const auto indexStr{ std::to_string(j++) };
						const std::string typeName{ expr::PrimitiveTokenTypeNames[(int)tkn.type] };
						std::cout << "  " << '[' << indexStr << ']' << indent(3, indexStr.size()) << typeName << indent(20, typeName.size() + 2) << tkn << '\n';
					}
				}
			}
		}

		VarMap variables;

		calc::Number result;
		int i{ 0 };
		for (const auto& expr : expressions) {
			try {
				const auto rpnExpr{ expr::to_rpn(expr) };

				if (debugExpressions) {
					std::cout << "Expression " << i++ << " in RPN:\n";
					int j{ 0 };
					for (const auto& tkn : rpnExpr) {
						const auto indexStr{ std::to_string(j++) };
						const std::string typeName{ expr::PrimitiveTokenTypeNames[(int)tkn.type] };
						std::cout << "  " << '[' << indexStr << ']' << indent(3, indexStr.size()) << typeName << indent(20, typeName.size() + 2) << tkn << '\n';
					}
				}

				result = expr::evaluate_rpn(rpnExpr, fnmap, variables);

				std::cout
					<< str::to_string(result.cast_to<long double>(), 16, false)
					<< std::endl;
			} catch (const std::exception& ex) {
				std::cerr
					<< csync.get_error()
					<< "Failed to evaluate expression \""
					<< csync(color::orange) << calc::expr::tkn::stringify_tokens<false>(expr.begin(), expr.end()) << csync()
					<< "\" due to exception:\n"
					<< indent(10)
					<< ex.what()
					<< '\n';
			}
			++i;
		}

		return 0;
	} catch (const std::exception& ex) {
		std::cerr << csync.get_fatal() << ex.what() << std::endl;
		return 1;
	}
}
