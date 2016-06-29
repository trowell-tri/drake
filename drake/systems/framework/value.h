#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>

#include "drake/drakeSystemFramework_export.h"

namespace drake {
namespace systems {

template <typename T>
class Value;

/// A fully type-erased container class.
///
/// Only Value<T> should inherit directly from AbstractValue. User-defined
/// classes that define additional type-erased features should inherit from
/// Value<T>.
class DRAKESYSTEMFRAMEWORK_EXPORT AbstractValue {
 public:
  AbstractValue() {}
  virtual ~AbstractValue() {}

  /// Returns a copy of this AbstractValue.
  virtual std::unique_ptr<AbstractValue> Clone() const = 0;

  /// Returns the value wrapped in this AbstractValue, which must be of
  /// exactly type T.  T cannot be a superclass, abstract or otherwise.
  /// In Debug builds, if the types don't match, std::bad_cast will be
  /// thrown.  In Release builds, this is not guaranteed.
  template <typename T>
  const T& GetValue() const {
    return DownCastOrMaybeThrow<T>()->get_value();
  }

  /// Returns the value wrapped in this AbstractValue, which must be of
  /// exactly type T.  T cannot be a superclass, abstract or otherwise.
  /// In Debug builds, if the types don't match, std::bad_cast will be
  /// thrown.  In Release builds, this is not guaranteed.
  /// Intentionally not const: holders of const references to the AbstractValue
  /// should not be able to mutate the value it contains.
  template <typename T>
  T& GetMutableValue() {
    return DownCastMutableOrMaybeThrow<T>()->get_mutable_value();
  }

  /// Sets the value wrapped in this AbstractValue, which must be of
  /// exactly type T.  T cannot be a superclass, abstract or otherwise.
  /// @param value_to_set The value to be copied into this AbstractValue.
  /// In Debug builds, if the types don't match, std::bad_cast will be
  /// thrown.  In Release builds, this is not guaranteed.
  template <typename T>
  void SetValue(const T& value_to_set) {
    DownCastMutableOrMaybeThrow<T>()->set_value(value_to_set);
  }

 private:
  /// Casts this class to a Value<T>*. In Debug builds, throws
  /// std::bad_cast if the cast fails.
  template <typename T>
  Value<T>* DownCastMutableOrMaybeThrow() {
    // We cast away const in this private non-const method so that we can reuse
    // DownCastOrMaybeThrow. This is equivalent to duplicating the logic of
    // DownCastOrMaybeThrow with a non-const target type.
    return const_cast<Value<T>*>(DownCastOrMaybeThrow<T>());
  }

  /// Casts this class to a const Value<T>*. In Debug builds, throws
  /// std::bad_cast if the cast fails.
  /// TODO(david-german-tri): Use static_cast in Release builds for speed.
  template <typename T>
  const Value<T>* DownCastOrMaybeThrow() const {
    const Value<T>* value = dynamic_cast<const Value<T>*>(this);
    if (value == nullptr) {
      throw std::bad_cast();
    }
    return value;
  }
};

/// A container class for an arbitrary type T. User-defined classes that
/// require additional type-erased features should subclass Value<T>, taking
/// care to define the copy constructors and override Clone().
/// @tparam T Must be copy-constructible, and therefore not abstract.
template <typename T>
class Value : public AbstractValue {
 public:
  explicit Value(const T& v) : value_(v) {}

  // Values are copyable but not moveable.
  Value(const Value<T>& other) = default;
  Value& operator=(const Value<T>& other) = default;
  Value(Value<T>&& other) = delete;
  Value& operator=(Value<T>&& other) = delete;

  ~Value() override {}

  std::unique_ptr<AbstractValue> Clone() const override {
    return std::make_unique<Value<T>>(*this);
  }

  /// Returns a const reference to the stored value.
  const T& get_value() const { return value_; }

  /// Returns a mutable reference to the stored value.
  T& get_mutable_value() { return value_; }

  /// Replaces the stored value with a new one.
  void set_value(const T& v) { value_ = v; }

 private:
  T value_;
};

}  // namespace systems
}  // namespace drake
