#ifndef CORE_AUTO_GRAD_HPP_
#define CORE_AUTO_GRAD_HPP_

#include <functional>
#include <iostream>
#include <memory>
#include <vector>

namespace core::auto_grad {

class AutoGrad : public std::enable_shared_from_this<AutoGrad> {
 public:
  // Value and gradient
  double value;
  double grad;

  // Computational graph tracking
  std::vector<std::shared_ptr<AutoGrad>> children;
  std::vector<std::function<double(double)>> grad_funcs;

  // Constructor
  explicit AutoGrad(const double value) : value(value), grad(0.0) {}

  // Compute gradients through back propagation
  void backward();

  // Operator overloading
  AutoGrad operator+(AutoGrad& other);
};

std::ostream& operator<<(std::ostream& os, const AutoGrad& auto_grad);

}  // namespace core::auto_grad

#endif  // CORE_AUTO_GRAD_HPP_
