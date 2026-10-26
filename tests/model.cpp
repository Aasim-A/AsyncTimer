#include "Test.h"

#include <algorithm>
#include <climits>
#include <cstdint>
#include <iterator>
#include <map>

namespace {
struct ExpectedTimer {
  unsigned int token;
  std::uint64_t deadline;
  unsigned long period;
  bool interval;
};

// Fixed algorithm and seeds make failures reproducible across standard libraries.
std::uint32_t randomValue(std::uint32_t &state) {
  state ^= state << 13;
  state ^= state >> 17;
  state ^= state << 5;
  return state;
}

void compareWithModel(std::uint32_t seed, unsigned short capacity,
                      unsigned long clockOrigin) {
  AsyncTimer timer(capacity);
  std::map<unsigned short, ExpectedTimer> expected;
  std::vector<unsigned int> actualCalls;
  std::uint64_t elapsed = 0;
  std::uint32_t random = seed;
  unsigned int nextToken = 0;
  fake::setTime(clockOrigin);

  for (int step = 0; step < 15000; ++step) {
    test::context() = "seed=" + std::to_string(seed) + " step=" + std::to_string(step);
    const auto operation = randomValue(random) % 16;
    const unsigned long duration = randomValue(random) % 101;
    auto selected = expected.begin();
    if (!expected.empty())
      std::advance(selected, randomValue(random) % expected.size());

    if (operation < 6) {
      const bool interval = operation % 2 == 0;
      const auto token = ++nextToken;
      auto callback = [&actualCalls, token]() { actualCalls.push_back(token); };
      const auto id = interval ? timer.setInterval(callback, duration)
                               : timer.setTimeout(callback, duration);
      if (expected.size() == capacity) {
        CHECK(id == 0);
      } else {
        CHECK(id != 0);
        CHECK(expected.count(id) == 0);
        expected.emplace(id, ExpectedTimer{token, elapsed + duration, duration, interval});
      }
    } else if (operation < 8) {
      std::vector<unsigned int> expectedCalls;
      for (auto it = expected.begin(); it != expected.end();) {
        if (it->second.deadline > elapsed) {
          ++it;
          continue;
        }
        expectedCalls.push_back(it->second.token);
        if (it->second.interval) {
          it->second.deadline = elapsed + it->second.period;
          ++it;
        } else {
          it = expected.erase(it);
        }
      }
      timer.handle();
      std::sort(expectedCalls.begin(), expectedCalls.end());
      std::sort(actualCalls.begin(), actualCalls.end());
      CHECK(actualCalls == expectedCalls);
      actualCalls.clear();
    } else if (operation < 10) {
      elapsed += duration;
      fake::setTime(clockOrigin + static_cast<unsigned long>(elapsed));
    } else if (operation == 10 && selected != expected.end()) {
      const auto id = selected->first;
      timer.cancel(id);
      expected.erase(selected);
      CHECK(timer.getRemaining(id) == 0);
    } else if (operation == 11 || operation == 12) {
      const bool includeIntervals = operation == 12;
      timer.cancelAll(includeIntervals);
      for (auto it = expected.begin(); it != expected.end();) {
        if (includeIntervals || !it->second.interval) it = expected.erase(it);
        else ++it;
      }
    } else if (selected != expected.end()) {
      auto &entry = selected->second;
      if (operation == 13) {
        timer.delay(selected->first, duration);
        entry.deadline += duration;
      } else if (operation == 14) {
        timer.changeDelay(selected->first, duration);
        entry.deadline = entry.deadline - entry.period + duration;
        entry.period = duration;
      } else if (operation == 15) {
        timer.reset(selected->first);
        entry.deadline = elapsed + entry.period;
      }
    }
    CHECK(actualCalls.empty()); // Public mutations must never invoke callbacks.
    for (const auto &pair : expected) {
      const auto remaining = pair.second.deadline > elapsed
                                 ? pair.second.deadline - elapsed : 0;
      CHECK(timer.getRemaining(pair.first) == remaining);
    }
    CHECK(timer.getRemaining(0) == 0);
  }
}
}

TEST(model_default_capacity) { compareWithModel(0x12345678, 10, 0); }
TEST(model_single_slot) { compareWithModel(0xabcdef01, 1, 100); }
TEST(model_empty_capacity) { compareWithModel(0x87654321, 0, 0); }
TEST(model_across_rollover) { compareWithModel(0x13579bdf, 10, ULONG_MAX - 500); }
TEST(model_larger_capacity_across_rollover) { compareWithModel(0x2468ace1, 37, ULONG_MAX - 500); }
