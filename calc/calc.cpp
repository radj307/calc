#include "TreeNode.hpp"

#include "tokenizer/lexer.hpp"
#include "tokenizer/primitive_tokenizer.hpp"

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
			<< "  -h, --help [SUBJECT]     Shows this help display, or details about the specified subject, then exits." << '\n'
			<< "  -v, --version            Prints the current version number, then exits." << '\n'
			<< '\n'
			;
	}
};

template<typename Tr, typename... Ts>
struct basic_operator {
	using operation_t = std::function<Tr(Ts...)>;

	constexpr basic_operator(const operation_t& func) : func{ func } {}

	/// @brief	The function that performs the operation.
	operation_t func;

	/**
	 * @brief			Evaluates the result of the operation with the specified arguments.
	 * @param ...args -	The arguments of the operation, in order.
	 * @returns			The result of the operation.
	 */
	Tr evaluate(Ts&&... args) const
	{
		return func(std::forward<Ts>(args)...);
	}

	/// @brief	Gets the result of the operation.
	template<std::convertible_to<Ts>... Tu>
	inline Tr operator()(Tu&&... args) const
	{
		return evaluate(std::forward<Tu>(args)...);
	}
};

template<typename T, typename Returns = T>
using unary_operator = basic_operator<Returns, T>;
template<typename T1, typename T2 = T1, typename Returns = T1>
using binary_operator = basic_operator<Returns, T1, T2>;

static struct {
	using primitive = calc::expr::tkn::primitive;
	using primitive_type = calc::expr::PrimitiveTokenType;
	using complex = calc::expr::tkn::complex;
	using complex_type = calc::expr::ComplexTokenType;

	std::map<complex, std::vector<primitive>> sequenceMap{
		//{ { complex_type::, 0, "" }}
	};

} evaluator;

struct expression {
	std::list<calc::expr::tkn::primitive> tokens;

	expression evaluate() const
	{

	}

	std::optional<calc::Number> evaluate_result() const
	{

		// cannot evaluate expression
		return std::nullopt;
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
	std::cout << combine_tokens(calc::expr::PrimitiveTokenType::Unknown, std::vector<calc::expr::tkn::lexeme>{
		{ calc::expr::LexemeType::Alpha, 0, "start" },
		{ calc::expr::LexemeType::Alpha, 10, "end" },
		{ calc::expr::LexemeType::Alpha, 15, "LOL" },
	}) << '\n';

	std::cout << "|    ^    |    ^    |" << '\n';
	std::cout << "0    5    10   15   20" << '\n';

	//TreeNode<std::string> root{ "root" };

	////root.addChildren({ "asdf"});
	//root.addChildren({
	//	{ "Node1", { { "SubNode1", { { "SubNode1", { { "SubSubNode", {
	//		{ "SubSubSubNode", {
	//			{ "SubSubSubSubNode", { { "A" }, { "B", { { "SubNode1" } } } } },
	//				 { "sn", { { "A" }, { "B", { { "SubNode1" } } } } },
	//				 } }
	//				 } },
	//				 { "SubSubNode", {
	//					 { "SubSubSubNode", {
	//						 { "SubSubSubSubNode", { { "A" }, { "B", { { "SubNode1" } } } } },
	//				 { "sn", { { "A" }, { "B", { { "SubNode1" } } } } },
	//				 } }
	//				 } }, } } } }, { "SubNode2" }, { "SubNode3" } } },
	//				 { "Node1", { { "SubNode1", { { "SubNode1", { { "SubSubNode", { { "SubSubSubNode", { { "SubSubSubSubNode", { { "A" }, { "B" } } } } } } } } }, { "SubNode2" }, { "SubNode3" } } }, { "SubNode2" }, { "SubNode3" } } },
	//				 { "Node2" },
	//				 { "Node3", { { "SubNode1" }, { "SubNode2" }, { "SubNode3", { { "SubNode1" }, { "SubNode2", { { "A" }, { "B" } } }, { "SubNode3" } } } } },
	//				 });

	//std::cout << print_tree<TreeNode<std::string>, 3>(root, +[](TreeNode<std::string>&& node) { return node.getChildren(); });

	try {
		opt3::ArgManager args{ argc, argv,
			// argument defs:
			opt3::make_template(opt3::CaptureStyle::Optional, opt3::ConflictStyle::Conflict, 'h', "help"),
		};

		const auto& [procPath, procName] { env::PATH{}.resolve_split(argv[0]) };


		// v REMOVE WHEN DONE v

		using namespace calc::expr;

		//const auto expr{ "(5 / 0.56, ..5_015 )(0b1110 * 0xFF0 *a 8 * 0ib ${ABCD})" };
		const auto expr{ "(09(-2^5 * 3 - 7) / (4a % 3) + (a - sqrt(25, 50 asdf())) + 4 * 7) * (sin(60) + cos(-45)) - (log(100) + 8 / (tan(30) * exp(2)))" };
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
			std::cout << print_help(procName.generic_string(), args.getv_any<opt3::Flag, opt3::Option>('h', "help"));
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
