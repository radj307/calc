#pragma once
// libcalc
#include <Number.hpp>		//< for calc::Number

// 307lib
#include <var.hpp>
#include <make_exception.hpp>

// stl
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

		/**
		 * @brief				Creates a callable wrapper for the specified function that allows it to be called
		 *						 with a vector of Number values in place of its actual argument types. It also
		 *						 casts the function's return value to a Number.
		 * @tparam Returns	  -	The return type for the specified function.
		 * @tparam ...Args	  -	The argument type(s) for the specified function.
		 * @param func		  -	The function to create a wrapper for.
		 * @returns				A function wrapper that accepts a vector of Numbers and returns a Number.
		 */
		template<var::numeric Returns, var::numeric... Args>
		constexpr auto wrap_function(Returns(*func)(Args...))
		{
			return [=](std::vector<Number> const& args) -> Number {
				return static_cast<Number>(std::apply(func, unwrap_args<Number, Args...>(args, std::index_sequence_for<Args...>())));
			};
		}
	}

	/// @brief	Operator abstract base class
	class basic_operator {
	protected:
		using operation_t = std::function<Number(std::vector<Number> const&)>;

	public:
		virtual ~basic_operator() = default;

		/// @brief	Gets the number of parameters required by the wrapped function.
		virtual [[nodiscard]] size_t getParamsCount() const noexcept = 0;

		/**
		 * @brief				Invokes the wrapped function with the specified operands. Does not catch exceptions.
		 * @param operands	  -	A vector of operands to pass to the wrapped function.
		 * @returns				The result of the operation.
		 */
		virtual Number invoke(std::vector<Number> const& operands) const noexcept(false) = 0;

		/**
		 * @brief				Attempts to invoke the wrapped function. Exceptions are caught and discarded.
		 *						 Note that passing the incorrect number of arguments will cause the operation to always return false.
		 * @param operands	  -	A vector of operands to pass to the wrapped function.
		 * @param result	  -	A Number reference to set to the result of the operation. If the operation fails, the result is NAN.
		 * @returns				true when successful; otherwise, false.
		 */
		virtual bool try_invoke(std::vector<Number> const& operands, Number& result) const noexcept = 0;

		/**
		 * @brief				Attempts to invoke the wrapped function. Exceptions are caught and discarded.
		 * @param operands    -	The parameters of the wrapped function.
		 * @returns				The result of the operation when successful; otherwise, std::nullopt.
		 */
		std::optional<Number> operator()(var::same_or_convertible<Number> auto const&... operands) const noexcept
		{
			if (Number result; try_invoke({ static_cast<Number>(operands)... }, result)) {
				return result;
			}
			return std::nullopt;
		}
	};

	/// @brief	An operator that wraps a single function.
	class singletype_operator : public basic_operator {

		size_t param_count;
		operation_t func;

	public:
		/**
		 * @brief				Creates a new basic_operator instance for the specified function.
		 * @tparam Returns	  -	The return type of the function.
		 * @tparam ...Args	  -	The argument type(s) of the function.
		 * @param func		  -	The function to use when performing an operation.
		 */
		template<typename Returns, typename... Args>
		singletype_operator(Returns(*func)(Args...)) : param_count{ sizeof...(Args) }, func{ internal::wrap_function<Returns, Args...>(func) } {}

		[[nodiscard]] size_t getParamsCount() const noexcept override
		{
			return param_count;
		}

		virtual Number invoke(std::vector<Number> const& operands) const noexcept(false)
		{
			if (operands.size() != param_count)
				throw make_exception("Function operator called with the incorrect number of arguments! Expected: ", param_count, ", Actual: ", operands.size());
			return func(operands);
		}
		virtual bool try_invoke(std::vector<Number> const& operands, Number& result) const noexcept override
		{
			if (operands.size() != param_count)
				return false;
			try {
				result = invoke(operands);
				return true;
			} catch (...) {
				result = NAN;
				return false;
			}
		}
	};
	/// @brief	An operator that wraps a pair of functions, one for integrals and another for floating-points.
	class dualtype_operator : public basic_operator {
		using basic_operator::operation_t;

		size_t param_count;
		operation_t int_func;
		operation_t real_func;

		bool canUseIntFunc(std::vector<Number> const& numbers) const noexcept
		{
			if (numbers.empty()) return true;

			for (const auto& n : numbers) {
				if (n.is_real()) return false;
			}
			return true;
		}

	public:
		/**
		 * @brief					Creates a new dualtype_operator instance for the specified function pair.
		 * @tparam IReturns		  -	The return type of the function that handles integral types.
		 * @tparam ...IArgs		  -	The argument type(s) of the function that handles integral types.
		 * @tparam FReturns		  -	The return type of the function that handles floating-point types.
		 * @tparam ...FArgs		  -	The argument type(s) of the function that handles floating-point types.
		 * @param intFunc		  -	The function that handles integral types.
		 * @param realFunc		  -	The function that handles floating-point types.
		 * @param preferIntFunc	  -	When true, prefers using the integral function when possible. Defaults to true.
		 */
		template<std::integral IReturns, std::floating_point FReturns, std::integral... IArgs, std::floating_point... FArgs>
		dualtype_operator(IReturns(*intFunc)(IArgs...), FReturns(*realFunc)(FArgs...), bool preferIntFunc = true) :
			param_count{ sizeof...(FArgs) },
			int_func{ internal::wrap_function<IReturns, IArgs...>(intFunc) },
			real_func{ internal::wrap_function<FReturns, FArgs...>(realFunc) }
		{
			static_assert((sizeof...(IArgs) == sizeof...(FArgs)), "dualtype_operator requires both functions to use the same number of parameters!");
		}

		[[nodiscard]] size_t getParamsCount() const noexcept override
		{
			return param_count;
		}

	#pragma region invoke
		Number invokef(std::vector<Number> const& operands) const noexcept(false)
		{
			if (operands.size() != param_count)
				throw make_exception("Function operator called with the incorrect number of arguments! Expected: ", param_count, ", Actual: ", operands.size());

			return real_func(operands);
		}
		Number invokei(std::vector<Number> const& operands) const noexcept(false)
		{
			if (operands.size() != param_count)
				throw make_exception("Function operator called with the incorrect number of arguments! Expected: ", param_count, ", Actual: ", operands.size());

			return int_func(operands);
		}
		/// @brief	Invokes the appropriate variant of the wrapped function for the specified operands, preferring to use the integral variant when possible.
		Number invoke(std::vector<Number> const& operands) const noexcept(false) override
		{
			if (canUseIntFunc(operands))
				return invokei(operands);
			return invokef(operands);
		}
	#pragma endregion invoke

	#pragma region try_invoke
		/**
		 * @brief				Attempts to invoke the floating-point variant of the wrapped function.
		 *						Exceptions are caught and discarded. Note that passing the incorrect
		 *						 number of operands will cause the operation to always return false.
		 * @param operands	  -	A vector of operands to pass to the wrapped function.
		 * @param result	  -	A Number reference to set to the result of the operation.
		 *						 If the operation fails, the result is always NAN.
		 * @returns				true when successful; otherwise, false.
		 */
		bool try_invokef(std::vector<Number> const& operands, Number& result) const noexcept
		{
			if (operands.size() != param_count)
				return false;
			try {
				result = invokef(operands);
				return true;
			} catch (...) {
				result = NAN;
				return false;
			}
		}
		/**
		 * @brief				Attempts to invoke the integral variant of the wrapped function.
		 *						Exceptions are caught and discarded. Note that passing the incorrect
		 *						 number of operands will cause the operation to always return false.
		 * @param operands	  -	A vector of operands to pass to the wrapped function.
		 * @param result	  -	A Number reference to set to the result of the operation.
		 *						 If the operation fails, the result is always NAN.
		 * @returns				true when successful; otherwise, false.
		 */
		bool try_invokei(std::vector<Number> const& operands, Number& result) const noexcept
		{
			if (operands.size() != param_count)
				return false;
			try {
				result = invokei(operands);
				return true;
			} catch (...) {
				result = NAN;
				return false;
			}
		}
		/**
		 * @brief				Attempts to invoke the appropriate variant of the wrapped function for
		 *						 the specified operands, preferring integral operations when possible.
		 *						Exceptions are caught and discarded. Note that passing the incorrect
		 *						 number of operands will cause the operation to always return false.
		 * @param operands	  -	A vector of operands to pass to the wrapped function.
		 * @param result	  -	A Number reference to set to the result of the operation.
		 *						 If the operation fails, the result is always NAN.
		 * @returns				true when successful; otherwise, false.
		 */
		bool try_invoke(std::vector<Number> const& operands, Number& result) const noexcept override
		{
			if (operands.size() != param_count)
				return false;
			try {
				result = invoke(operands);
				return true;
			} catch (...) {
				result = NAN;
				return false;
			}
		}
	#pragma endregion try_invoke
	};
}
