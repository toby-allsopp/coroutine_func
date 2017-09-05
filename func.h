#pragma once

#include <experimental/coroutine>
#include <tuple>

template <typename Ret, typename... Args>
class func {
 public:
  template <typename F>
  func(F&& f) : func(create(std::forward<F>(f))) {}

  func(func const&) = delete;
  func(func&& other) noexcept : h(other.h) { other.h = nullptr; }

  func& operator=(func const&) = delete;
  func& operator=(func&& other) noexcept {
    h = other.h;
    other.h = nullptr;
    return *this;
  }

  ~func() {
    if (h) h.destroy();
  }

  // template <typename... ActualArgs>
  Ret operator()(Args... args) {
    auto args_tuple = std::tuple<Args...>{args...};
    h.promise().arguments = &args_tuple;
    h.resume();
    return h.promise().ret_value;
  }

  struct promise_type;

 private:
  struct get_args {
    promise_type* p;
    bool await_ready() { return true; }
    void await_suspend(std::experimental::coroutine_handle<>) {}
    decltype(auto) await_resume() { return std::move(*p->arguments); }
  };

  struct promise_type {
    auto get_return_object() { return func(*this); }
    auto initial_suspend() { return std::experimental::suspend_always{}; }
    auto await_transform(get_args) { return get_args{this}; }
    auto yield_value(Ret ret) {
      ret_value = std::move(ret);
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
      co_yield std::apply(f, co_await get_args{});
    }
  }
};