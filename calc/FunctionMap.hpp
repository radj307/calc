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
			param_count{ sizeof...(FArgs) },
			int_func{ internal::wrap_function<IReturns, IArgs...>(intFunc) },
			real_func{ internal::wrap_function<FReturns, FArgs...>(realFunc) },
			prefer_int_func{ preferIntFunc }
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
			if (operands.size() != param_count)
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
	/*class fallback_operator : basic_operator {
		using basic_operator::operation_t;
		using voperator = std::variant<singletype_operator, dualtype_operator>;

		std::vector<voperator> operators;

	public:
		fallback_operator(std::initializer_list<voperator> operators) : operators{ operators } {}

	};*/

	struct FunctionMap {
		// See https://cplusplus.com/reference/cmath/
		std::map<std::string, basic_operator*> map{
			/// Trigonometric Functions
			std::make_pair("cos", (basic_operator*)new singletype_operator{ cosl }),
			std::make_pair("sin", (basic_operator*)new singletype_operator{ sinl }),
			std::make_pair("tan", (basic_operator*)new singletype_operator{ tanl }),
			std::make_pair("acos", (basic_operator*)new singletype_operator{ acosl }),
			std::make_pair("asin", (basic_operator*)new singletype_operator{ asinl }),
			std::make_pair("atan", (basic_operator*)new singletype_operator{ atanl }),
			std::make_pair("atan2", (basic_operator*)new singletype_operator{ atan2l }),
			/// Hyperbolic Functions
			std::make_pair("cosh", (basic_operator*)new singletype_operator{ coshl }),
			std::make_pair("sinh", (basic_operator*)new singletype_operator{ sinhl }),
			std::make_pair("tanh", (basic_operator*)new singletype_operator{ tanhl }),
			std::make_pair("acosh", (basic_operator*)new singletype_operator{ acoshl }),
			std::make_pair("asinh", (basic_operator*)new singletype_operator{ asinhl }),
			std::make_pair("atanh", (basic_operator*)new singletype_operator{ atanhl }),
			/// Exponential and Logarithmic Functions
			std::make_pair("exp", (basic_operator*)new singletype_operator{ expl }),
			//std::make_pair("frexp", (basic_operator*)new singletype_operator{ frexpl }), //< uses pointers, must be adapted
			std::make_pair("ldexp", (basic_operator*)new singletype_operator{ ldexpl }),
			std::make_pair("log", (basic_operator*)new singletype_operator{ logl }),
			std::make_pair("log10", (basic_operator*)new singletype_operator{ log10l }),
			//std::make_pair("modf", (basic_operator*)new singletype_operator{ modfl }), //< uses pointers, must be adapted
			std::make_pair("exp2", (basic_operator*)new singletype_operator{ exp2l }),
			std::make_pair("expm1", (basic_operator*)new singletype_operator{ expm1l }),
			std::make_pair("ilogb", (basic_operator*)new singletype_operator{ ilogbl }),
			std::make_pair("log1p", (basic_operator*)new singletype_operator{ log1pl }),
			std::make_pair("log2", (basic_operator*)new singletype_operator{ log2l }),
			std::make_pair("logb", (basic_operator*)new singletype_operator{ logbl }),
			std::make_pair("scalbn", (basic_operator*)new singletype_operator{ scalbnl }),
			std::make_pair("scalbln", (basic_operator*)new singletype_operator{ scalblnl }),
			/// Power Functions
			std::make_pair("pow", (basic_operator*)new dualtype_operator{ ipow, powl }),
			std::make_pair("sqrt", (basic_operator*)new singletype_operator{ sqrtl }),
			std::make_pair("cbrt", (basic_operator*)new singletype_operator{ cbrtl }),
			std::make_pair("hypot", (basic_operator*)new singletype_operator{ hypotl }),
			/// Error & Gamma Functions
			std::make_pair("erf", (basic_operator*)new singletype_operator{ erfl }),
			std::make_pair("erfc", (basic_operator*)new singletype_operator{ erfcl }),
			std::make_pair("tgamma", (basic_operator*)new singletype_operator{ tgammal }),
			std::make_pair("lgamma", (basic_operator*)new singletype_operator{ lgammal }),
			/// Rounding & Remainder Functions
			std::make_pair("ceil", (basic_operator*)new singletype_operator{ ceill }),
			std::make_pair("floor", (basic_operator*)new singletype_operator{ floorl }),
			std::make_pair("fmod", (basic_operator*)new singletype_operator{ fmodl }),
			std::make_pair("trunc", (basic_operator*)new singletype_operator{ truncl }),
			std::make_pair("round", (basic_operator*)new singletype_operator{ roundl }),
			std::make_pair("remainder", (basic_operator*)new singletype_operator{ remainderl }),
			/// Floating-point Manipulation Functions
			std::make_pair("copysign", (basic_operator*)new singletype_operator{ copysignl }),
			//std::make_pair("nan", (basic_operator*)new singletype_operator{ nanl }), //< requires a char*
			std::make_pair("nextafter", (basic_operator*)new singletype_operator{ nextafterl }),
			std::make_pair("nexttoward", (basic_operator*)new singletype_operator{ nexttowardl }),
			/// Minimum, Maximum, & Difference Functions
			//std::make_pair("dim", (basic_operator*)new singletype_operator{ fdiml }), //< weird
			std::make_pair("max", (basic_operator*)new dualtype_operator{ imax, fmaxl }),
			std::make_pair("min", (basic_operator*)new dualtype_operator{ imin, fminl }),
			/// Other
			std::make_pair("abs", (basic_operator*)new dualtype_operator{ llabs, fabsl }),
			std::make_pair("fma", (basic_operator*)new singletype_operator{ fmal }),
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
