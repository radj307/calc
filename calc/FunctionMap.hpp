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
	class basic_operator {
	public:
		using operation_t = std::function<Number(std::vector<Number> const&)>;
	private:

		size_t operand_count;
		operation_t operation;

	public:
		basic_operator() {}

		/**
		 * @brief
		 * @param operands
		 * @return
		 */
		Number invoke(std::vector<Number> const& operands) const
		{
			return operation(operands);
		}
		/**
		 * @brief				Attempts to invoke the operation while catching and discarding any exceptions that occur.
		 * @param operands    -	The parameters of the operation function.
		 * @param result	  -	A Number reference to set to the result of the operation.
		 * @returns				true when successful; otherwise, false.
		 */
		bool try_invoke(std::vector<Number> const& operands, Number& result)
		{
			try {
				result = invoke(operands);
				return true;
			} catch (...) {
				result = NAN;
				return false;
			}
		}
	};

	class generic_function {
		void* func{ nullptr };
		std::type_info func_type;
		std::type_info return_type;
		std::vector<std::type_info> arg_types;

	public:
		template<typename Returns, typename... Args>
		constexpr generic_function(const std::function<Returns(Args...)>& func) :
			func{ reinterpret_cast<void*>(func.target()) },
			func_type{ func.target_type() },
			return_type{ typeid(Returns) },
			arg_types{ typeid(Args)... }
		{}

		std::any invoke(std::vector<std::any> const& argv) const noexcept(false)
		{

		}
	};

	template<class Func, typename... Ts>
	auto call(const Func& function, std::tuple<Ts...>&& args)
	{
		return std::apply(function, std::forward<std::tuple<Ts...>>(args));
	}

	// Unwrap function to extract elements from std::vector<std::any>
	template<typename... Args, std::size_t... Indices>
	auto unwrap(const std::vector<std::any>& vec, std::index_sequence<Indices...> = std::index_sequence_for<Args...>())
	{
		return std::make_tuple(std::any_cast<Args>(vec[Indices])...);
	}

	template<typename... Args, class Func>
	std::function<std::any(std::vector<std::any>)> wrap_call(Func func)
	{
		return std::move([=](std::vector<std::any> const& args) -> std::any {
			return call(func, unwrap<Args...>(args, std::index_sequence_for<Args...>()));
		});
	}

	struct FunctionMap {


		std::map<std::string, std::function<std::any(std::vector<std::any> const&)>> map{
			// using std::make_pair() here makes compilation much faster than using uniform initialization
			std::make_pair("pow", wrap_call<long double, long double>(std::powl)),
		};



		template<typename... Ts>
		std::optional<std::any> invoke(std::string const& functionName, Ts&&... args) const
		{
			if (const auto& fn{ map.find(functionName) }; fn != map.end()) {
				return fn->second({ std::forward<Ts>(args)... });
			}
			return std::nullopt;
		}

	};
}
