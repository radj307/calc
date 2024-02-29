#pragma once
#include "calc_exception.hpp"

// 307lib
#include <var.hpp>
#include <make_exception.hpp>

// Boost::MultiPrecision
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>

// STL
#include <variant>		//< for std::variant
#include <limits>		//< for std::numeric_limits
#include <typeinfo>		//< for typeid

namespace calc {
#pragma region precision_exception

	static bool NUMBER_THROW_ON_UNSAFE_CAST{ true };

	struct precision_exception : calc_exception {
		precision_exception(std::string const& message) :
			calc_exception(message, SuggestedFix::UnsafeCast)
		{}
		precision_exception(std::string const& message, SuggestedFix const extraSuggestedFixes) :
			calc_exception(message, SuggestedFix::UnsafeCast | extraSuggestedFixes)
		{}
	};

#pragma endregion precision_exception

#pragma region Number

	struct Number {
		using int_t = boost::multiprecision::cpp_int;
		using real_t = boost::multiprecision::cpp_dec_float_100;
		using value_t = std::variant<int_t, real_t>;

		value_t value;

	#pragma region Constructors

		WINCONSTEXPR Number() = default;
	#pragma region Integral Ctors
		WINCONSTEXPR Number(int8_t const value) : value{ static_cast<int_t>(value) } {}
		WINCONSTEXPR Number(int16_t const value) : value{ static_cast<int_t>(value) } {}
		WINCONSTEXPR Number(int32_t const value) : value{ static_cast<int_t>(value) } {}
		WINCONSTEXPR Number(long const value) : value{ static_cast<int_t>(value) } {}
		WINCONSTEXPR Number(long long const value) : value{ static_cast<int_t>(value) } {}
		WINCONSTEXPR Number(uint8_t const value) : value{ static_cast<int_t>(value) } {}
		WINCONSTEXPR Number(uint16_t const value) : value{ static_cast<int_t>(value) } {}
		WINCONSTEXPR Number(uint32_t const value) : value{ static_cast<int_t>(value) } {}
		WINCONSTEXPR Number(unsigned long const value) : value{ static_cast<int_t>(value) } {}
		WINCONSTEXPR Number(unsigned long long const value) : value{ static_cast<int_t>(value) } {}
	#pragma endregion Integral Ctors
	#pragma region Floating-Point Ctors
		WINCONSTEXPR Number(long double const value) : value{ (std::trunc(value) == value) ? value_t{ static_cast<int_t>(value) } : value_t{ static_cast<real_t>(value) } } {}
		WINCONSTEXPR Number(double const value) : value{ (std::trunc(value) == value) ? value_t{ static_cast<int_t>(value) } : value_t{ static_cast<real_t>(value) } } {}
		WINCONSTEXPR Number(float const value) : value{ (std::trunc(value) == value) ? value_t{ static_cast<int_t>(value) } : value_t{ static_cast<real_t>(value) } } {}
	#pragma endregion Floating-Point Ctors
		WINCONSTEXPR Number(int_t const& value) : value{ value } {}
		WINCONSTEXPR Number(real_t const& value) : value{ (boost::multiprecision::trunc(value) == value) ? value_t{ static_cast<int_t>(value) } : value_t{ value } } {}
		WINCONSTEXPR Number(bool const value) : value{ static_cast<int_t>(value) } {}

	#pragma endregion Constructors

	#pragma region Conversion Operators

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
		constexpr operator long double() const noexcept { return cast_to<long double>(); }
	#pragma endregion Floating-Point Conversion Operators
		constexpr operator bool() const noexcept { return !is_zero(); }

	#pragma endregion Conversion Operators

	#pragma region Methods

		template<var::any_same<int_t, real_t> T>
		constexpr bool is_type() const noexcept { return std::holds_alternative<T>(value); }
		constexpr bool is_integer() const noexcept { return is_type<int_t>(); }
		constexpr bool is_real() const noexcept { return is_type<real_t>(); }

