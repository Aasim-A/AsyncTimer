#include "Test.h"

#include <climits>

TEST(timeoutAcrossRollover) {
  AsyncTimer timer;
  unsigned int calls = 0;
  fake::setTime(ULONG_MAX - 9);
  const auto id = timer.setTimeout([&]() { ++calls; }, 20);
  CHECK(timer.getRemaining(id) == 20);
  fake::setTime(ULONG_MAX);
  timer.handle();
  CHECK(calls == 0);
  CHECK(timer.getRemaining(id) == 11);
  fake::setTime(0);
  timer.handle();
  CHECK(calls == 0);
  CHECK(timer.getRemaining(id) == 10);
  fake::setTime(9);
  timer.handle();
  CHECK(calls == 0);
  CHECK(timer.getRemaining(id) == 1);
  fake::setTime(10);
  CHECK(timer.getRemaining(id) == 0);
  timer.handle();
  timer.handle();
  CHECK(calls == 1);
  CHECK(timer.getRemaining(id) == 0);
}

TEST(intervalAcrossRollover) {
  AsyncTimer timer;
  unsigned int calls = 0;
  fake::setTime(ULONG_MAX - 14);
  timer.setInterval([&]() { ++calls; }, 10);
  fake::setTime(ULONG_MAX - 4);
  timer.handle();
  CHECK(calls == 1);
  fake::setTime(4);
  timer.handle();
  CHECK(calls == 1);
  fake::setTime(5);
  timer.handle();
  CHECK(calls == 2);
  fake::setTime(15);
  timer.handle();
  CHECK(calls == 3);
}

static void delayedInterval(unsigned long start) {
  AsyncTimer timer;
  unsigned int calls = 0;
  fake::setTime(start);
  const auto id = timer.setInterval([&]() { ++calls; }, 20);
  fake::setTime(start + 5);
  timer.delay(id, 15);
  timer.delay(id, 5);
  CHECK(timer.getRemaining(id) == 35);
  timer.handle();
  CHECK(calls == 0);
  fake::setTime(start + 39);
  timer.handle();
  CHECK(calls == 0);
  CHECK(timer.getRemaining(id) == 1);
  fake::setTime(start + 40);
  timer.handle();
  CHECK(calls == 1);
  CHECK(timer.getRemaining(id) == 20);
  fake::setTime(start + 60);
  timer.handle();
  CHECK(calls == 2);
}

TEST(resetAndChangeDelayAcrossRollover) {
  AsyncTimer timer;
  unsigned int calls = 0;
  fake::setTime(ULONG_MAX - 9);
  const auto id = timer.setTimeout([&]() { ++calls; }, 20);
  timer.delay(id, 30);
  fake::setTime(ULONG_MAX - 4);
  timer.reset(id);
  CHECK(timer.getRemaining(id) == 20);
  timer.changeDelay(id, 10);
  CHECK(timer.getRemaining(id) == 10);
  fake::setTime(4);
  timer.handle();
  CHECK(calls == 0);
  fake::setTime(5);
  timer.handle();
  CHECK(calls == 1);
}

TEST(overdueAndZeroDelay) {
  AsyncTimer timer;
  unsigned int calls = 0;
  fake::setTime(ULONG_MAX - 2);
  const auto id = timer.setTimeout([&]() { ++calls; }, 1);
  fake::setTime(3);
  CHECK(timer.getRemaining(id) == 0);
  timer.handle();
  CHECK(calls == 1);
  timer.setTimeout([&]() { ++calls; }, 0);
  timer.handle();
  CHECK(calls == 2);
}

TEST(delayFromIntervalCallback) {
  AsyncTimer timer;
  unsigned int calls = 0;
  fake::setTime(ULONG_MAX - 14);
  unsigned short id = 0;
  id = timer.setInterval([&]() {
    ++calls;
    timer.delay(id, 5);
  }, 10);
  fake::setTime(ULONG_MAX - 4);
  timer.handle();
  CHECK(calls == 1);
  fake::setTime(5);
  timer.handle();
  CHECK(calls == 1);
  fake::setTime(10);
  timer.handle();
  CHECK(calls == 2);
}

TEST(longDelayAcrossRollover) {
  AsyncTimer timer;
  unsigned int calls = 0;
  const unsigned long duration = ULONG_MAX / 2 + 10;
  const unsigned long start = ULONG_MAX - 9;
  fake::setTime(start);
  const auto id = timer.setTimeout([&]() { ++calls; }, duration);
  CHECK(timer.getRemaining(id) == duration);
  fake::setTime(start + duration - 1);
  timer.handle();
  CHECK(calls == 0);
  CHECK(timer.getRemaining(id) == 1);
  fake::setTime(start + duration);
  timer.handle();
  CHECK(calls == 1);
}

TEST(cancelAndReuseDelayedTimer) {
  AsyncTimer timer(1);
  unsigned int calls = 0;
  fake::setTime(ULONG_MAX - 4);
  auto id = timer.setTimeout([&]() { ++calls; }, 10);
  timer.delay(id, 100);
  timer.cancel(id);
  id = timer.setTimeout([&]() { ++calls; }, 10);
  CHECK(id != 0);
  CHECK(timer.getRemaining(id) == 10);
  fake::setTime(5);
  timer.handle();
  CHECK(calls == 1);
}

TEST(delayedIntervalWithoutRollover) { delayedInterval(100); }
TEST(delayedIntervalAcrossRollover) { delayedInterval(ULONG_MAX - 9); }

TEST(maximum_unsigned_delay) {
  AsyncTimer timer;
  int calls = 0;
  fake::setTime(10);
  const auto id = timer.setTimeout([&]() { ++calls; }, ULONG_MAX);
  CHECK(timer.getRemaining(id) == ULONG_MAX);
  fake::advance(ULONG_MAX - 1);
  timer.handle();
  CHECK(calls == 0);
  CHECK(timer.getRemaining(id) == 1);
  fake::advance(1);
  timer.handle();
  CHECK(calls == 1);
}

TEST(interval_survives_multiple_clock_rollovers) {
  AsyncTimer timer;
  int calls = 0;
  const unsigned long period = ULONG_MAX / 2;
  timer.setInterval([&]() { ++calls; }, period);
  for (int expected = 1; expected <= 8; ++expected) {
    fake::advance(period);
    timer.handle();
    CHECK(calls == expected);
  }
}

TEST(timeout_due_exactly_at_zero_after_rollover) {
  AsyncTimer timer;
  int calls = 0;
  fake::setTime(ULONG_MAX - 9);
  const auto id = timer.setTimeout([&]() { ++calls; }, 10);
  fake::setTime(ULONG_MAX);
  timer.handle();
  CHECK(calls == 0);
  CHECK(timer.getRemaining(id) == 1);
  fake::setTime(0);
  timer.handle();
  CHECK(calls == 1);
}
