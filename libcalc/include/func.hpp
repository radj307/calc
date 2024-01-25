#pragma once
// libcalc
#include <Number.hpp>		//< for calc::Number

// 307lib
#include <var.hpp>
#include <make_exception.hpp>

// stl
#include <cmath>
#include <vector>
#include <functional>

namespace calc {
	namespace internal {
		/// @brief	Unwraps the specified vector into a tuple with the specified type(s) using static_cast.
		///			The vector must contain at least as many types as ...Ts.
		template<typename T, typename... Ts, std::size_t... Indicies>
		constexpr auto unwrap_args(std::vector<T> const& vec, std::index_sequence<Indicies...>)
		{
			return std::make_tuple(static_cast<Ts>(vec[Indicies])...);
		}
	}

	/// @brief	Function wrapper interface functor.
	struct base_func {
		virtual ~base_func() = default;

		virtual constexpr size_t getParamsCount() const noexcept = 0;
		virtual constexpr Number invoke(std::vector<Number> const&) const = 0;

		constexpr operator base_func* () noexcept { return this; }
		constexpr operator base_func const* () const noexcept { return this; }

		WINCONSTEXPR Number operator()(std::vector<Number> const& args)
		{
			return invoke(args);
		}
	};

	/// @brief	Function wrapper implementation. To instantiate with a lambda, wrap it with std::function{} to explicitly convert it
	template<typename Returns, typename... Args>
	class func : public base_func {
		using function_t = std::function<Returns(Args...)>;

		function_t function;

	public:
		func(function_t const& function) : function{ function } {}
		func(Returns(*function)(Args...)) : function{ function } {}

		constexpr size_t getParamsCount() const noexcept override
		{
			return sizeof...(Args);
		}

		constexpr Number invoke(std::vector<Number> const& operands) const override
		{
			if (operands.size() != sizeof...(Args))
				throw make_exception("Function called with the incorrect number of arguments! Expected: ", sizeof...(Args), ", Actual: ", operands.size());
			return static_cast<Number>(std::apply(function, internal::unwrap_args<Number, Args...>(operands, std::index_sequence_for<Args...>())));
		}
	};
}