		WINCONSTEXPR bool has_integral_value() const noexcept
		{
			if (is_integer()) return true;
			const auto v{ cast_to_<real_t>() };
			if (!BOOST_MP_ISFINITE(v)) return false;
			try {
				// this can throw when exceptionally large numbers are present
				return boost::multiprecision::trunc(v) == v;
			} catch (...) {
				return false;
			}
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

	#pragma region can_fit_value
		template<std::integral T>
		constexpr bool can_fit_value() const noexcept
		{
			if (!has_integral_value()) return false;
			const int_t v{ cast_to_<int_t>() };
			return v >= std::numeric_limits<T>::min() && v <= std::numeric_limits<T>::max();
		}
		template<std::floating_point T>
		constexpr bool can_fit_value() const noexcept
		{
			const real_t v{ cast_to_<real_t>() };
			return v >= std::numeric_limits<T>::min() && v <= std::numeric_limits<T>::max();
		}
		template<std::same_as<typename Number::int_t> T>
		constexpr bool can_fit_value() const noexcept
		{
			return is_integer() || has_integral_value();
		}
		template<std::same_as<typename Number::real_t> T>
		constexpr bool can_fit_value() const noexcept
		{
			return true;
		}
		template<typename T>
		constexpr std::enable_if_t<(!std::floating_point<T> && !std::integral<T>), bool> can_fit_value() const noexcept
		{
			return false;
		}
	#pragma endregion can_fit_value

	private:
		// Internal cast to. 
		template<typename T>
		constexpr T cast_to_() const
		{
			return std::visit([](auto const& value) { return static_cast<T>(value); }, value);
		}
	public:

		/**
		 * @brief		Casts the number's value to the specified type, T.
		 * @tparam T  -	The type to cast the value to.
		 * @returns		The number's value static_cast-ed to type T.
		 * @exception	Throws ex::except when NUMBER_THROW_ON_UNSAFE_CAST is
		 *				 true and casting to type T would lose precision.
		 */
		template<typename T>
		constexpr T cast_to() const
		{
			if (NUMBER_THROW_ON_UNSAFE_CAST && !this->can_fit_value<T>()) {
				throw precision_exception{ str::stringify(
					"Cannot convert from \"",
					std::visit([](auto&& value) -> std::string { {
							using U = std::decay_t<decltype(value)>;
							if constexpr (std::same_as<U, typename Number::int_t>)
								return "cpp_int"; //< cpp_int
							else
								return "cpp_float"; //< cpp_dec_float_100
						} }, value),
					"\" to type \"",
					std::visit([&]() -> std::string { {
							using U = std::decay_t<T>;
							if constexpr (std::same_as<U, typename Number::int_t>)
								return "cpp_int"; //< cpp_int
							else if constexpr (std::same_as<U, typename Number::real_t>)
								return "cpp_float"; //< cpp_dec_float_100
							else
								return typeid(U).name(); //< other
						} }),
					"\" because the conversion would lose precision!"
				), SuggestedFix::SmallerNumbers };
			}
			return cast_to_<T>();
		}

		std::string str() const
		{
			return str::stringify(*this);
		}

	#pragma endregion Methods

	#pragma region Operators

		/// @brief	Addition operator
		friend WINCONSTEXPR Number operator+(const Number& l, const Number& r)
		{
			return std::visit([](auto&& l, auto&& r) { {
					if constexpr (std::same_as<std::decay_t<decltype(l)>, std::decay_t<decltype(r)>>)
						return Number{ l + r };
					else return Number{ static_cast<typename Number::real_t>(l) + static_cast<typename Number::real_t>(r) };
				} }, l.value, r.value);
		}
		friend WINCONSTEXPR Number operator+=(Number& l, const Number& r)
		{
			return l = l + r;
		}
		/// @brief	Subtraction operator
		friend WINCONSTEXPR Number operator-(const Number& l, const Number& r)
		{
			return std::visit([](auto&& l, auto&& r) { {
					if constexpr (std::same_as<std::decay_t<decltype(l)>, std::decay_t<decltype(r)>>)
						return Number{ l - r };
					else return Number{ static_cast<typename Number::real_t>(l) - static_cast<typename Number::real_t>(r) };
				} }, l.value, r.value);
		}
		friend WINCONSTEXPR Number& operator-=(Number& l, Number const& r)
		{
			return l = l - r;
		}
		/// @brief	Multiplication operator
		friend WINCONSTEXPR Number operator*(const Number& l, const Number& r)
		{
			return std::visit([](auto&& l, auto&& r) { {
					if constexpr (std::same_as<std::decay_t<decltype(l)>, std::decay_t<decltype(r)>>)
						return Number{ l * r };
					else return Number{ static_cast<typename Number::real_t>(l) * static_cast<typename Number::real_t>(r) };
				} }, l.value, r.value);
		}
		friend WINCONSTEXPR Number& operator*=(Number& l, Number const& r)
		{
			return l = l * r;
		}
		/// @brief	Division operator
		friend WINCONSTEXPR Number operator/(const Number& l, const Number& r)
		{
			return std::visit([](auto&& l, auto&& r) { {
					if constexpr (std::same_as<std::decay_t<decltype(l)>, std::decay_t<decltype(r)>>)
						return Number{ l / r };
					else return Number{ static_cast<typename Number::real_t>(l) / static_cast<typename Number::real_t>(r) };
				} }, l.value, r.value);
		}
		friend WINCONSTEXPR Number& operator/=(Number& l, Number const& r)
		{
			return l = l / r;
		}
		/// @brief	Modulo operator
		friend WINCONSTEXPR Number operator%(const Number& l, const Number& r)
		{
			return std::visit([](auto&& lVal, auto&& rVal) { {
					using Tl = std::decay_t<decltype(lVal)>;
					using Tr = std::decay_t<decltype(rVal)>;

					if constexpr (std::same_as<Tl, typename Number::int_t> && std::same_as<Tr, typename Number::int_t>)
						return Number{ lVal % rVal }; //< lVal & rVal are both int
					else return Number{ boost::multiprecision::fmod(static_cast<typename Number::real_t>(lVal), static_cast<typename Number::real_t>(rVal)) };
				} }, l.value, r.value);
		}
		/// @brief	Negation Operator
		friend WINCONSTEXPR Number operator-(const Number& n)
		{
			return std::visit([](auto&& n) { return Number{ -n }; }, n.value);
		}
		/// @brief	Bitwise OR operator
		friend WINCONSTEXPR Number operator|(const Number& l, const Number& r)
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
		friend WINCONSTEXPR Number operator&(const Number& l, const Number& r)
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
		friend WINCONSTEXPR Number operator^(const Number& l, const Number& r)
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
		/// @brief	Bitwise NOT operator
		friend WINCONSTEXPR Number operator~(const Number& n)
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
		friend WINCONSTEXPR Number operator>>(Number const& l, Number const& r)
		{
			if (!l.has_integral_value())
				throw make_exception("Operator >> (BitshiftRight) requires integral types, but the left-side operand was ", std::get<real_t>(l.value), "!");
			else if (!r.has_integral_value())
				throw make_exception("Operator >> (BitshiftRight) requires integral types, but the right-side operand was ", std::get<real_t>(r.value), "!");

			return std::visit([](auto&& lVal, auto&& rVal) { {
					using Tl = std::decay_t<decltype(lVal)>;
					using Tr = std::decay_t<decltype(rVal)>;

					if constexpr (std::same_as<Tl, typename Number::int_t> && std::same_as<Tr, typename Number::int_t>)
						return Number{ lVal >> static_cast<int>(rVal) }; //< both sides are int
					else return Number{ static_cast<typename Number::int_t>(lVal) >> static_cast<int>(rVal) };
				} }, l.value, r.value);
		}
		/// @brief	Bitshift-left operator
		friend WINCONSTEXPR Number operator<<(Number const& l, Number const& r)
		{
			if (!l.has_integral_value())
				throw make_exception("Operator << (BitshiftLeft) requires integral types, but the left-side operand was ", std::get<real_t>(l.value), "!");
			else if (!r.has_integral_value())
				throw make_exception("Operator << (BitshiftLeft) requires integral types, but the right-side operand was ", std::get<real_t>(r.value), "!");

			return std::visit([](auto&& lVal, auto&& rVal) { {
					using Tl = std::decay_t<decltype(lVal)>;
					using Tr = std::decay_t<decltype(rVal)>;

					if constexpr (std::same_as<Tl, typename Number::int_t> && std::same_as<Tr, typename Number::int_t>)
						return Number{ lVal << static_cast<int>(rVal) }; //< both sides are int
					else return Number{ static_cast<typename Number::int_t>(lVal) << static_cast<int>(rVal) };
				} }, l.value, r.value);
		}

		/// @returns	true when the value is zero; otherwise, false.
		friend constexpr bool operator!(Number const& n)
		{
			return n.is_zero();
		}
		/// @returns	true when equal; otherwise, false.
		friend constexpr bool operator==(const Number& l, const Number& r)
		{
			return std::visit([](auto&& lVal, auto&& rVal) { {
					using Tl = std::decay_t<decltype(lVal)>;
					using Tr = std::decay_t<decltype(rVal)>;

					if constexpr (std::same_as<Tl, Tr>)
						return lVal == rVal;
					else return static_cast<typename Number::real_t>(lVal) == static_cast<typename Number::real_t>(rVal);
				} }, l.value, r.value);
		}
		/// @returns	true when not equal; otherwise, false.
		friend constexpr bool operator!=(const Number& l, const Number& r)
		{
			return std::visit([](auto&& lVal, auto&& rVal) { {
					using Tl = std::decay_t<decltype(lVal)>;
					using Tr = std::decay_t<decltype(rVal)>;

					if constexpr (std::same_as<Tl, Tr>)
						return lVal != rVal;
					else return static_cast<typename Number::real_t>(lVal) != static_cast<typename Number::real_t>(rVal);
				} }, l.value, r.value);
		}
		/// @returns	true when a is less than b; otherwise, false.
		friend constexpr bool operator<(Number const& a, Number const& b)
		{
			return std::visit([](auto&& a, auto&& b) { {
					using A = std::decay_t<decltype(a)>;
					using B = std::decay_t<decltype(b)>;

					if constexpr (std::same_as<A, B>)
						return a < b;
					else return static_cast<typename Number::real_t>(a) < static_cast<typename Number::real_t>(b);
				} }, a.value, b.value);
		}
		/// @returns	true when a is less than or equal to b; otherwise, false.
		friend constexpr bool operator<=(Number const& a, Number const& b)
		{
			return std::visit([](auto&& a, auto&& b) { {
					using A = std::decay_t<decltype(a)>;
					using B = std::decay_t<decltype(b)>;

					if constexpr (std::same_as<A, B>)
						return a <= b;
					else return static_cast<typename Number::real_t>(a) <= static_cast<typename Number::real_t>(b);
				} }, a.value, b.value);
		}
		/// @returns	true when a is greater than b; otherwise, false.
		friend constexpr bool operator>(Number const& a, Number const& b)
		{
			return std::visit([](auto&& a, auto&& b) { {
					using A = std::decay_t<decltype(a)>;
					using B = std::decay_t<decltype(b)>;

					if constexpr (std::same_as<A, B>)
						return a > b;
					else return static_cast<typename Number::real_t>(a) > static_cast<typename Number::real_t>(b);
				} }, a.value, b.value);
		}
		/// @returns	true when a is greater than or equal to b; otherwise, false.
		friend constexpr bool operator>=(Number const& a, Number const& b)
		{
			return std::visit([](auto&& a, auto&& b) { {
					using A = std::decay_t<decltype(a)>;
					using B = std::decay_t<decltype(b)>;

					if constexpr (std::same_as<A, B>)
						return a >= b;
					else return static_cast<typename Number::real_t>(a) >= static_cast<typename Number::real_t>(b);
				} }, a.value, b.value);
		}

		friend std::istream& operator>>(std::istream& is, Number& n)
		{
			std::string s;
			is >> s;
			if (s.find('.') != std::string::npos)
				n.value = int_t{ s };
			else n.value = real_t{ s };
			return is;
		}
		friend std::ostream& operator<<(std::ostream& os, const Number& n)
		{
			std::visit([&](auto&& value) { os << $fwd(value); }, n.value);
			return os;
		}

	#pragma endregion Operators
	};

#pragma endregion Number

#pragma region Trigonometric Functions

