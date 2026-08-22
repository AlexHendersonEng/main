#include "auto_grad/auto_grad.hpp"

#include <ranges>

namespace core::auto_grad {

AutoGrad AutoGrad::operator+(const AutoGrad& other) const {
  AutoGrad result(value_ + other.value_);

  // Store references to the original operands
  result.children_.push_back(
      std::shared_ptr<AutoGrad>(const_cast<AutoGrad*>(this), [](void*) {}));
  result.children_.push_back(
      std::shared_ptr<AutoGrad>(const_cast<AutoGrad*>(&other), [](void*) {}));

  // Define gradient functions for each child
  // For addition: d(a+b)/da = 1, d(a+b)/db = 1
  result.grad_funcs_.emplace_back([](double grad) { return grad; });
  result.grad_funcs_.emplace_back([](double grad) { return grad; });

  return result;
}

AutoGrad AutoGrad::operator-(const AutoGrad& other) const {
  AutoGrad result(value_ - other.value_);

  // Store references to the original operands
  result.children_.push_back(
      std::shared_ptr<AutoGrad>(const_cast<AutoGrad*>(this), [](void*) {}));
  result.children_.push_back(
      std::shared_ptr<AutoGrad>(const_cast<AutoGrad*>(&other), [](void*) {}));

  // Define gradient functions for each child
  // For subtraction: d(a-b)/da = 1, d(a-b)/db = -1
  result.grad_funcs_.emplace_back([](double grad) { return grad; });
  result.grad_funcs_.emplace_back([](double grad) { return -grad; });

  return result;
}

AutoGrad AutoGrad::operator*(const AutoGrad& other) const {
  AutoGrad result(value_ * other.value_);

  // Store references to the original operands
  result.children_.push_back(
      std::shared_ptr<AutoGrad>(const_cast<AutoGrad*>(this), [](void*) {}));
  result.children_.push_back(
      std::shared_ptr<AutoGrad>(const_cast<AutoGrad*>(&other), [](void*) {}));

  // Define gradient functions for each child
  // For multiplication: d(a*b)/da = b, d(a*b)/db = a
  // Capture by value to avoid dangling references
  const double other_value = other.value_;
  const double this_value = value_;
  result.grad_funcs_.emplace_back(
      [other_value](const double grad) { return grad * other_value; });
  result.grad_funcs_.emplace_back(
      [this_value](const double grad) { return grad * this_value; });

  return result;
}

AutoGrad AutoGrad::operator/(const AutoGrad& other) const {
  AutoGrad result(value_ / other.value_);

  // Store references to the original operands
  result.children_.push_back(
      std::shared_ptr<AutoGrad>(const_cast<AutoGrad*>(this), [](void*) {}));
  result.children_.push_back(
      std::shared_ptr<AutoGrad>(const_cast<AutoGrad*>(&other), [](void*) {}));

  // For division: d(a/b)/da = 1/b, d(a/b)/db = -a/(b*b)
  const double other_value = other.value_;
  const double this_value = value_;
  result.grad_funcs_.emplace_back(
      [other_value](double grad) { return grad / other_value; });
  result.grad_funcs_.emplace_back([this_value, other_value](const double grad) {
    return -grad * this_value / (other_value * other_value);
  });

  return result;
}

AutoGrad AutoGrad::operator-() const {
  AutoGrad result(-value_);

  // Unary negation depends only on this node
  result.children_.push_back(
      std::shared_ptr<AutoGrad>(const_cast<AutoGrad*>(this), [](void*) {}));
  result.grad_funcs_.emplace_back([](const double grad) { return -grad; });

  return result;
}

AutoGrad AutoGrad::operator+(const double scalar) const {
  AutoGrad result(value_ + scalar);

  // Scalar is treated as constant; only this node receives gradient
  result.children_.push_back(
      std::shared_ptr<AutoGrad>(const_cast<AutoGrad*>(this), [](void*) {}));
  result.grad_funcs_.emplace_back([](const double grad) { return grad; });

  return result;
}

AutoGrad AutoGrad::operator-(const double scalar) const {
  AutoGrad result(value_ - scalar);

  // Scalar is treated as constant; only this node receives gradient
  result.children_.push_back(
      std::shared_ptr<AutoGrad>(const_cast<AutoGrad*>(this), [](void*) {}));
  result.grad_funcs_.emplace_back([](const double grad) { return grad; });

  return result;
}

AutoGrad AutoGrad::operator*(const double scalar) const {
  AutoGrad result(value_ * scalar);

  // Scalar is treated as constant; only this node receives gradient
  result.children_.push_back(
      std::shared_ptr<AutoGrad>(const_cast<AutoGrad*>(this), [](void*) {}));
  result.grad_funcs_.emplace_back(
      [scalar](const double grad) { return grad * scalar; });

  return result;
}

AutoGrad AutoGrad::operator/(const double scalar) const {
  AutoGrad result(value_ / scalar);

  // Scalar is treated as constant; only this node receives gradient
  result.children_.push_back(
      std::shared_ptr<AutoGrad>(const_cast<AutoGrad*>(this), [](void*) {}));
  result.grad_funcs_.emplace_back(
      [scalar](const double grad) { return grad / scalar; });

  return result;
}

AutoGrad operator+(const double scalar, const AutoGrad& auto_grad) {
  return auto_grad + scalar;
}

AutoGrad operator-(const double scalar, const AutoGrad& auto_grad) {
  AutoGrad result(scalar - auto_grad.value_);

  result.children_.push_back(std::shared_ptr<AutoGrad>(
      const_cast<AutoGrad*>(&auto_grad), [](void*) {}));
  result.grad_funcs_.emplace_back([](double grad) { return -grad; });

  return result;
}

AutoGrad operator*(const double scalar, const AutoGrad& auto_grad) {
  return auto_grad * scalar;
}

AutoGrad operator/(double scalar, const AutoGrad& auto_grad) {
  AutoGrad result(scalar / auto_grad.value_);

  const double child_value = auto_grad.value_;
  result.children_.push_back(std::shared_ptr<AutoGrad>(
      const_cast<AutoGrad*>(&auto_grad), [](void*) {}));
  result.grad_funcs_.emplace_back([scalar, child_value](double grad) {
    return -grad * scalar / (child_value * child_value);
  });

  return result;
}

void AutoGrad::Backward() {
  // Initialize gradient for the root node
  grad_ = 1.0;

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
    for (size_t i = 0; i < node->children_.size(); ++i) {
      AutoGrad* child = node->children_[i].get();
      const double child_grad = node->grad_funcs_[i](node->grad_);
      child->grad_ += child_grad;
      queue.push_back(child);
    }
  }
}

void AutoGrad::ZeroGrad() {
  // Set gradient of current node to zero
  grad_ = 0.0;

  // Traverse computational graph and zero gradients
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

    // Zero gradient for children
    for (auto& child_ptr : node->children_) {
      AutoGrad* child = child_ptr.get();
      child->grad_ = 0.0;
      queue.push_back(child);
    }
  }
}

std::ostream& operator<<(std::ostream& os, const AutoGrad& auto_grad) {
  os << "Value: " << auto_grad.value_ << ", Grad: " << auto_grad.grad_
     << std::endl;
  return os;
}

}  // namespace core::auto_grad
