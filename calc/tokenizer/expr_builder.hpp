#pragma once
#include "token.hpp"
#include "expr_node.hpp"

namespace calc::expr::tkn {


	class expr_builder {
		using const_iterator = std::vector<primitive>::const_iterator;
		using iterator_range = std::pair<const_iterator, const_iterator>;

	protected:
		std::vector<primitive> primitives;

		const_iterator findEnd(const_iterator const& begin, PrimitiveTokenType const& open, PrimitiveTokenType const& close) const
		{
			size_t depth{ 0 };

			const auto it_end{ primitives.end() };
			for (auto it{ begin }; it != it_end; ++it) {
				if (const auto& type{ it->type }; type == open) {
					++depth;
				}
				else if (type == close) {
					--depth;

					if (depth == 0) return it;
				}
			}

			return it_end;
		}

	public:
		expr_builder(std::vector<primitive> const& primitives) : primitives{ primitives } {}
		expr_builder(const_iterator const& begin, const_iterator const& end) : primitives{ begin, end } {}

		expr_node& build(expr_node& root) const
		{
			for (auto it{ primitives.begin() }, it_end{ primitives.end() };
				 it != it_end;
				 ++it) {
				switch (it->type) {
				case PrimitiveTokenType::ExpressionOpen: {
					const auto close{ findEnd(it, PrimitiveTokenType::ExpressionOpen, PrimitiveTokenType::ExpressionClose) };
					// TODO: check if close iterator is valid

					auto node{ expr_builder{ it + 1, close }.build(ComplexTokenType::SubExpression) }; //< RECURSE
					root.addChild(node);

					it = close;
					break;
				}
				case PrimitiveTokenType::FunctionName: {
					const auto end{ findEnd(it + 1, PrimitiveTokenType::FunctionParamsOpen, PrimitiveTokenType::FunctionParamsClose) };
					// TODO: check if end iterator is valid

					auto node{ expr_builder{ it + 2, end }.build(combine_tokens(ComplexTokenType::Function, it, end + 1)) }; //< RECURSE
					// insert the function name token as the first child
					node.children.insert(node.children.begin(), *it);
					root.addChild(node);

					it = end;
					break;
				}
				default:
					// add a primitive child token
					root.addChild({ *it });
					break;
				}
			}

			return root;
		}
		expr_node build(complex const& rootToken) const
		{
			expr_node root{ rootToken };
			return build(root);
		}
		expr_node build(ComplexTokenType const& rootTokenType) const
		{
			return build(combine_tokens(rootTokenType, primitives));
		}
		expr_node build() const { return build(ComplexTokenType::SubExpression); }
	};
}