	/// @brief	Compute cosine
	WINCONSTEXPR Number cos(Number const& x)
	{
		return Number{ boost::multiprecision::cos(x.cast_to<Number::real_t>()) };
	}
	/// @brief	Compute sine
	WINCONSTEXPR Number sin(Number const& x)
	{
		return Number{ boost::multiprecision::sin(x.cast_to<Number::real_t>()) };
	}
	/// @brief	Compute tangent
	WINCONSTEXPR Number tan(Number const& x)
	{
		return Number{ boost::multiprecision::tan(x.cast_to<Number::real_t>()) };
	}
	/// @brief	Compute arc cosine
	WINCONSTEXPR Number acos(Number const& x)
	{
		return Number{ boost::multiprecision::acos(x.cast_to<Number::real_t>()) };
	}
	/// @brief	Compute arc sine
	WINCONSTEXPR Number asin(Number const& x)
	{
		return Number{ boost::multiprecision::asin(x.cast_to<Number::real_t>()) };
	}
	/// @brief	Compute arc tangent
	WINCONSTEXPR Number atan(Number const& x)
	{
		return Number{ boost::multiprecision::atan(x.cast_to<Number::real_t>()) };
	}
	/// @brief	Compute arc tangent with two parameters
	WINCONSTEXPR Number atan2(Number const& y, Number const& x)
	{
		return Number{ boost::multiprecision::atan2(y.cast_to<Number::real_t>(), x.cast_to<Number::real_t>()) };
	}

#pragma endregion Trigonometric Functions

#pragma region Hyperbolic Functions

