#ifndef CORE_AUTO_GRAD_HPP_
#define CORE_AUTO_GRAD_HPP_

/**
 * @file auto_grad.hpp
 * @brief A lightweight scalar automatic differentiation type.
 */

#include <functional>
#include <iostream>
#include <memory>
#include <vector>

namespace core::auto_grad {

/**
 * Scalar value node used to build a computational graph and compute
 * first-order derivatives via reverse-mode automatic differentiation.
 *
 * <p>Each operation creates a new node and records parent references and
 * local derivative callbacks. Calling @c Backward propagates gradients from
 * the current node to its dependencies.
 */
class AutoGrad : public std::enable_shared_from_this<AutoGrad> {
 public:
  /**
   * Constructs a scalar node with the provided value and zero gradient.
   *
   * @param value initial scalar value
   */
  explicit AutoGrad(const double value) : value_(value), grad_(0.0) {}

  /**
   * Gets the scalar value stored in this node.
   *
   * @return current scalar value
   */
  [[nodiscard]] double Value() const { return value_; }

  /**
   * Gets the accumulated gradient for this node.
   *
   * @return accumulated derivative value
   */
  [[nodiscard]] double Grad() const { return grad_; }

  /**
   * Performs reverse-mode back propagation from this node.
   *
   * <p>This treats the current node as the output and accumulates gradients
   * in all connected child nodes based on the recorded local derivatives.
   */
  void Backward();

  /**
   * Resets this node's gradient accumulator to zero.
   */
  void ZeroGrad();

  /**
   * Adds two scalar nodes and returns the resulting node.
   *
   * @param other right-hand operand
   * @return node representing {@code this + other}
   */
  AutoGrad operator+(const AutoGrad& other) const;

  /**
   * Subtracts one scalar node from another and returns the resulting node.
   *
   * @param other right-hand operand
   * @return node representing {@code this - other}
   */
  AutoGrad operator-(const AutoGrad& other) const;

  /**
   * Multiplies two scalar nodes and returns the resulting node.
   *
   * @param other right-hand operand
   * @return node representing {@code this * other}
   */
  AutoGrad operator*(const AutoGrad& other) const;

  /**
   * Provides mutable access to the gradient accumulator.
   *
   * <p>Intended primarily for tests or controlled manual gradient updates.
   *
   * @return reference to the stored gradient value
   */
  double& MutableGrad() { return grad_; }

  /**
   * Streams a readable representation of an AutoGrad node.
   *
   * @param os output stream
   * @param auto_grad node to print
   * @return the output stream
   */
  friend std::ostream& operator<<(std::ostream& os, const AutoGrad& auto_grad);

 private:
  // Value and gradient
  double value_;
  mutable double grad_;

  // Computational graph tracking
  mutable std::vector<std::shared_ptr<AutoGrad>> children_;
  mutable std::vector<std::function<double(double)>> grad_funcs_;
};

/**
 * Streams a readable representation of an AutoGrad node.
 *
 * @param os output stream
 * @param auto_grad node to print
 * @return the output stream
 */
std::ostream& operator<<(std::ostream& os, const AutoGrad& auto_grad);

}  // namespace core::auto_grad

#endif  // CORE_AUTO_GRAD_HPP_
