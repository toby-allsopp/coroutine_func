// coroutinefunc.cpp : Defines the entry point for the console application.
//

#include "func.h"

#include <iostream>

int main() {
  {
    func<int> f = []() { return 7; };
    std::cout << f() << std::endl;
    std::cout << f() << std::endl;
  }
  {
    func<int, std::unique_ptr<int> const&> f = [](auto const& p) { return *p; };
    std::cout << f(std::make_unique<int>(7)) << std::endl;
    std::cout << f(std::make_unique<int>(-1)) << std::endl;
  }
  {
      func<std::unique_ptr<int>, std::unique_ptr<int> const&> f = [](auto const& p) { return std::make_unique<int>(*p); };
      std::cout << *f(std::make_unique<int>(7)) << std::endl;
      std::cout << *f(std::make_unique<int>(-1)) << std::endl;
  }
  {
    func<int, int> f = [](int i) -> int { throw i; };
    try {
      std::cout << f(7) << std::endl;
    } catch (int i) {
      std::cout << "Caught " << i << std::endl;
    }
    try {
      std::cout << f(-1) << std::endl;
    } catch (int i) {
      std::cout << "Caught " << i << std::endl;
    }
  }
}