	/// @brief	Compute hyperbolic cosine
	WINCONSTEXPR Number cosh(Number const& x)
	{
		return Number{ boost::multiprecision::cosh(x.cast_to<Number::real_t>()) };
	}
	/// @brief	Compute hyperbolic sine
	WINCONSTEXPR Number sinh(Number const& x)
	{
		return Number{ boost::multiprecision::sinh(x.cast_to<Number::real_t>()) };
	}
	/// @brief	Compute hyperbolic tangent
	WINCONSTEXPR Number tanh(Number const& x)
	{
		return Number{ boost::multiprecision::tanh(x.cast_to<Number::real_t>()) };
	}
	/// @brief	Compute area hyperbolic cosine
	WINCONSTEXPR Number acosh(Number const& x)
	{
		return Number{ boost::multiprecision::acosh(x.cast_to<Number::real_t>()) };
	}
	/// @brief	Compute area hyperbolic sine
	WINCONSTEXPR Number asinh(Number const& x)
	{
		return Number{ boost::multiprecision::asinh(x.cast_to<Number::real_t>()) };
	}
	/// @brief	Compute area hyperbolic tangent
	WINCONSTEXPR Number atanh(Number const& x)
	{
		return Number{ boost::multiprecision::atanh(x.cast_to<Number::real_t>()) };
	}

#pragma endregion Hyperbolic Functions

#pragma region Exponential and Logarithmic Functions

