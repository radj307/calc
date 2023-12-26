#pragma once
#include "token.hpp"
#include "../TreeNode.hpp"

#include <vector>
#include <variant>

namespace calc::expr::tkn {

	using vtoken = std::variant<primitive, complex>;


	inline std::ostream& operator<<(std::ostream& os, const vtoken& v)
	{
		std::visit([&os](auto&& value) {
			os << '[' << get_name(value.type) << "]:" << '\"' << value << '\"';
		}, v);
		return os;
	}

	using expr_node = TreeNode<vtoken>;
}
