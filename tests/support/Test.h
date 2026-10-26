#ifndef HOST_TEST_H
#define HOST_TEST_H

#include "AsyncTimer.h"
#include "FakeArduino.h"

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace test {
struct Case { const char *name; void (*run)(); };
std::vector<Case> &cases();
std::string &context();
struct Register {
  Register(const char *name, void (*run)()) { cases().push_back({name, run}); }
};
inline void fail(const char *expression, const char *file, int line) {
  std::ostringstream message;
  message << file << ':' << line << ": " << expression;
  if (!context().empty()) message << " (" << context() << ')';
  throw std::runtime_error(message.str());
}
}

#define TEST(name) \
  static void name(); \
  static test::Register register_##name(#name, name); \
  static void name()
// Unlike assert(), checks remain enabled in Release builds.
#define CHECK(expression) \
  do { if (!(expression)) test::fail(#expression, __FILE__, __LINE__); } while (false)

#endif
