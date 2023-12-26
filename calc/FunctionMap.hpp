#pragma once
#include <Number.hpp>		//< for calc::Number

#include <cmath>			//< for math functions
#include <cstdint>			//< for std::uint8_t (for clarity)
#include <map>				//< for std::map
#include <string>			//< for std::string
#include <functional>		//< for std::function

#include <any>				//< for std::any

#include <typeinfo>

namespace calc {

	/// @brief	Unwraps the specified vector into a tuple with the specified type(s) using static_cast.
	///			The vector must contain at least as many types as ...Ts.
	template<typename T, typename... Ts, std::size_t... Indicies>
	constexpr auto unwrap_args(std::vector<T> const& vec, std::index_sequence<Indicies...>)
	{
		return std::make_tuple(static_cast<Ts>(vec[Indicies])...);
	}

	/**
	 * @brief				Creates a callable wrapper for the specified function that allows it to be called
	 *						 with a vector of Number values in place of its actual argument types. It also
	 *						 casts the function's return value to a Number.
	 * @tparam Returns	  -	The return type for the specified function.
	 * @tparam ...Args	  -	The argument type(s) for the specified function.
	 * @param func		  -	The function to create a wrapper for.
	 * @returns				A function wrapper that accepts a vector of Numbers and returns a Number.
	 */
	template<typename Returns, typename... Args>
	constexpr auto wrap_function(Returns(*func)(Args...))
	{
		static_assert(var::numeric<Returns>, "wrap_function() can only be used for functions that return a numeric type!");
		static_assert((var::numeric<Args> && ...), "wrap_function() can only be used for functions that have numeric parameter types only!");

		return [=](std::vector<Number> const& args) -> Number {
			return Number{ std::apply(func, unwrap_args<Number, Args...>(args, std::index_sequence_for<Args...>())) };
		};
	}


	class basic_operator {
	public:
		using operation_t = std::function<Number(std::vector<Number> const&)>;
	private:
		operation_t func;

	public:
		//basic_operator(operation_t const& func) : func{ func } {}
		template<typename Returns, typename... Args>
		basic_operator(Returns(*func)(Args...)) : func{ wrap_function<Returns, Args...>(func) } {}

		/**
		 * @brief
		 * @param operands
		 * @return
		 */
		Number invoke(std::vector<Number> const& operands) const noexcept(false)
		{
			return func(operands);
		}
		/**
		 * @brief				Attempts to invoke the operation while catching and discarding any exceptions that occur.
		 * @param operands    -	The parameters of the operation function.
		 * @param result	  -	A Number reference to set to the result of the operation. If the operation fails, the result is NAN.
		 * @returns				true when successful; otherwise, false.
		 */
		bool try_invoke(std::vector<Number> const& operands, Number& result) const noexcept
		{
			try {
				result = invoke(operands);
				return true;
			} catch (...) {
				result = NAN;
				return false;
			}
		}

		template<var::same_or_convertible<Number>... Ts>
		std::optional<Number> operator()(Ts const&... operands) const noexcept
		{
			Number result;
			if (try_invoke({ static_cast<Number>(operands)... }, result)) {
				return result;
			}
			return std::nullopt;
		}
	};
}
