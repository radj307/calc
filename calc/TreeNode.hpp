#pragma once
#include <var.hpp>

#include <vector>
#include <optional>
#include <stack>
#include <queue>

namespace calc {
	/**
	 * @brief		A tree node object that can hold a value of type T.
	 * @tparam T  -	The type of value contained by each node in the tree.
	 */
	template<typename T>
	class TreeNode {
		using node_t = TreeNode<T>;

		using iterator = std::vector<node_t>::iterator;
		using const_iterator = std::vector<node_t>::const_iterator;

	public:
		std::vector<node_t> children;
		/// @brief	The value of this TreeNode instance.
		T value;

		/// @brief	Default constructor. Requires type T to be default-constructible.
		constexpr TreeNode() requires (std::constructible_from<T>) {}
		/// @brief	Creates a new TreeNode instance with the specified value.
		constexpr TreeNode(var::same_or_convertible<T> auto&& value) : value{ std::move(value) } {}
		/// @brief	Creates a new TreeNode instance with the specified value.
		constexpr TreeNode(var::same_or_convertible<T> auto const& value) : value{ value } {}
		/// @brief	Creates a new TreeNode instance with the specified value and children nodes.
		constexpr TreeNode(var::same_or_convertible<T> auto&& value, std::initializer_list<node_t> children) : value{ std::move(value) }, children{ children } {}
		/// @brief	Creates a new TreeNode instance with the specified value and children nodes.
		constexpr TreeNode(var::same_or_convertible<T> auto const& value, std::initializer_list<node_t> children) : value{ value }, children{ children } {}

	#pragma region begin/end
		/// @brief	Gets the beginning iterator for this node's children.
		[[nodiscard]] auto begin() const noexcept { return children.begin(); }
		/// @brief	Gets the beginning iterator for this node's children.
		[[nodiscard]] auto begin() noexcept { return children.begin(); }
		/// @brief	Gets the ending iterator for this node's children.
		[[nodiscard]] auto end() const noexcept { return children.end(); }
		/// @brief	Gets the ending iterator for this node's children.
		[[nodiscard]] auto end() noexcept { return children.end(); }
		/// @brief	Gets the reverse beginning iterator for this node's children.
		[[nodiscard]] auto rbegin() const noexcept { return children.rbegin(); }
		/// @brief	Gets the reverse beginning iterator for this node's children.
		[[nodiscard]] auto rbegin() noexcept { return children.rbegin(); }
		/// @brief	Gets the reverse ending iterator for this node's children.
		[[nodiscard]] auto rend() const noexcept { return children.rend(); }
		/// @brief	Gets the reverse ending iterator for this node's children.
		[[nodiscard]] auto rend() noexcept { return children.rend(); }
	#pragma endregion begin/end

		/// @brief	Gets the immediate child node that matches the specified predicate.
		[[nodiscard]] std::optional<node_t> getChild(const std::function<bool(node_t)>& predicate) noexcept
		{
			if (const auto& it{ std::find_if(children.begin(), children.end(), predicate) }; it != children.end()) {
				return *it;
			}
			return std::nullopt;
		}
		/// @brief	Gets the immediate child node that matches the specified predicate.
		[[nodiscard]] std::optional<node_t> getChild(const std::function<bool(node_t)>& predicate) const noexcept
		{
			if (const auto& it{ std::find_if(children.begin(), children.end(), predicate) }; it != children.end()) {
				return *it;
			}
			return std::nullopt;
		}

		/// @brief	Performs a depth-first search of the subtree for a node that matches the specified predicate.
		[[nodiscard]] std::optional<node_t> getChildDepthFirst(const std::function<bool(node_t)>& predicate)
		{
			std::stack<node_t> stack;
			stack.push(*this);

			while (!stack.empty()) {
				auto current{ stack.top() };
				stack.pop();

				if (predicate(current))
					return current;

				for (auto& it : current.children) {
					stack.push(it);
				}
			}
			return std::nullopt;
		}
		/// @brief	Performs a breadth-first search of the subtree for a node that matches the specified predicate.
		[[nodiscard]] std::optional<node_t> getChildBreadthFirst(const std::function<bool(node_t)>& predicate)
		{
			std::queue<node_t> queue;
			queue.push(*this);

			while (!queue.empty()) {
				auto current{ queue.front() };
				queue.pop();

				if (predicate(current))
					return current;

				for (auto& it : current.children) {
					queue.push(it);
				}
			}
			return std::nullopt;
		}

		/// @brief	Adds the specified child node.
		void addChild(node_t const& child)
		{
			children.emplace_back(child);
		}
		/// @brief	Adds the specified child node.
		void addChild(node_t&& child)
		{
			children.emplace_back(std::move(child));
		}

		/// @brief	Adds the specified child nodes.
		void addChildren(std::initializer_list<node_t> children)
		{
			this->children.insert(this->children.end(), children.begin(), children.end());
		}

		/// @brief	Removes the specified child node.
		bool removeChild(node_t const& child)
		{
			if (auto it{ std::find(children.begin(), children.end(), child) }; it != children.end()) {
				children.erase(it);
				return true;
			}
			return false;
		}

		/// @brief	Determines whether the specified nodes are the same instance.
		[[nodiscard]] friend bool operator==(node_t const& l, node_t const& r)
		{
			return &l == &r;
		}
		/// @brief	Determines whether the specified nodes are different instances.
		[[nodiscard]] friend bool operator!=(node_t const& l, node_t const& r)
		{
			return &l != &r;
		}
	};

	/// @brief	Prints the value of the specified tree node to a stream.
	template<var::streamable T>
	std::ostream& operator<<(std::ostream& os, TreeNode<T> const& n)
	{
		return os << n.value;
	}
}
