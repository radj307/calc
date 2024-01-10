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

		/// @brief	Determines if the numeric value is an integral or not.
		bool has_integral_value() const noexcept
		{
			if (is_integer()) return true;
			const auto v{ std::get<real_t>(value) };
			return std::truncl(v) == v;
		}

		friend bool operator==(const Number& l, const Number& r)
		{
			return std::visit([](auto&& lVal, auto&& rVal) { return lVal == rVal; }, l.value, r.value);
		}
		friend bool operator!=(const Number& l, const Number& r)
		{
			return std::visit([](auto&& lVal, auto&& rVal) { return lVal != rVal; }, l.value, r.value);
		}

		/// @brief	Addition operator
		friend Number operator+(const Number& l, const Number& r)
		{
			return std::visit([](auto&& l, auto&& r) { return Number{ static_cast<long double>(l) + static_cast<long double>(r) }; }, l.value, r.value);
		}
		/// @brief	Subtraction operator
		friend Number operator-(const Number& l, const Number& r)
		{
			return std::visit([](auto&& l, auto&& r) { return Number{ static_cast<long double>(l) - static_cast<long double>(r) }; }, l.value, r.value);
		}
		/// @brief	Multiplication operator
		friend Number operator*(const Number& l, const Number& r)
		{
			return std::visit([](auto&& l, auto&& r) { return Number{ static_cast<long double>(l) * static_cast<long double>(r) }; }, l.value, r.value);
		}
		/// @brief	Division operator
		friend Number operator/(const Number& l, const Number& r)
		{
			return std::visit([](auto&& l, auto&& r) { return Number{ static_cast<long double>(l) / static_cast<long double>(r) }; }, l.value, r.value);
		}
		/// @brief	Modulo operator
		friend Number operator%(const Number& l, const Number& r)
		{
			return std::visit([](auto&& lVal, auto&& rVal) { {
					using Tl = std::decay_t<decltype(lVal)>;
					using Tr = std::decay_t<decltype(rVal)>;

					if constexpr (std::same_as<Tl, typename Number::int_t> && std::same_as<Tr, typename Number::int_t>)
						return Number{ lVal % rVal }; //< lVal & rVal are both int
					else return Number{ fmodl(lVal, rVal) };
				} }, l.value, r.value);
		}
		/// @brief	Bitwise OR operator
		friend Number operator|(const Number& l, const Number& r)
		{
			if (!l.has_integral_value())
				throw make_exception("Operator | (BitwiseOR) requires integral types, but the left-side operand was ", std::get<real_t>(l.value), "!");
			else if (!r.has_integral_value())
				throw make_exception("Operator | (BitwiseOR) requires integral types, but the right-side operand was ", std::get<real_t>(r.value), "!");

			return std::visit([](auto&& lVal, auto&& rVal) -> Number { {
					using Tl = std::decay_t<decltype(lVal)>;
					using Tr = std::decay_t<decltype(rVal)>;

					if constexpr (std::same_as<Tl, typename Number::int_t> && std::same_as<Tr, typename Number::int_t>)
						return Number{ lVal | rVal }; //< both sides are int
					else return Number{ static_cast<typename Number::int_t>(lVal) | static_cast<typename Number::int_t>(rVal) };
				} }, l.value, r.value);
		}
		/// @brief	Bitwise AND operator
		friend Number operator&(const Number& l, const Number& r)
		{
			if (!l.has_integral_value())
				throw make_exception("Operator & (BitwiseAND) requires integral types, but the left-side operand was ", std::get<real_t>(l.value), "!");
			else if (!r.has_integral_value())
				throw make_exception("Operator & (BitwiseAND) requires integral types, but the right-side operand was ", std::get<real_t>(r.value), "!");

			return std::visit([](auto&& lVal, auto&& rVal) -> Number { {
					using Tl = std::decay_t<decltype(lVal)>;
					using Tr = std::decay_t<decltype(rVal)>;

					if constexpr (std::same_as<Tl, typename Number::int_t> && std::same_as<Tr, typename Number::int_t>)
						return Number{ lVal & rVal }; //< both sides are int
					else return Number{ static_cast<typename Number::int_t>(lVal) & static_cast<typename Number::int_t>(rVal) };
				} }, l.value, r.value);
		}
		/// @brief	Bitwise XOR operator
		friend Number operator^(const Number& l, const Number& r)
		{
			if (!l.has_integral_value())
				throw make_exception("Operator ^ (BitwiseXOR) requires integral types, but the left-side operand was ", std::get<real_t>(l.value), "!");
			else if (!r.has_integral_value())
				throw make_exception("Operator ^ (BitwiseXOR) requires integral types, but the right-side operand was ", std::get<real_t>(r.value), "!");

			return std::visit([](auto&& lVal, auto&& rVal) -> Number { {
					using Tl = std::decay_t<decltype(lVal)>;
					using Tr = std::decay_t<decltype(rVal)>;

					if constexpr (std::same_as<Tl, typename Number::int_t> && std::same_as<Tr, typename Number::int_t>)
						return Number{ lVal ^ rVal }; //< both sides are int
					else return Number{ static_cast<typename Number::int_t>(lVal) ^ static_cast<typename Number::int_t>(rVal) };
				} }, l.value, r.value);
		}
		friend Number operator~(const Number& n)
		{
			if (!n.has_integral_value())
				throw make_exception("Operator ~ (BitwiseNOT) requires integral type!");

			return std::visit([](auto&& n) { {
					if constexpr (std::same_as<std::decay_t<decltype(n)>, typename Number::int_t>)
						return Number{ ~n }; //< is int
					else return Number{ ~static_cast<typename Number::int_t>(n) };
				} }, n.value);
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
			std::visit([&](auto&& value) { os << $fwd(value); }, n.value);
			return os;
		}
	};
}
