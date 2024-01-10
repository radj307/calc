#include "version.h"
#include "copyright.h"

#include "tokenizer/lexer.hpp"
#include "tokenizer/primitive_tokenizer.hpp"
#include "to_rpn.hpp"
#include "evaluate_rpn.hpp"
#include "helpers/vec_helpers.h"	//< for split_vec

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
			<< "         include an argument terminator \"--\" prior to the expression." << '\n'
			<< '\n'
			<< "OPTIONS:\n"
			<< "  -h, --help [SUBJECT]     Shows this help display, or details about the specified subject, then exits." << '\n'
			<< "  -v, --version            Prints the current version number, then exits." << '\n'
			<< '\n'
			<< "  -d, --debug              Shows the arguments, tokens, and expressions received by the application." << '\n'
			<< "      --functions          Displays a list of all of the functions supported by the current instance." << '\n'
			//<< '\n'
			//<< "  " << '\n'
			;
	}
};

inline calc::expr::PrimitiveTokenType get_common_number_type(std::vector<calc::expr::tkn::primitive> const&);

int main(const int argc, char** argv)
{
	using namespace calc;

	try {
		opt3::ArgManager args{ argc, argv,
			// capturing argument defs:
			opt3::make_template(opt3::CaptureStyle::Optional, opt3::ConflictStyle::Conflict, 'h', "help"),
		};

		const auto& [procPath, procName] { env::PATH{}.resolve_split(argv[0]) };
		const bool hasPipedInput{ hasPendingDataSTDIN() };

		// -h | --help
		if ((!hasPipedInput && args.empty()) || args.check_any<opt3::Flag, opt3::Option>('h', "help")) {
			std::cout << print_help(procName.generic_string(), args.getv_any<opt3::Flag, opt3::Option>('h', "help"));
			return 0;
		}
		// -v | --version
		else if (args.check_any<opt3::Flag, opt3::Option>('v', "version")) {
			std::cout << calc_VERSION_EXTENDED << std::endl;
			return 0;
		}

		// load/build the function map
		FunctionMap fnmap;

		// functions ; show the table of functions & exit
		if (args.check_any<opt3::Option>("functions", "function")) {
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

	#pragma region Expression Debug
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
	#pragma endregion Expression Debug

		// split the expression by separator characters
		std::vector<std::vector<expr::tkn::primitive>> expressions{ split_vec(tokens, [](auto&& tkn) { return tkn.type == expr::PrimitiveTokenType::Separator; }) };

		// exit with error if there is nothing to evaluate
		if (expressions.empty() && !debugExpressions)
			throw make_exception("Nothing to do!");

	#pragma region Expression Debug
		if (debugExpressions) {
			// print out all of the tokens in each expression as written
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
	#pragma endregion Expression Debug

		// create the table of variables
		VarMap variables;

		// enumerate each expression, convert to RPN, and evaluate
		calc::Number result;
		int i{ 0 };
		std::optional<expr::tkn::primitive> setVariable{ std::nullopt }; //< contains variable that is being set, if any
		for (auto it{ expressions.begin() }, it_end{ expressions.end() };
			 it != it_end;
			 ++it, setVariable = std::nullopt, ++i) {
			auto expr{ *it };

			// check if this is a variable setter before converting to RPN
			if (expr.size() >= 2
				&& expr.at(0).type == expr::PrimitiveTokenType::Variable
				&& expr.at(1).type == expr::PrimitiveTokenType::Setter) {
				// is a variable setter, not an actual expression
				setVariable = expr.at(0);
				// erase the first and second tokens
				expr.erase(expr.begin(), expr.begin() + 2);

				if (expr.empty()) {
					// set variable to undefined & continue
					const auto varName{ setVariable.value().text };
					variables.erase(varName);

				#pragma region Expression Debug
					if (debugExpressions) {
						// print that the variable was set to undefined
						std::cout << "Expression " << i << " set \"" << varName << "\" to undefined\n";
					}
				#pragma endregion Expression Debug

					continue;
				}
			}

			// evaluate the remaining expression
			try {
				// convert to RPN
				const auto rpnExpr{ expr::to_rpn(expr) };

			#pragma region Expression Debug
				if (debugExpressions) {
					// print the expression in RPN
					std::cout << "Expression " << i << " in RPN:\n";
					int j{ 0 };
					for (const auto& tkn : rpnExpr) {
						const auto indexStr{ std::to_string(j++) };
						const std::string typeName{ expr::PrimitiveTokenTypeNames[(int)tkn.type] };
						std::cout << "  " << '[' << indexStr << ']' << indent(3, indexStr.size()) << typeName << indent(20, typeName.size() + 2) << tkn << '\n';
					}
				}
			#pragma endregion Expression Debug

				// evaluate the result
				result = expr::evaluate_rpn(rpnExpr, fnmap, variables);
			} catch (const std::exception& ex) {
				std::cerr
					<< csync.get_error()
					<< "Failed to evaluate expression \""
					<< csync(color::orange) << calc::expr::tkn::stringify_tokens<false>(expr.begin(), expr.end()) << csync()
					<< "\" due to exception:\n"
					<< indent(10)
					<< ex.what()
					<< '\n';

				continue;
			}

			if (setVariable.has_value()) {
				// set the variable's value
				const auto varName{ setVariable.value().text };
				variables[varName] = result;

			#pragma region Expression Debug
				if (debugExpressions) {
					std::cout << "Expression " << i << " set variable \"" << varName << "\" to " << result << '\n';
				}
			#pragma endregion Expression Debug
			}
			else {
				std::string result_str{};
				// determine which number base to print the result in
				using expr::PrimitiveTokenType;
				switch (get_common_number_type(expr)) {
				default:
					result_str = str::to_string(result.cast_to<long double>(), 16, false);
					break;
				case PrimitiveTokenType::BinaryNumber:
					result_str = "0b" + str::fromnumber(result.cast_to<long long>(), 2);
					break;
				case PrimitiveTokenType::OctalNumber:
					result_str = str::fromnumber(result.cast_to<long long>(), 8);
					if (!result_str.starts_with('0'))
						result_str.insert(result_str.begin(), '0');
					break;
				case PrimitiveTokenType::HexNumber:
					result_str = "0x" + str::fromnumber(result.cast_to<long long>(), 16);
					break;
				}
				// print to the console
				std::cout << result_str << std::endl;
			}
		}

		return 0;
	} catch (const std::exception& ex) {
		std::cerr << csync.get_fatal() << ex.what() << std::endl;
		return 1;
	}
}

inline calc::expr::PrimitiveTokenType get_common_number_type(std::vector<calc::expr::tkn::primitive> const& expr)
/**
 * @brief							Checks if the specified expression includes only numbers
 *									 in a specific base, and returns the token type.
 * @param expr					  -	The expression to get the common numeric type in.
 * @returns							The common numeric token type when expr contains numbers of
 *									 one type; otherwise, PrimitiveTokenType::Unknown
 */
{
	using namespace calc::expr;
	if (expr.empty()) return PrimitiveTokenType::Unknown;

	std::vector<tkn::primitive> numbers;
	numbers.reserve(expr.size());
	for (const auto& tkn : expr) {
		if (is_number(tkn.type))
			numbers.emplace_back(tkn);
	}

	if (numbers.empty()) return PrimitiveTokenType::Unknown;
	else if (numbers.size() == 1) return numbers[0].type;

	return std::all_of(numbers.begin() + 1, numbers.end(), [&numbers](auto&& v) { return v.type == numbers[0].type; })
		? numbers[0].type
		: PrimitiveTokenType::Unknown;
}
