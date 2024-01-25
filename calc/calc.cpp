#include "version.h"
#include "copyright.h"

// calc
#include "tokenizer_impl/custom_primitive_tokenizer.hpp"

// libcalc
#include <tokenizer/lexer.hpp>
#include <to_rpn.hpp>
#include <evaluate_rpn.hpp>
#include <helpers/vec_helpers.h>	//< for split_vec

// 307lib
#include <TermAPI.hpp>				//< for console helpers
#include <color-sync.hpp>			//< for sync
#include <opt3.hpp>					//< for ArgManager
#include <envpath.hpp>				//< for PATH
#include <hasPendingDataSTDIN.h>	//< for checking piped input
#include <print_at.hpp>				//< for print_at

#include "global.h" //< for calc global settings


struct print_help {
	const std::string executableName;
	const std::optional<std::string> helpTopic;

	static const std::vector<std::pair<std::vector<std::string>, std::function<void(std::ostream&, print_help const&)>>> topics;

	print_help(std::string const& executableName, std::optional<std::string> const& helpTopic) : executableName{ executableName }, helpTopic{ helpTopic } {}

	friend std::ostream& operator<<(std::ostream& os, const print_help& h)
	{
		// check if the user has provided an optional capture
		if (h.helpTopic.has_value()) {
			const auto topic{ str::tolower(h.helpTopic.value()) };

			for (const auto& [names, printer] : print_help::topics) {
				if (std::any_of(names.begin(), names.end(), [&topic](auto&& name) { return topic == name; })) {
					printer(os, h);
					return os;
				}
			}

			throw make_exception('\"', topic, "\" is not a recognized help topic!\n",
								 indent(10), "Use \"", h.executableName, " --help topics\" to see a list of topics.");
		}
		return os
			<< "calc " << calc_VERSION_EXTENDED << ' ' << calc_COPYRIGHT << '\n'
			<< "  Commandline calculator.\n"
			<< '\n'
			<< "USAGE:\n"
			<< "  " << h.executableName << " [OPTIONS] [--] \"<EXPRESSION>\"" << '\n'
			<< '\n'
			<< "  NOTE: Wrap expressions that use brackets in quotes, or the brackets will be removed by the shell." << '\n'
			<< "  NOTE: Negative numbers are interpreted as flags because they start with a dash. To prevent this," << '\n'
			<< "         include an argument terminator \"--\" prior to the expression." << '\n'
			<< '\n'
			<< "OPTIONS:\n"
			<< "  -h, --help [TOPIC]       Shows this help display, or details about the specified topic, then exits." << '\n'
			<< "                           Use \"--help topics\" to see a list of available topics." << '\n'
			<< "  -v, --version            Prints the current version number, then exits." << '\n'
			<< '\n'
			<< "  -d, --debug              Shows the arguments, tokens, and expressions received by the application." << '\n'
			<< "      --functions          Displays a list of all of the functions supported by the current instance." << '\n'
			<< "  -e, --echo               Outputs the expression that resulted in the output value." << '\n'
			<< "  -^, --pow                Interprets the ^ operator as an Exponent instead of BitwiseXOR." << '\n'
			<< '\n'
			<< "  -2, --bin, --base-2      Outputs numbers in binary (base-2)." << '\n'
			<< "  -8, --oct, --base-8      Outputs numbers in octal (base-8)." << '\n'
			<< "  -1, --dec, --base-10     Outputs numbers in decimal (base-10)." << '\n'
			<< "  -x, --hex, --base-16     Outputs numbers in hexadecimal (base-16)." << '\n'
			;
	}
};
inline const std::vector<std::pair<std::vector<std::string>, std::function<void(std::ostream&, print_help const&)>>> print_help::topics{
	std::make_pair(std::vector<std::string>{ "topics", "topic", "help" }, [](std::ostream& os, print_help const& h) { {
			os
				<< "USAGE:\n"
				<< "  " << h.executableName << " --help <TOPIC>\n"
				<< '\n'
				<< "  View extended documentation on a specific topic." << '\n'
				<< '\n'
				<< "TOPICS:\n";
			for (const auto& [names, _] : print_help::topics) {
				os << "  - \"" << *names.begin() /*<< str::stringify_join(names.begin(), names.end(), "\", \"")*/ << "\"\n";
			}
		} }),
	std::make_pair(std::vector<std::string>{ "syntax", "expr", "expression", "expressions" }, [](std::ostream& os, print_help const& h) { {
			os // syntax help doc:
				<< "SYNTAX\n"
				<< '\n'
				<< "TOKENS:\n"
				<< "  Expressions are tokenized to produce a sequence of tokens." << '\n'
				<< "  Whitespace is not considered to be a token." << '\n'
				<< '\n'
				<< "NUMBERS:\n"
				<< "  Numbers can be represented in binary, octal, decimal, or hexadecimal." << '\n'
				<< "  - Binary numbers start with \"0b\":       \"0b111101101\"" << '\n'
				<< "  - Octal numbers start with '0':         \"0755\"" << '\n' //< these are aligned, for some reason
				<< "  - Decimal numbers start with [1-9]:     \"493\"" << '\n'
				<< "  - Hexadecimal numbers start with \"0x\":  \"0x1ED\"" << '\n'
				<< "  Decimal numbers may be integers or floating-points." << '\n'
				<< "  Binary and hexadecimal numbers may also include underscores '_' to make" << '\n'
				<< "   them more readable." << '\n'
				<< '\n'
				<< "VARIABLES:\n"
				<< "  Variables consist of any number of consecutive alphabetic or underscore characters." << '\n'
				<< "  Variables may be set in sub-expressions, but they must be defined prior to using them:" << '\n'
				<< "    \"a = pow(2, 10); b: 1; a + b\"" << '\n'
				<< "  Variables can also be unset if there aren't any tokens after the setter:" << '\n'
				<< "    \"a = \"" << '\n'
				<< '\n'
				<< "OPERATORS:\n"
				<< "  Operators usually consist of 1 or 2 symbols, and may have different meanings" << '\n'
				<< "    depending on the types of the surrounding tokens." << '\n'
				<< "  Bitwise operators require integer operands, and will throw if you use them" << '\n'
				<< "   with a floating-point. You can use the \"trunc\" function to convert floats to int." << '\n'
				<< '\n'
				<< "FUNCTIONS:\n"
				<< "  Functions are sequences of alphabetic or underscore characters, followed by parentheses ()." << '\n'
				<< "  A list of available functions can be viewed with \"" << h.executableName << " --functions\"" << '\n'
				<< "  Functions must be called with the correct number of parameters, or an exception is thrown." << '\n'
				;
		} }),
	std::make_pair(std::vector<std::string>{ "debug", "dbg" }, [](std::ostream& os, print_help const& h) { {
					os
						<< "USAGE:\n"
						<< "  " << h.executableName << " -d -- \"<EXPRESSION>\"" << '\n'
						<< '\n'
						<< "  The debug option helps with debugging expressions in a number of ways. It shows the following information:" << '\n'
						<< "  - Arguments received by the application." << '\n'
						<< "  - The entire tokenized expression." << '\n'
						<< "  - Each sub-expression after being split by occurrences of the " << calc::expr::PrimitiveTokenTypeNames[(int)calc::expr::PrimitiveTokenType::Separator] << " token." << '\n'
						<< "  - Each sub-expression after being converted to RPN (reverse polish notation)." << '\n'
						<< "  - The values of variables whenever they're used in an expression." << '\n'
						<< '\n'
						<< "  Common problems & resolutions:" << '\n'
						<< "  - Expression segments starting with '-' aren't included in the tokenized expression." << '\n'
						<< "    This happens because arguments that start with '-' are parsed as flags instead of parameters." << '\n'
						<< "    It can be resolved by including an argument terminator \"--\" prior to the expression. For example:" << '\n'
						<< "     " << h.executableName << " -- -1 + 1" << '\n'
						<< "  - Certain characters aren't received by the application, such as brackets." << '\n'
						<< "    This happens with some shells because they have special handling for specific characters," << '\n'
						<< "     which are stripped before being passed to the application." << '\n'
						<< "    It can be resolved by enclosing the expression in double-quotes. For example:" << '\n'
						<< "     " << h.executableName << " \"pow(2, 10)\"" << '\n'
						;
				} }),
};

