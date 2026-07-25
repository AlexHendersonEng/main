#include "auto_grad/auto_grad.hpp"

namespace core::auto_grad {

AutoGrad AutoGrad::operator+(AutoGrad& other) {
  AutoGrad result(this->value + other.value);

  // Store references to the original operands
  result.children.push_back(
      std::shared_ptr<AutoGrad>(const_cast<AutoGrad*>(this), [](void*) {}));
  result.children.push_back(
      std::shared_ptr<AutoGrad>(const_cast<AutoGrad*>(&other), [](void*) {}));

  // Define gradient functions for each child
  // For addition: d(a+b)/da = 1, d(a+b)/db = 1
  result.grad_funcs.emplace_back([](const double grad) { return grad; });
  result.grad_funcs.emplace_back([](const double grad) { return grad; });

  return result;
}

void AutoGrad::backward() {
  // Initialize gradient for the root node
  this->grad = 1.0;

  // Traverse computational graph and propagate gradients
  std::vector<AutoGrad*> visited;
  std::vector<AutoGrad*> queue;
  queue.push_back(this);

  while (!queue.empty()) {
    AutoGrad* node = queue.back();
    queue.pop_back();

    // Skip if already visited
    if (std::ranges::find(visited, node) != visited.end()) {
      continue;
    }
    visited.push_back(node);

    // Propagate gradient to children
    for (size_t i = 0; i < node->children.size(); ++i) {
      AutoGrad* child = node->children[i].get();
      const double child_grad = node->grad_funcs[i](node->grad);
      child->grad += child_grad;
      queue.push_back(child);
    }
  }
}

std::ostream& operator<<(std::ostream& os, const AutoGrad& auto_grad) {
  os << "Value: " << auto_grad.value << ", Grad: " << auto_grad.grad
     << std::endl;
  return os;
}

}  // namespace core::auto_grad
