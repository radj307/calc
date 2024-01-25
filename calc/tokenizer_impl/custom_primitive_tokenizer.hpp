#pragma once
#include "../global.h"	//< for csync

// libcalc
#include <tokenizer/primitive_tokenizer.hpp> //< for calc::expr::tkn::primitive_tokenizer

namespace calc::expr::tkn {
	// adds color to primitive_tokenizer error messages
	class custom_primitive_tokenizer : public primitive_tokenizer {
		using base_t = primitive_tokenizer;
		using const_iterator = typename std::vector<lexeme>::const_iterator;

	protected:
		std::string make_error_msg(const_iterator const& beginTkn, const_iterator const& endTkn, const_iterator const& errorTkn, std::string const& message, size_t const indent_sz = 10) const override
		{
			std::stringstream ss;

			const auto expr_str{ stringify_lexemes(begin, end) };

			size_t endPos{ expr_str.size() };
			if (endTkn != end) endPos = endTkn->pos;

			// print the entire expression
			ss << expr_str << '\n';

			// print the error indicator
			ss
				<< indent(indent_sz + beginTkn->pos)
				<< csync(color::dark_red) << indent(errorTkn->pos, beginTkn->pos, '~')
				<< csync(color::red) << indent(errorTkn->getEndPos(), errorTkn->pos, '^')
				<< csync(color::dark_red) << indent(endPos, errorTkn->getEndPos(), '~')
				<< csync() << '\n'
				;

			// print the error message
			if (!message.empty())
				ss << indent(indent_sz) << message << '\n';

			return ss.str();
		}

	public:
		using base_t::base_t;
	};
}
