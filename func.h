#pragma once

#include <experimental/coroutine>
#include <functional>
#include <tuple>

template <typename Ret, typename... Args>
class func {
 public:
  func() noexcept : h(nullptr) {}

  template <typename F>
  func(F&& f) : func(create(std::forward<F>(f))) {}

  func(func const&) = delete;
  func(func&& other) noexcept : func() {
    using std::swap;
    swap(h, other.h);
  }

  func& operator=(func const&) = delete;
  func& operator=(func&& other) noexcept {
    h = nullptr;
    using std::swap;
    swap(h, other.h);
    return *this;
  }

  ~func() {
    if (h) h.destroy();
  }

  Ret operator()(Args... args) {
    if (!h) throw std::bad_function_call();
    auto args_tuple = std::tuple<Args...>{args...};
    h.promise().arguments = &args_tuple;
    h.resume();
    return std::move(h.promise().ret_value);
  }

  struct promise_type;

 private:
  struct promise_type {
    auto get_return_object() { return func(*this); }
    auto initial_suspend() { return std::experimental::suspend_always{}; }
    template <typename F>
    auto yield_value(F&& f) {
      ret_value = std::apply(f, std::move(*arguments));
      return std::experimental::suspend_always{};
    }
    void return_void() {}
    void unhandled_exception() { throw; }
    auto final_suspend() { return std::experimental::suspend_always{}; }

    std::tuple<Args...>* arguments;
    Ret ret_value;
  };

  using handle_type = std::experimental::coroutine_handle<promise_type>;
  handle_type h;

  explicit func(promise_type& p) : h(handle_type::from_promise(p)) {}

  template <typename F>
  static func create(F f) {
    for (;;) {
      co_yield f;
    }
  }
};
