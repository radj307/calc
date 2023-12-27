#pragma once
#include "token.hpp"
#include "../TreeNode.hpp"

#include <var.hpp>

#include <vector>
#include <variant>

namespace calc::expr::tkn {

	struct vtoken : std::variant<primitive, complex> {
		using base_t = std::variant<primitive, complex>;
		using base_t::base_t; //< use base ctors

		bool isAnyType(std::same_as<ComplexTokenType> auto const&... tokenTypes) const noexcept
		{
			return std::holds_alternative<complex>(*this)
				&& std::visit([&tokenTypes...](auto&& tkn) {
					return var::variadic_or((tkn.type == tokenTypes)...);
				}, *this);
		}
		bool isAnyType(std::same_as<PrimitiveTokenType> auto const&... tokenTypes) const noexcept
		{
			return std::holds_alternative<primitive>(*this)
				&& std::visit([&tokenTypes...](auto&& tkn) {
					return var::variadic_or((tkn.type == tokenTypes)...);
				}, *this);
		}
	};

	inline std::ostream& operator<<(std::ostream& os, const vtoken& v)
	{
		std::visit([&os](auto&& value) {
			os << '[' << get_name(value.type) << "]:" << '\"' << value << '\"';
		}, v);
		return os;
	}

	using expr_node = TreeNode<vtoken>;
}
