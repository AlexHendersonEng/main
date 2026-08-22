#ifndef CORE_BLOCK_SIM_PORT_HPP_
#define CORE_BLOCK_SIM_PORT_HPP_

namespace core::block_sim {

class PortBase {
 public:
  PortBase() = default;
  virtual ~PortBase() = default;
};

template <typename T>
class Port : public PortBase {
 public:
  Port() : value_{} {}

  T get() const { return value_; }
  void set(const T value) { value_ = value; }

 private:
  T value_;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_PORT_HPP_