	/// @brief	Compute exponential function
	WINCONSTEXPR Number exp(Number const& x)
	{
		return Number{ boost::multiprecision::exp(x.cast_to<Number::real_t>()) };
	}
	/// @brief	Generate value from significand and exponent
	WINCONSTEXPR Number ldexp(Number const& sig, Number const& exp)
	{
		if (!exp.has_integral_value())
			throw make_exception("The second operand to ldexp must be an integral!");
		else if (!exp.can_fit_value<int64_t>())
			throw make_exception("The second operand to ldexp must be smaller than ", std::numeric_limits<int64_t>::max());

		return Number{ boost::multiprecision::ldexp(sig.cast_to<Number::real_t>(), exp.cast_to<int64_t>()) };
	}
	/// @brief	Compute natural logarithm
	WINCONSTEXPR Number log(Number const& x)
	{
		return Number{ boost::multiprecision::log(x.cast_to<Number::real_t>()) };
	}
	/// @brief	Compute common logarithm
	WINCONSTEXPR Number log10(Number const& x)
	{
		return Number{ boost::multiprecision::log10(x.cast_to<Number::real_t>()) };
	}
	/// @brief	Compute binary exponential function
	WINCONSTEXPR Number exp2(Number const& x)
	{
		return Number{ boost::multiprecision::exp2(x.cast_to<Number::real_t>()) };
	}
	/// @brief	Compute exponential minus one
	WINCONSTEXPR Number expm1(Number const& x)
	{
		return Number{ boost::multiprecision::expm1(x.cast_to<Number::real_t>()) };
	}
	/// @brief	Integer binary logarithm
	WINCONSTEXPR Number ilogb(Number const& x)
	{
		return Number{ boost::multiprecision::ilogb(x.cast_to<Number::real_t>()) };
	}
	/// @brief	Compute logarithm plus one
	WINCONSTEXPR Number log1p(Number const& x)
	{
		return Number{ boost::multiprecision::log1p(x.cast_to<Number::real_t>()) };
	}
	/// @brief	Compute binary logarithm
	WINCONSTEXPR Number log2(Number const& x)
	{
		return Number{ boost::multiprecision::log2(x.cast_to<Number::real_t>()) };
	}
	/// @brief	Compute floating-point base logarithm
	WINCONSTEXPR Number logb(Number const& x)
	{
		return Number{ boost::multiprecision::logb(x.cast_to<Number::real_t>()) };
	}
	/// @brief	Scale significand using floating-point base exponent
	WINCONSTEXPR Number scalbn(Number const& x, Number const& y)
	{
		if (!y.has_integral_value())
			throw make_exception("The second operand to scalbn/scalbln must be an integral!");
		else if (!y.can_fit_value<int64_t>())
			throw make_exception("The second operand to scalbn/scalbln must be smaller than ", std::numeric_limits<int64_t>::max());

		return Number{ boost::multiprecision::scalbln(x.cast_to<Number::real_t>(), y.cast_to<int64_t>()) };
	}

#pragma endregion Exponential and Logarithmic Functions

#pragma region Power Functions

