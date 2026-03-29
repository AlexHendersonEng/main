#pragma once

#include <typeinfo>

template <typename T>
class Signal {
 public:
  // Constructors
  Signal(T value) : _value(value) {};
  Signal() : _value{} {};

  // Destructor
  ~Signal() = default;

  // Type
  const std::type_info& type() const { return typeid(T); }

  // Getter
  T get() const { return _value; }

  // Setter
  void set(T value) { _value = value; }

 private:
  // Signal value
  T _value;
};
