#pragma once
#include <stack>

/// @brief	Pops the top item off of the specified stack and returns it.
template<typename T>
constexpr T pop_off(std::stack<T>& stack)
{
	const auto top{ stack.top() };
	stack.pop();
	return top;
}
