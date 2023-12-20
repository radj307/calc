﻿#include "ExpressionTokenizer.hpp"
#include "Number.hpp"

#include <TermAPI.hpp>		//< for console helpers
#include <color-sync.hpp>	//< for sync
#include <opt3.hpp>			//< for ArgManager
#include <envpath.hpp>		//< for PATH

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
			<< "  -m, --mode <MODE>        Specifies the mode of operation to use." << '\n'
			<< "                           - Standard  (Default)" << '\n'
			<< "                           - Statistics" << '\n'
			;
	}
};

int main(const int argc, char** argv)
{
	try {
		opt3::ArgManager args{ argc, argv,
			// argument defs:
		};

		const auto& [procPath, procName] { env::PATH{}.resolve_split(argv[0]) };

		if (/*args.empty() ||*/ args.check_any<opt3::Flag, opt3::Option>('h', "help")) {
			std::cout << print_help(procName.generic_string());
			return 0;
		}
		else if (args.check_any<opt3::Flag, opt3::Option>('v', "version")) {
			std::cout /* << calc_VERSION_EXTENDED */;
			return 0;
		}

		const auto& expression{ str::join(args.getv_all<opt3::Parameter>()) };

		using namespace calc::expr;

		const auto expr{ "(5 / 0.56 )(0b1110 * 0xFF0)" };
		const auto lexemes = token::lexer{ expr }.get_lexemes(false);

		std::cout << "INPUT: " << expr << '\n';

		for (const auto& it : lexemes) {
			std::stringstream tempbuf;
			tempbuf << '[' << it.pos;
			if (const auto& endPos{ it.getEndPos() - 1 }; it.pos != endPos)
				tempbuf << "-" << endPos;
			tempbuf << "] (" << it.getEndPos() - it.pos << ')';
			const auto s{ tempbuf.str() };
			std::cout << s << indent(20, s.size()) << it.value << '\n';
		}

		return 0;
	} catch (const std::exception& ex) {
		std::cerr << csync.get_fatal() << ex.what() << std::endl;
		return 1;
	}
}
