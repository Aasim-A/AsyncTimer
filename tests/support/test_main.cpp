#include "Test.h"

#include <iostream>

namespace test {
std::vector<Case> &cases() { static std::vector<Case> all; return all; }
std::string &context() { static std::string value; return value; }
}

int main(int argc, char **argv) {
  unsigned int passed = 0;
  unsigned int failed = 0;
  for (const auto &testCase : test::cases()) {
    if (argc > 1 && std::string(testCase.name).find(argv[1]) == std::string::npos)
      continue;
    fake::reset();
    std::srand(1);
    test::context().clear();
    try {
      testCase.run();
      ++passed;
      std::cout << "PASS " << testCase.name << std::endl;
    } catch (const std::exception &error) {
      ++failed;
      std::cerr << "FAIL " << testCase.name << ": " << error.what() << std::endl;
    } catch (...) {
      ++failed;
      std::cerr << "FAIL " << testCase.name << ": unknown exception" << std::endl;
    }
  }
  std::cout << passed << " passed, " << failed << " failed" << std::endl;
  return failed != 0 || passed == 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