inline calc::expr::PrimitiveTokenType get_common_number_type(std::vector<calc::expr::tkn::primitive> const&);
inline std::string num_to_bin(calc::Number const&);
inline std::string num_to_oct(calc::Number const&);
inline std::string num_to_dec(calc::Number const&);
inline std::string num_to_hex(calc::Number const&);

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

		// create the function map
		FunctionMap fnmap;

		// --functions
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
			tokens = expr::tkn::custom_primitive_tokenizer{
				// lexemes:
				expr::tkn::lexer{ std::move(exprbuf) }.get_lexemes(),
				// functionMap:
				&fnmap,
				// caretIsExponent:
				args.check_any<opt3::Flag, opt3::Option>('^', "pow")
			}.tokenize();
		}

		const bool debug{ args.check_any<opt3::Flag, opt3::Option>('d', "dbg", "debug") };

	#pragma region Expression Debug
		if (debug) {
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
		if (expressions.empty() && !debug)
			throw make_exception("Nothing to do!");

	#pragma region Expression Debug
		if (debug) {
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

		const auto echoExpr{ args.check_any<opt3::Flag, opt3::Option>('e', "echo") };

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
					if (debug) {
						// print that the variable was set to undefined
						std::cout << "Expression " << i << " set \"" << varName << "\" to undefined\n";
					}
				#pragma endregion Expression Debug

					continue;
				}
			}

			// convert expression to RPN
			std::vector<expr::tkn::primitive> rpnExpr;
			try {
				rpnExpr = expr::to_rpn(expr, fnmap);
			} catch (const std::exception& ex) {
				std::cerr
					<< csync.get_error()
					<< "Failed to convert \""
					<< csync(color::red) << expr::tkn::stringify_tokens<false>(expr.begin(), expr.end()) << csync()
					<< "\" to RPN due to exception:\n"
					<< indent(10)
					<< ex.what()
					;
				continue;
			}

		#pragma region Expression Debug
			if (debug) {
				// print the expression in RPN
				std::cout << "Expression " << i << " in RPN:\n";
				int j{ 0 };
				for (const auto& tkn : rpnExpr) {
					const auto indexStr{ std::to_string(j++) };
					const std::string typeName{ expr::PrimitiveTokenTypeNames[(int)tkn.type] };
					std::cout << "  " << '[' << indexStr << ']' << indent(2, indexStr.size()) << typeName << indent(20, typeName.size() + 2) << tkn << '\n';
				}

				if (std::any_of(rpnExpr.begin(), rpnExpr.end(), [](auto&& tkn) { return tkn.type == expr::PrimitiveTokenType::Variable; }))
					std::cout << "Expression " << i << " variable map:\n" << term::print_at(2, std::nullopt, variables);
			}
		#pragma endregion Expression Debug

			// evaluate the RPN expression
			try {
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
					<< '\n'
					;
				continue;
			}

			// handle the result
			if (setVariable.has_value()) {
				// set the variable's value
				const auto varName{ setVariable.value().text };
				variables[varName] = result;

			#pragma region Expression Debug
				if (debug) {
					std::cout << "Expression " << i << " set variable \"" << varName << "\" to " << result << '\n';
				}
			#pragma endregion Expression Debug
			}
			else {
				std::string result_str{};
				// determine which number base to print the result in
				using expr::PrimitiveTokenType;

				if (args.check_any<opt3::Flag, opt3::Option>('2', "bin", "binary", "base-2"))
					result_str = num_to_bin(result);
				else if (args.check_any<opt3::Flag, opt3::Option>('8', "oct", "octal", "base-8"))
					result_str = num_to_oct(result);
				else if (args.check_any<opt3::Flag, opt3::Option>('1', "dec", "decimal", "base-10"))
					result_str = num_to_dec(result);
				else if (args.check_any<opt3::Flag, opt3::Option>('x', "hex", "hexadecimal", "base-16"))
					result_str = num_to_hex(result);
				else {
					switch (get_common_number_type(expr)) {
					default:
						result_str = num_to_dec(result);
						break;
					case PrimitiveTokenType::BinaryNumber:
						result_str = num_to_bin(result);
						break;
					case PrimitiveTokenType::OctalNumber:
						result_str = num_to_oct(result);
						break;
					case PrimitiveTokenType::HexNumber:
						result_str = num_to_hex(result);
						break;
					}
				}

				if (echoExpr) {
					std::cout << expr::tkn::stringify_tokens(expr.begin(), expr.end()) << " = ";
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

/**
 * @brief							Checks if the specified expression includes only numbers
 *									 in a specific base, and returns the token type.
 * @param expr					  -	The expression to get the common numeric type in.
 * @returns							The common numeric token type when expr contains numbers of
 *									 one type; otherwise, PrimitiveTokenType::Unknown
 */
inline calc::expr::PrimitiveTokenType get_common_number_type(std::vector<calc::expr::tkn::primitive> const& expr)
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

/// @brief	Strips trailing zeroes and decimal points.
inline std::string truncate_zeroes(std::string const& num)
{
	if (const auto decPos{ num.find('.') }; decPos != std::string::npos) {
		if (const auto lastNonZero{ num.find_last_of("123456789") };
			lastNonZero == std::string::npos) {
			return num;
		}
		else if (lastNonZero <= decPos) {
			// all zeroes after the decimal
			return num.substr(0, decPos);
		}
		else {
			return num.substr(0, lastNonZero + 1);
		}
	}
	return num;
}

/// @brief	Converts the specified number to a binary string.
inline std::string num_to_bin(calc::Number const& num)
{
	if (!num.has_integral_value())
		throw make_exception("Cannot convert floating-point value \"", num, "\" to binary!");
	return "0b" + to_base(num, 2);
}
/// @brief	Converts the specified number to an octal string.
inline std::string num_to_oct(calc::Number const& num)
{
	if (!num.has_integral_value())
		throw make_exception("Cannot convert floating-point value \"", num, "\" to octal!");
	const auto s{ to_base(num, 8) };
	return s.starts_with('0') ? s : "0" + s;
}
/// @brief	Converts the specified number to a decimal string.
inline std::string num_to_dec(calc::Number const& num)
{
	if (num.has_integral_value())
		return to_base(num, 10);
	else return truncate_zeroes(str::stringify(std::fixed, std::setprecision(128), num.cast_to<boost::multiprecision::cpp_dec_float_100>()));
}
/// @brief	Converts the specified number to a hexadecimal string.
inline std::string num_to_hex(calc::Number const& num)
{
	if (!num.has_integral_value())
		throw make_exception("Cannot convert floating-point value \"", num, "\" to hexadecimal!");
	return "0x" + to_base(num, 16);
}
