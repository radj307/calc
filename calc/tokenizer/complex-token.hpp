#pragma once
#include "token.hpp"

#include <vector>
#include <variant>

namespace calc::expr::tkn {

	using generic = basic_token<std::uint8_t>;

	template<var::derived_from_templated<basic_token> T>
	constexpr generic to_generic_token(T const& token)
	{
		return token.generic_token<std::uint8_t>();
	}

	/// @brief	A complex token, the most advanced kind of token subtype.
	class complex : public basic_token<ComplexTokenType> {
		using base_t = basic_token<ComplexTokenType>;
		using type_t = typename base_t::type_t;
		using complex_ptr = std::shared_ptr<complex>;

		using vtoken_t = std::variant<primitive, complex_ptr>;

		template<class Getter>
		static constexpr auto vtoken_get(vtoken_t const& vtoken, const Getter& getter)
		{
			return std::visit([&getter](auto&& value) {
				using T = std::decay_t<decltype(value)>;
				if constexpr (std::same_as<T, complex_ptr>)
					return getter(to_generic_token(static_cast<basic_token<ComplexTokenType>>(*value)));
				else return getter(to_generic_token(value));
			}, vtoken);
		}

		static constexpr std::vector<vtoken_t> to_vtokens(std::vector<primitive> const& primitives)
		{
			std::vector<vtoken_t> vec;
			vec.reserve(primitives.size());
			vec.insert(vec.end(), primitives.begin(), primitives.end());
			return vec;
		}
		static constexpr std::vector<generic> to_generic_tokens(std::vector<vtoken_t> const& vtokens)
		{
			std::vector<generic> vec;
			vec.reserve(vtokens.size());
			for (const auto& vtkn : vtokens) {
				vec.emplace_back(std::visit([](auto&& tkn) {
					if constexpr (std::same_as<std::decay_t<decltype(tkn)>, complex_ptr>)
						return to_generic_token(static_cast<basic_token<ComplexTokenType>>(*tkn));
					else return to_generic_token(tkn);
				}, vtkn));
			}
			return vec;
		}

	public:
		constexpr complex(const type_t& type, const std::vector<primitive>& primitives) : base_t(type, combine_tokens(ComplexTokenType::Unknown, primitives)), subtokens{ to_vtokens(primitives) } {}
		constexpr complex(const type_t& type, const std::vector<vtoken_t>& subtokens) : base_t(type, combine_tokens(ComplexTokenType::Unknown, to_generic_tokens(subtokens))), subtokens{ subtokens } {}

		/// @brief	A vector of the primitives that compose this complex token instance.
		std::vector<vtoken_t> subtokens;
	};
}