	/// @brief	Raise to power
	WINCONSTEXPR Number pow(Number const& base, Number const& exp)
	{
		if (base.has_integral_value() && exp.has_integral_value()) {
			if (exp.can_fit_value<int64_t>()) {
				return Number{ boost::multiprecision::pow(base.cast_to<Number::int_t>(), exp.cast_to<int64_t>()) };
			}
			else if (exp.can_fit_value<uint64_t>()) {
				return Number{ boost::multiprecision::pow(base.cast_to<Number::int_t>(), exp.cast_to<uint64_t>()) };
			}
		}

		return Number{ boost::multiprecision::pow(base.cast_to<Number::real_t>(), exp.cast_to<Number::real_t>()) };
	}
	/// @brief	Compute square root
	WINCONSTEXPR Number sqrt(Number const& n)
	{
		return boost::multiprecision::sqrt(n.cast_to<Number::real_t>());
	}
	/// @brief	Compute cubic root
	WINCONSTEXPR Number cbrt(Number const& n)
	{
		return boost::multiprecision::cbrt(n.cast_to<Number::real_t>());
	}
	/// @brief	Compute cubic root
	WINCONSTEXPR Number hypot(Number const& x, Number const& y)
	{
		return boost::multiprecision::hypot(x.cast_to<Number::real_t>(), y.cast_to<Number::real_t>());
	}

#pragma endregion Power Functions

#pragma region Error & Gamma Functions

	/// @brief	Compute error function
	WINCONSTEXPR Number erf(Number const& n)
	{
		return Number{ boost::multiprecision::erf(n.cast_to<Number::real_t>()) };
	}
	/// @brief	Compute complementary error function
	WINCONSTEXPR Number erfc(Number const& n)
	{
		return Number{ boost::multiprecision::erfc(n.cast_to<Number::real_t>()) };
	}
	/// @brief	Compute gamma function
	WINCONSTEXPR Number tgamma(Number const& n)
	{
		return Number{ boost::multiprecision::tgamma(n.cast_to<Number::real_t>()) };
	}
	/// @brief	Compute log-gamma function
	WINCONSTEXPR Number lgamma(Number const& n)
	{
		return Number{ boost::multiprecision::lgamma(n.cast_to<Number::real_t>()) };
	}

#pragma endregion Error & Gamma Functions

#pragma region Rounding & Remainder Functions

