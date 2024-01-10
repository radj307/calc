#pragma once
#include <stack>

template<typename T>
constexpr T pop_off(std::stack<T>& stack)
{
	const auto top{ stack.top() };
	stack.pop();
	return top;
}
