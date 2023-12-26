#pragma once
// libcalc
#include <Number.hpp>		//< for calc::Number
#include <intmath.hpp>		//< for integral math functions

// 307lib
#include <var.hpp>			//< for concepts

// stl
#include <cmath>			//< for math functions
#include <cstdint>			//< for std::uint8_t (for clarity)
#include <map>				//< for std::map
#include <string>			//< for std::string
#include <functional>		//< for std::function

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
		template<typename Returns, typename... Args>
		constexpr auto wrap_function(Returns(*func)(Args...))
		{
			static_assert(var::numeric<Returns>, "wrap_function() can only be used for functions that return a numeric type!");
			static_assert((var::numeric<Args> && ...), "wrap_function() can only be used for functions that have numeric parameter types only!");

			return [=](std::vector<Number> const& args) -> Number {
				return Number{ std::apply(func, unwrap_args<Number, Args...>(args, std::index_sequence_for<Args...>())) };
			};
		}
	}

	/// @brief	An operator that wraps a single function.
	class basic_operator {
	protected:
		using operation_t = std::function<Number(std::vector<Number> const&)>;

		size_t func_param_count;
		operation_t func;

	public:
		/**
		 * @brief				Creates a new basic_operator instance for the specified function.
		 * @tparam Returns	  -	The return type of the function.
		 * @tparam ...Args	  -	The argument type(s) of the function.
		 * @param func		  -	The function to use when performing an operation.
		 */
		template<typename Returns, typename... Args>
		basic_operator(Returns(*func)(Args...)) : func_param_count{ sizeof...(Args) }, func{ internal::wrap_function<Returns, Args...>(func) } {}

		/// @brief	Gets the number of parameters required by the wrapped function.
		[[nodiscard]] size_t getRequiredFuncParamsCount() const noexcept
		{
			return func_param_count;
		}

		/**
		 * @brief				Invokes the wrapped function with the specified operands. Does not catch exceptions.
		 * @param operands	  -	A vector of operands to pass to the wrapped function.
		 * @returns				The result of the operation.
		 */
		virtual Number invoke(std::vector<Number> const& operands) const noexcept(false)
		{
			if (operands.size() != func_param_count)
				throw make_exception("Function operator called with the incorrect number of arguments! Expected: ", func_param_count, ", Actual: ", operands.size());
			return func(operands);
		}
		/**
		 * @brief				Attempts to invoke the wrapped function. Exceptions are caught and discarded.
		 *						 Note that passing the incorrect number of arguments will cause the operation to always return false.
		 * @param operands	  -	A vector of operands to pass to the wrapped function.
		 * @param result	  -	A Number reference to set to the result of the operation. If the operation fails, the result is NAN.
		 * @returns				true when successful; otherwise, false.
		 */
		virtual bool try_invoke(std::vector<Number> const& operands, Number& result) const noexcept
		{
			if (operands.size() != func_param_count)
				return false;
			try {
				result = invoke(operands);
				return true;
			} catch (...) {
				result = NAN;
				return false;
			}
		}

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
	/// @brief	An operator that wraps a pair of functions, one for integrals and another for floating-points.
	class dualtype_operator : basic_operator {
		using basic_operator::operation_t;

		size_t real_func_param_count;
		operation_t real_func;
		bool prefer_int_func;

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
			basic_operator(intFunc),
			real_func_param_count{ sizeof...(FArgs) },
			real_func{ internal::wrap_function<FReturns, FArgs...>(realFunc) },
			prefer_int_func{ preferIntFunc }
		{
			static_assert((sizeof...(IArgs) == sizeof...(FArgs)), "dualtype_operator requires both functions to use the same number of parameters!");
		}

	#pragma region invoke
		Number invokef(std::vector<Number> const& operands) const noexcept(false)
		{
			if (operands.size() != func_param_count)
				throw make_exception("Function operator called with the incorrect number of arguments! Expected: ", func_param_count, ", Actual: ", operands.size());

			return real_func(operands);
		}
		Number invokei(std::vector<Number> const& operands) const noexcept(false)
		{
			if (operands.size() != func_param_count)
				throw make_exception("Function operator called with the incorrect number of arguments! Expected: ", func_param_count, ", Actual: ", operands.size());

			return func(operands);
		}
		Number invoke(std::vector<Number> const& operands, bool preferIntFunc) const noexcept(false)
		{
			if (!preferIntFunc || operands.empty() || operands.front().is_real())
				return invokef(operands);
			return invokei(operands);
		}
		/// @brief	Invokes the appropriate variant of the wrapped function for the specified operands, preferring to use the integral variant when possible.
		Number invoke(std::vector<Number> const& operands) const noexcept(false) override
		{
			return invoke(operands, prefer_int_func);
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
			if (operands.size() != func_param_count)
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
			if (operands.size() != func_param_count)
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
		 *						 the specified operands and preference. Exceptions are caught and discarded.
		 *						Note that passing the incorrect number of operands
		 *						 will cause the operation to always return false.
		 * @param operands	  -	A vector of operands to pass to the wrapped function.
		 * @param result	  -	A Number reference to set to the result of the operation.
		 *						 If the operation fails, the result is always NAN.
		 * @returns				true when successful; otherwise, false.
		 */
		bool try_invoke(std::vector<Number> const& operands, Number& result, bool preferIntFunc) const noexcept
		{
			if (operands.size() != func_param_count)
				return false;
			if (!preferIntFunc || operands.empty() || operands.front().is_real())
				return try_invokef(operands, result);
			return try_invokei(operands, result);
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
			return try_invoke(operands, result, prefer_int_func);
		}
	#pragma endregion try_invoke
	};

	struct FunctionMap {
		// See https://cplusplus.com/reference/cmath/
		std::map<std::string, basic_operator*> map{
			// Trigonometric Functions
			std::make_pair("cos", new basic_operator{ cosl }),
			std::make_pair("sin", new basic_operator{ sinl }),
			std::make_pair("tan", new basic_operator{ tanl }),
			std::make_pair("acos", new basic_operator{ acosl }),
			std::make_pair("asin", new basic_operator{ asinl }),
			std::make_pair("atan", new basic_operator{ atanl }),
			std::make_pair("atan2", new basic_operator{ atan2l }),
			// Hyperbolic Functions
			std::make_pair("cosh", new basic_operator{ coshl }),
			std::make_pair("sinh", new basic_operator{ sinhl }),
			std::make_pair("tanh", new basic_operator{ tanhl }),
			std::make_pair("acosh", new basic_operator{ acoshl }),
			std::make_pair("asinh", new basic_operator{ asinhl }),
			std::make_pair("atanh", new basic_operator{ atanhl }),
			// Exponential and Logarithmic Functions
			std::make_pair("exp", new basic_operator{ expl }),
			std::make_pair("frexp", new basic_operator{ frexpl }),
			std::make_pair("ldexp", new basic_operator{ ldexpl }),
			std::make_pair("log", new basic_operator{ logl }),
			std::make_pair("log10", new basic_operator{ log10l }),
			std::make_pair("modf", new basic_operator{ modfl }),
			std::make_pair("exp2", new basic_operator{ exp2l }),
			std::make_pair("expm1", new basic_operator{ expm1l }),
			std::make_pair("ilogb", new basic_operator{ ilogbl }),
			std::make_pair("log1p", new basic_operator{ log1pl }),
			std::make_pair("log2", new basic_operator{ log2l }),
			std::make_pair("logb", new basic_operator{ logbl }),
			std::make_pair("scalbn", new basic_operator{ scalbnl }),
			std::make_pair("scalbln", new basic_operator{ scalblnl }),
			// Power Functions
			std::make_pair("pow", (basic_operator*)new dualtype_operator{ ipow, powl }),
			std::make_pair("sqrt", new basic_operator{ sqrtl }),
			std::make_pair("cbrt", new basic_operator{ cbrtl }),
			std::make_pair("hypot", new basic_operator{ hypotl }),
			// Error & Gamma Functions
			std::make_pair("erf", new basic_operator{ erfl }),
			std::make_pair("erfc", new basic_operator{ erfcl }),
			std::make_pair("tgamma", new basic_operator{ tgammal }),
			std::make_pair("lgamma", new basic_operator{ lgammal }),
			// Rounding & Remainder Functions
			std::make_pair("ceil", new basic_operator{ ceill }),
			std::make_pair("floor", new basic_operator{ floorl }),
			std::make_pair("fmod", new basic_operator{ fmodl }),
			std::make_pair("trunc", new basic_operator{ truncl }),
			std::make_pair("round", new basic_operator{ roundl }),
			// Other
			std::make_pair("abs", (basic_operator*)new dualtype_operator{ llabs, fabsl }),
			std::make_pair("fma", new basic_operator{ fmal }),
		};

		~FunctionMap()
		{
			// delete the pointers in the map
			for (auto& pr : map) {
				if (pr.second != nullptr) {
					delete pr.second;
					pr.second = nullptr;
				}
			}
		}

		/**
		 * @brief					Gets a pointer to the function operator with the specified name.
		 * @param functionName	  -	The name of the function to retrieve.
		 * @returns					A const operator pointer when successful; otherwise, nullptr.
		 */
		basic_operator const* const get(std::string const& functionName) const noexcept
		{
			if (const auto& it{ map.find(functionName) }; it != map.end())
				return it->second;
			return nullptr;
		}
	};
}
