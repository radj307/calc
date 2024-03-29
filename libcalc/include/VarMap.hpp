#pragma once
// libcalc
#include <Number.hpp>

// 307lib
#include <print_table.hpp>

// STL
#include <map>
#include <string>
#include <concepts>
#include <ostream>

namespace calc {
	/// @brief	Thin wrapper around a std::map that contains the variable table.
	struct VarMap {
		/// @brief	The numeric value of a variable.
		using value_t = Number;

		/// @brief	Stores variable name-value mappings.
		std::map<std::string, value_t> map;

		/// @brief	Creates a new instance with no variables.
		VarMap() {}
		/// @brief	Creates a new instance with the specified variables.
		VarMap(std::initializer_list<std::pair<std::string, value_t>> variables) : map{ variables.begin(), variables.end() } {}
		/// @brief	Creates a new instance with the variables in the specified range.
		template<std::input_or_output_iterator Iter> VarMap(Iter const& begin, Iter const& end) : map{ begin, end } {}

		/**
		 * @brief			Gets the specified variable's value.
		 * @param name	  -	The name of a variable.
		 * @returns			A reference to the value of the variable with the specified name.
		 */
		auto& operator[](std::string const& name)
		{
			return map[name];
		}

		/**
		 * @brief			Determines if the specified variable is defined.
		 * @param name	  -	The name of a variable.
		 * @returns			true when defined; otherwise, false.
		 */
		inline bool isDefined(std::string const& name) const
		{
			return map.contains(name);
		}

		/// @brief	Sets the specified variable to undefined.
		inline auto erase(std::string const& name)
		{
			return map.erase(name);
		}

		/// @brief	Pretty-prints the variable table.
		friend std::ostream& operator<<(std::ostream& os, VarMap const& vm)
		{
			return os << term::print_table(vm.map.begin(), vm.map.end(), {
				{ "Name", [](auto&& pr) { return pr.first; } },
				{ "Value", [](auto&& pr) { return str::stringify(pr.second); } },
										   });
		}
	};
}
