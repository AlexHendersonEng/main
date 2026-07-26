#ifndef CORE_AUTO_GRAD_HPP_
#define CORE_AUTO_GRAD_HPP_

#include <functional>
#include <iostream>
#include <memory>
#include <vector>

namespace core::auto_grad {

class AutoGrad : public std::enable_shared_from_this<AutoGrad> {
 public:
  // Constructor
  explicit AutoGrad(const double value) : value_(value), grad_(0.0) {}

  // Getters
  [[nodiscard]] double Value() const { return value_; }
  [[nodiscard]] double Grad() const { return grad_; }

  // Compute gradients through back propagation
  void Backward();

  // Zero gradients
  void ZeroGrad();

  // Operator overloading
  AutoGrad operator+(const AutoGrad& other) const;
  AutoGrad operator-(const AutoGrad& other) const;
  AutoGrad operator*(const AutoGrad& other) const;

  // Public access to gradient for testing/modification
  double& MutableGrad() { return grad_; }

  friend std::ostream& operator<<(std::ostream& os, const AutoGrad& auto_grad);

 private:
  // Value and gradient
  double value_;
  mutable double grad_;

  // Computational graph tracking
  mutable std::vector<std::shared_ptr<AutoGrad>> children_;
  mutable std::vector<std::function<double(double)>> grad_funcs_;
};

std::ostream& operator<<(std::ostream& os, const AutoGrad& auto_grad);

}  // namespace core::auto_grad

#endif  // CORE_AUTO_GRAD_HPP_
