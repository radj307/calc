#pragma once
#include <var.hpp>
#include <strcore.hpp>

#include <cstdint>
#include <variant>
#include <utility>

namespace calc {
	struct Number {
		using int_t = int64_t;
		using real_t = long double;
		using value_t = std::variant<int_t, real_t>;

		value_t value;

		constexpr Number() = default;
	#pragma region Integral Ctors
		constexpr Number(int8_t const& value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(int8_t&& value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(int16_t const& value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(int16_t&& value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(int32_t const& value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(int32_t&& value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(long const& value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(long&& value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(int64_t const& value) : value{ value } {}
		constexpr Number(int64_t&& value) : value{ std::forward<int_t>(value) } {}
		constexpr Number(uint8_t const& value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(uint8_t&& value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(uint16_t const& value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(uint16_t&& value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(uint32_t const& value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(uint32_t&& value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(unsigned long const& value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(unsigned long&& value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(uint64_t const& value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(uint64_t&& value) : value{ static_cast<int_t>(value) } {}
	#pragma endregion Integral Ctors
	#pragma region Floating-Point Ctors
		constexpr Number(real_t const& value) : value{ value } {}
		constexpr Number(real_t&& value) : value{ std::forward<real_t>(value) } {}
		constexpr Number(double const& value) : value{ static_cast<real_t>(value) } {}
		constexpr Number(double&& value) : value{ static_cast<real_t>(value) } {}
		constexpr Number(float const& value) : value{ static_cast<real_t>(value) } {}
		constexpr Number(float&& value) : value{ static_cast<real_t>(value) } {}
	#pragma endregion Floating-Point Ctors

	#pragma region Integral Conversion Operators
		constexpr operator int8_t() const noexcept { return cast_to<int8_t>(); }
		constexpr operator int16_t() const noexcept { return cast_to<int16_t>(); }
		constexpr operator int32_t() const noexcept { return cast_to<int32_t>(); }
		constexpr operator long() const noexcept { return cast_to<long>(); }
		constexpr operator int64_t() const noexcept { return cast_to<int64_t>(); }
		constexpr operator uint8_t() const noexcept { return cast_to<uint8_t>(); }
		constexpr operator uint16_t() const noexcept { return cast_to<uint16_t>(); }
		constexpr operator uint32_t() const noexcept { return cast_to<uint32_t>(); }
		constexpr operator unsigned long() const noexcept { return cast_to<unsigned long>(); }
		constexpr operator uint64_t() const noexcept { return cast_to<uint64_t>(); }
	#pragma endregion Integral Conversion Operators
	#pragma region Floating-Point Conversion Operators
		constexpr operator float() const noexcept { return cast_to<float>(); }
		constexpr operator double() const noexcept { return cast_to<double>(); }
		constexpr operator real_t() const noexcept { return cast_to<real_t>(); }
	#pragma endregion Floating-Point Conversion Operators

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
		/// @returns	true when the underlying type is an integer; otherwise, false.
		constexpr bool is_integer() const noexcept { return is_type<int_t>(); }
		/// @returns	true when the underlying type is a floating-point; otherwise, false.
		constexpr bool is_real() const noexcept { return is_type<real_t>(); }

		friend bool operator==(const Number& l, const Number& r)
		{
			return std::visit([](auto&& lVal, auto&& rVal) {
				return lVal == rVal;
							  }, l.value, r.value);
		}
		friend bool operator!=(const Number& l, const Number& r)
		{
			return std::visit([](auto&& lVal, auto&& rVal) {
				return lVal != rVal;
							  }, l.value, r.value);
		}

		friend Number operator/(const Number& l, const Number& r)
		{
			return std::visit([](auto&& lVal, auto&& rVal) {
				return Number{ lVal / rVal };
			}, l.value, r.value);
		}
		friend Number operator*(const Number& l, const Number& r)
		{
			return std::visit([](auto&& lVal, auto&& rVal) {
				return Number{ lVal * rVal };
			}, l.value, r.value);
		}
		friend Number operator+(const Number& l, const Number& r)
		{
			return std::visit([](auto&& lVal, auto&& rVal) {
				return Number{ lVal + rVal };
			}, l.value, r.value);
		}
		friend Number operator-(const Number& l, const Number& r)
		{
			return std::visit([](auto&& lVal, auto&& rVal) {
				return Number{ lVal - rVal };
			}, l.value, r.value);
		}

		friend Number& operator/=(Number& l, const Number& r)
		{
			std::visit([](auto& lVal, auto&& rVal) {
				lVal /= rVal;
			}, l.value, r.value);
			return l;
		}
		friend Number& operator*=(Number& l, const Number& r)
		{
			std::visit([](auto& lVal, auto&& rVal) {
				lVal *= rVal;
			}, l.value, r.value);
			return l;
		}
		friend Number& operator+=(Number& l, const Number& r)
		{
			std::visit([](auto& lVal, auto&& rVal) {
				lVal += rVal;
			}, l.value, r.value);
			return l;
		}
		friend Number& operator-=(Number& l, const Number& r)
		{
			std::visit([](auto& lVal, auto&& rVal) {
				lVal -= rVal;
			}, l.value, r.value);
			return l;
		}

		template<var::numeric T>
		constexpr T cast_to() const noexcept
		{
			return std::visit([](auto const& value) { return static_cast<T>(value); }, value);
		}

		template<var::any_same_or_convertible_to<int_t, real_t> T>
		Number& operator=(T&& v)
		{
			if constexpr (var::same_or_convertible<T, int_t>)
				value = static_cast<int_t>(std::move(v));
			else if constexpr (var::same_or_convertible<T, real_t>)
				value = static_cast<real_t>(std::move(v));
			return *this;
		}

		friend std::ostream& operator<<(std::ostream& os, const Number& n)
		{
			std::visit([&](auto&& value) {
				os << $fwd(value);
					   }, n.value);
			return os;
		}
	};
}
