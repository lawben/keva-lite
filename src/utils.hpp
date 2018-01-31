#pragma once

namespace keva {

class Noncopyable {
 public:
  Noncopyable() = default;
  Noncopyable(Noncopyable&&) = default;
  Noncopyable& operator=(Noncopyable&&) = default;
  ~Noncopyable() = default;
  Noncopyable(const Noncopyable&) = delete;
  const Noncopyable& operator=(const Noncopyable&) = delete;
};

template <typename T>
inline void Assert(const T& value, const std::string& msg) {
  if (static_cast<bool>(value)) {
    return;
  }
  throw std::logic_error(msg);
}

}  // namespace keva