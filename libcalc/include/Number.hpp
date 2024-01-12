#pragma once
// 307lib
#include <var.hpp>
#include <strcore.hpp>
#include <strconv.hpp>

// STL
#include <cstdint>		//< for int typedefs
#include <cmath>		//< for std::trunc
#include <variant>		//< for std::variant

namespace calc {
	struct Number {
		using int_t = int64_t;
		using real_t = long double;
		using value_t = std::variant<int_t, real_t>;

		value_t value;

		constexpr Number() = default;
	#pragma region Integral Ctors
		constexpr Number(int8_t const value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(int16_t const value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(int32_t const value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(long const value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(long long const value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(uint8_t const value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(uint16_t const value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(uint32_t const value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(unsigned long const value) : value{ static_cast<int_t>(value) } {}
		constexpr Number(unsigned long long const value) : value{ static_cast<int_t>(value) } {}
	#pragma endregion Integral Ctors
	#pragma region Floating-Point Ctors
		constexpr Number(long double const value) : value{ static_cast<real_t>(value) } {}
		constexpr Number(double const value) : value{ static_cast<real_t>(value) } {}
		constexpr Number(float const value) : value{ static_cast<real_t>(value) } {}
	#pragma endregion Floating-Point Ctors
		constexpr Number(bool const value) : value{ static_cast<int_t>(value) } {}

	#pragma region Integral Conversion Operators
		constexpr operator int8_t() const noexcept { return cast_to<int8_t>(); }
		constexpr operator int16_t() const noexcept { return cast_to<int16_t>(); }
		constexpr operator int32_t() const noexcept { return cast_to<int32_t>(); }
		constexpr operator long() const noexcept { return cast_to<long>(); }
		constexpr operator long long() const noexcept { return cast_to<long long>(); }
		constexpr operator uint8_t() const noexcept { return cast_to<uint8_t>(); }
		constexpr operator uint16_t() const noexcept { return cast_to<uint16_t>(); }
		constexpr operator uint32_t() const noexcept { return cast_to<uint32_t>(); }
		constexpr operator unsigned long() const noexcept { return cast_to<unsigned long>(); }
		constexpr operator unsigned long long() const noexcept { return cast_to<unsigned long long>(); }
	#pragma endregion Integral Conversion Operators
	#pragma region Floating-Point Conversion Operators
		constexpr operator float() const noexcept { return cast_to<float>(); }
		constexpr operator double() const noexcept { return cast_to<double>(); }
		constexpr operator real_t() const noexcept { return cast_to<real_t>(); }
	#pragma endregion Floating-Point Conversion Operators
		constexpr operator bool() const noexcept { return !is_zero(); }

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
		constexpr bool has_integral_value() const noexcept
		{
			if (is_integer()) return true;
			const auto v{ std::get<real_t>(value) };
			return std::truncl(v) == v;
		}
		/// @brief	Determines if the numeric value is equal to zero.
		constexpr bool is_zero() const noexcept
		{
			return std::visit([](auto&& n) { return n == 0; }, value);
		}
		/// @brief	Determines if the numeric value is greater than zero.
		constexpr bool is_positive() const noexcept
		{
			return std::visit([](auto&& n) { return n > 0; }, value);
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
		/// @brief	Negation Operator
		friend Number operator-(const Number& n)
		{
			return std::visit([](auto&& n) { return Number{ -n }; }, n.value);
		}
		/// @brief	Bitwise OR operator
		friend Number operator|(const Number& l, const Number& r)
		{
			if (!l.has_integral_value())
				throw make_exception("Operator | (BitwiseOR) requires integral types, but the left-side operand was ", str::to_string(std::get<real_t>(l.value), 16), "!");
			else if (!r.has_integral_value())
				throw make_exception("Operator | (BitwiseOR) requires integral types, but the right-side operand was ", str::to_string(std::get<real_t>(r.value), 16), "!");

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
				throw make_exception("Operator & (BitwiseAND) requires integral types, but the left-side operand was ", str::to_string(std::get<real_t>(l.value), 16), "!");
			else if (!r.has_integral_value())
				throw make_exception("Operator & (BitwiseAND) requires integral types, but the right-side operand was ", str::to_string(std::get<real_t>(r.value), 16), "!");

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
				throw make_exception("Operator ^ (BitwiseXOR) requires integral types, but the left-side operand was ", str::to_string(std::get<real_t>(l.value), 16), "!");
			else if (!r.has_integral_value())
				throw make_exception("Operator ^ (BitwiseXOR) requires integral types, but the right-side operand was ", str::to_string(std::get<real_t>(r.value), 16), "!");

			return std::visit([](auto&& lVal, auto&& rVal) -> Number { {
					using Tl = std::decay_t<decltype(lVal)>;
					using Tr = std::decay_t<decltype(rVal)>;

					if constexpr (std::same_as<Tl, typename Number::int_t> && std::same_as<Tr, typename Number::int_t>)
						return Number{ lVal ^ rVal }; //< both sides are int
					else return Number{ static_cast<typename Number::int_t>(lVal) ^ static_cast<typename Number::int_t>(rVal) };
				} }, l.value, r.value);
		}
		/// @brief	Bitwise NOT operator
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
		/// @brief	Bitshift-right operator
		friend Number operator>>(Number const& l, Number const& r)
		{
			if (!l.has_integral_value())
				throw make_exception("Operator >> (BitshiftRight) requires integral types, but the left-side operand was ", str::to_string(std::get<real_t>(l.value), 16), "!");
			else if (!r.has_integral_value())
				throw make_exception("Operator >> (BitshiftRight) requires integral types, but the right-side operand was ", str::to_string(std::get<real_t>(r.value), 16), "!");

			return std::visit([](auto&& lVal, auto&& rVal) { {
					using Tl = std::decay_t<decltype(lVal)>;
					using Tr = std::decay_t<decltype(rVal)>;

					if constexpr (std::same_as<Tl, typename Number::int_t> && std::same_as<Tr, typename Number::int_t>)
						return Number{ lVal >> rVal }; //< both sides are int
					else return Number{ static_cast<typename Number::int_t>(lVal) >> static_cast<typename Number::int_t>(rVal) };
				} }, l.value, r.value);
		}
		/// @brief	Bitshift-left operator
		friend Number operator<<(Number const& l, Number const& r)
		{
			if (!l.has_integral_value())
				throw make_exception("Operator << (BitshiftLeft) requires integral types, but the left-side operand was ", str::to_string(std::get<real_t>(l.value), 16), "!");
			else if (!r.has_integral_value())
				throw make_exception("Operator << (BitshiftLeft) requires integral types, but the right-side operand was ", str::to_string(std::get<real_t>(r.value), 16), "!");

			return std::visit([](auto&& lVal, auto&& rVal) { {
					using Tl = std::decay_t<decltype(lVal)>;
					using Tr = std::decay_t<decltype(rVal)>;

					if constexpr (std::same_as<Tl, typename Number::int_t> && std::same_as<Tr, typename Number::int_t>)
						return Number{ lVal << rVal }; //< both sides are int
					else return Number{ static_cast<typename Number::int_t>(lVal) << static_cast<typename Number::int_t>(rVal) };
				} }, l.value, r.value);
		}

		/// @returns	true when the value is zero; otherwise, false.
		friend bool operator!(Number const& n)
		{
			return n.is_zero();
		}
		/// @returns	true when a is less than b; otherwise, false.
		friend bool operator<(Number const& a, Number const& b)
		{
			return std::visit([](auto&& a, auto&& b) { return a < b; }, a.value, b.value);
		}
		/// @returns	true when a is less than or equal to b; otherwise, false.
		friend bool operator<=(Number const& a, Number const& b)
		{
			return std::visit([](auto&& a, auto&& b) { return a <= b; }, a.value, b.value);
		}
		/// @returns	true when a is greater than b; otherwise, false.
		friend bool operator>(Number const& a, Number const& b)
		{
			return std::visit([](auto&& a, auto&& b) { return a > b; }, a.value, b.value);
		}
		/// @returns	true when a is greater than or equal to b; otherwise, false.
		friend bool operator>=(Number const& a, Number const& b)
		{
			return std::visit([](auto&& a, auto&& b) { return a >= b; }, a.value, b.value);
		}

		template<var::numeric T>
		constexpr T cast_to() const noexcept
		{
			return std::visit([](auto const& value) { return static_cast<T>(value); }, value);
		}

		friend std::ostream& operator<<(std::ostream& os, const Number& n)
		{
			std::visit([&](auto&& value) { os << $fwd(value); }, n.value);
			return os;
		}
	};
}
