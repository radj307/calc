#pragma once
#include <var.hpp>
#include <strcore.hpp>

#include <cstdint>
#include <variant>
#include <utility>

namespace calc {
	struct Number {
		using int_t = size_t;
		using real_t = double;
		using value_t = std::variant<int_t, real_t>;

		value_t value;

		constexpr Number() = default;
		constexpr Number(const value_t& value) : value{ value } {}
		constexpr Number(value_t&& value) : value{ std::move(value) } {}

		static Number from_binary(std::string const& binaryNumber)
		{
			return{ str::tonumber<int_t>(binaryNumber, 2) };
		}
		static Number from_octal(std::string const& octalNumber)
		{
			return{ str::tonumber<int_t>(octalNumber, 8) };
		}
		static Number from_hex(std::string const& hexNumber)
		{
			return{ str::tonumber<int_t>(hexNumber, 16) };
		}

		/**
		 * @brief		Checks if this value is the specified type.
		 * @tparam T  - The type to check.
		 * @returns		true when this value is the specified type; otherwise, false.
		 */
		template<var::any_same<int_t, real_t> T>
		constexpr bool is_type() const noexcept { return std::holds_alternative<T>(value); }

		constexpr bool is_integer() const noexcept { return is_type<int_t>(); }
		constexpr bool is_real_number() const noexcept { return is_type<real_t>(); }

		template<var::any_same_or_convertible<int_t, real_t> T>
		friend bool operator==(const Number& l, const T r)
		{
			return std::visit([&r](auto&& value) {
				return value == r;
			}, l.value);
		}
		friend bool operator==(const Number& l, const Number& r)
		{
			return std::visit([](auto&& lVal, auto&& rVal) {
				return lVal == rVal;
			}, l.value, r.value);
		}

		template<var::any_same_or_convertible_to<int_t, real_t> T>
		Number& operator=(T&& v)
		{
			if constexpr (std::same_as<T, int_t>)
				value = static_cast<int_t>(std::move(v));
			else if constexpr (std::same_as<T, real_t>)
				value = static_cast<real_t>(std::move(v));
			return *this;
		}
	};
}
