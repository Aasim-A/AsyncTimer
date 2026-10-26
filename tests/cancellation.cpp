#include "Test.h"

TEST(cancel_timeout_before_due) {
  AsyncTimer timer;
  int calls = 0;
  const auto id = timer.setTimeout([&]() { ++calls; }, 10);
  timer.cancel(id);
  fake::advance(20);
  timer.handle();
  CHECK(calls == 0);
  CHECK(timer.getRemaining(id) == 0);
}

TEST(cancel_overdue_timeout_before_handle) {
  AsyncTimer timer;
  int calls = 0;
  const auto id = timer.setTimeout([&]() { ++calls; }, 10);
  fake::advance(20);
  timer.cancel(id);
  timer.handle();
  CHECK(calls == 0);
}

TEST(cancel_interval_stops_future_calls) {
  AsyncTimer timer;
  int calls = 0;
  const auto id = timer.setInterval([&]() { ++calls; }, 10);
  fake::advance(10);
  timer.handle();
  CHECK(calls == 1);
  timer.cancel(id);
  fake::advance(100);
  timer.handle();
  CHECK(calls == 1);
}

TEST(cancel_is_idempotent_and_does_not_revive_timer) {
  AsyncTimer timer(1);
  int calls = 0;
  const auto id = timer.setTimeout([&]() { ++calls; }, 10);
  timer.cancel(id);
  timer.cancel(id);
  timer.reset(id);
  timer.delay(id, 5);
  timer.changeDelay(id, 0);
  fake::advance(100);
  timer.handle();
  CHECK(calls == 0);
  CHECK(timer.setTimeout([]() {}, 0) != 0);
  CHECK(timer.setTimeout([]() {}, 0) == 0);
}

TEST(cancel_all_on_empty_timer_is_safe_and_repeatable) {
  AsyncTimer timer(3);
  for (int i = 0; i < 4; ++i) {
    timer.cancelAll();
    timer.cancelAll(false);
  }
  for (int i = 0; i < 3; ++i) CHECK(timer.setTimeout([]() {}, 10) != 0);
  CHECK(timer.setTimeout([]() {}, 10) == 0);
}

TEST(cancel_all_on_partly_used_timer_restores_capacity) {
  AsyncTimer timer(4);
  int calls = 0;
  timer.setTimeout([&]() { ++calls; }, 10);
  timer.setInterval([&]() { ++calls; }, 10);
  timer.cancelAll();
  fake::advance(10);
  timer.handle();
  CHECK(calls == 0);
  for (int i = 0; i < 4; ++i) CHECK(timer.setTimeout([]() {}, 10) != 0);
  CHECK(timer.setTimeout([]() {}, 10) == 0);
}

TEST(cancel_all_on_full_timer_stops_everything) {
  AsyncTimer timer(2);
  int calls = 0;
  const auto timeout = timer.setTimeout([&]() { ++calls; }, 10);
  const auto interval = timer.setInterval([&]() { ++calls; }, 10);
  timer.cancelAll();
  CHECK(timer.getRemaining(timeout) == 0);
  CHECK(timer.getRemaining(interval) == 0);
  fake::advance(20);
  timer.handle();
  CHECK(calls == 0);
}

TEST(cancel_all_false_preserves_intervals_and_their_remaining_time) {
  AsyncTimer timer(4);
  int timeouts = 0, intervals = 0;
  timer.setTimeout([&]() { ++timeouts; }, 10);
  const auto interval = timer.setInterval([&]() { ++intervals; }, 20);
  fake::advance(5);
  timer.cancelAll(false);
  timer.cancelAll(false);
  CHECK(timer.getRemaining(interval) == 15);
  fake::advance(15);
  timer.handle();
  CHECK(timeouts == 0 && intervals == 1);
  for (int i = 0; i < 3; ++i) CHECK(timer.setTimeout([]() {}, 10) != 0);
  CHECK(timer.setTimeout([]() {}, 10) == 0);
}

TEST(cancel_all_false_with_only_intervals_is_a_noop) {
  AsyncTimer timer(2);
  int calls = 0;
  timer.setInterval([&]() { ++calls; }, 10);
  timer.setInterval([&]() { ++calls; }, 10);
  timer.cancelAll(false);
  CHECK(timer.setTimeout([]() {}, 0) == 0);
  fake::advance(10);
  timer.handle();
  CHECK(calls == 2);
}

TEST(cancel_all_after_expiration_and_individual_cancellation) {
  AsyncTimer timer(3);
  timer.setTimeout([]() {}, 0);
  const auto id = timer.setInterval([]() {}, 10);
  timer.handle();
  timer.cancel(id);
  timer.cancelAll();
  for (int i = 0; i < 3; ++i) CHECK(timer.setInterval([]() {}, 10) != 0);
  CHECK(timer.setInterval([]() {}, 10) == 0);
}

TEST(repeated_bulk_cancellation_and_refill) {
  AsyncTimer timer(5);
  for (int cycle = 0; cycle < 100; ++cycle) {
    for (int i = 0; i < 5; ++i) CHECK(timer.setTimeout([]() {}, 10) != 0);
    timer.cancelAll();
    timer.cancelAll();
    timer.handle();
  }
}