	/// @brief	Round up value
	WINCONSTEXPR Number ceil(Number const& n)
	{
		return n.is_integer() ? n : Number{ boost::multiprecision::ceil(n.cast_to<Number::real_t>()) };
	}
	/// @brief	Round down value
	WINCONSTEXPR Number floor(Number const& n)
	{
		return n.is_integer() ? n : Number{ boost::multiprecision::floor(n.cast_to<Number::real_t>()) };
	}
	/// @brief	Compute remainder of division
	WINCONSTEXPR Number fmod(Number const& numer, Number const& denom)
	{
		return numer % denom;
	}
	/// @brief	Truncate value
	WINCONSTEXPR Number trunc(Number const& n)
	{
		return n.is_integer() ? n : Number{ boost::multiprecision::trunc(n.cast_to<Number::real_t>()) };
	}
	/// @brief	Round to nearest
	WINCONSTEXPR Number round(Number const& n)
	{
		return n.is_integer() ? n : Number{ boost::multiprecision::round(n.cast_to<Number::real_t>()) };
	}
	/// @brief	Round to nearby integral value
	WINCONSTEXPR Number nearbyint(Number const& n)
	{
		return n.is_integer() ? n : Number{ boost::multiprecision::nearbyint(n.cast_to<Number::real_t>()) };
	}
	/// @brief	Compute remainder of division
	WINCONSTEXPR Number remainder(Number const& numer, Number const& denom)
	{
		return numer % denom;
	}

#pragma endregion Rounding & Remainder Functions

#pragma region Floating-point Manipulation Functions

	/// @brief	Copy sign
	WINCONSTEXPR Number copysign(Number const& number, Number const& sign)
	{
		if (number.has_integral_value() && sign.has_integral_value())
			return Number{ boost::multiprecision::copysign(number.cast_to<Number::int_t>(), sign.cast_to<Number::int_t>()) };
		return Number{ boost::multiprecision::copysign(number.cast_to<Number::real_t>(), sign.cast_to<Number::real_t>()) };
	}
	/// @brief	Next representable value
	WINCONSTEXPR Number nextafter(Number const& a, Number const& b)
	{
		return Number{ boost::multiprecision::nextafter(a.cast_to<Number::real_t>(), b.cast_to<Number::real_t>()) };
	}
	/// @brief	Next representable value toward precise value
	WINCONSTEXPR Number nexttoward(Number const& a, Number const& b)
	{
		return Number{ boost::multiprecision::nexttoward(a.cast_to<Number::real_t>(), b.cast_to<Number::real_t>()) };
	}

#pragma endregion Floating-point Manipulation Functions

#pragma region Minimum, Maximum, & Difference Functions

	/// @brief	Positive difference
	WINCONSTEXPR Number fdim(Number const& a, Number const& b)
	{
		return Number{ boost::multiprecision::fdim(a.cast_to<Number::real_t>(), b.cast_to<Number::real_t>()) };
	}
	/// @brief	Get larger value
	WINCONSTEXPR Number max(Number const& a, Number const& b)
	{
		if (a.has_integral_value() && b.has_integral_value())
			return Number{ boost::multiprecision::max(a.cast_to<Number::int_t>(), b.cast_to<Number::int_t>()) };
		else return Number{ boost::multiprecision::fmax(a.cast_to<Number::real_t>(), b.cast_to<Number::real_t>()) };
	}
	/// @brief	Get smaller value
	WINCONSTEXPR Number min(Number const& a, Number const& b)
	{
		if (a.has_integral_value() && b.has_integral_value())
			return Number{ boost::multiprecision::min(a.cast_to<Number::int_t>(), b.cast_to<Number::int_t>()) };
		else return Number{ boost::multiprecision::fmin(a.cast_to<Number::real_t>(), b.cast_to<Number::real_t>()) };
	}

#pragma endregion Minimum, Maximum, & Difference Functions

#pragma region Other

	/// @brief	Get Absolute Value
	WINCONSTEXPR Number abs(Number const& n)
	{
		if (n.has_integral_value())
			return Number{ boost::multiprecision::abs(n.cast_to<Number::int_t>()) };
		else return Number{ boost::multiprecision::abs(n.cast_to<Number::real_t>()) };
	}
	/// @brief	Multiply-add
	WINCONSTEXPR Number fma(Number const& x, Number const& y, Number const& z)
	{
		return Number{ boost::multiprecision::fma(x.cast_to<Number::real_t>(), y.cast_to<Number::real_t>(), z.cast_to<Number::real_t>()) };
	}

#pragma endregion Other
}
