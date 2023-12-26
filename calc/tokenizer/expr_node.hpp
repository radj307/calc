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

	struct expr_node : public TreeNode<vtoken> {
		template<typename T>
		constexpr bool is_type() const noexcept { return std::holds_alternative<T>(value); }
		constexpr bool is_primitive() const noexcept { return is_type<primitive>(); }
		constexpr bool is_complex() const noexcept { return is_type<complex>(); }


	};
}
