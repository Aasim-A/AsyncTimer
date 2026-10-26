#include "Test.h"

TEST(empty_timer_is_safe) {
  AsyncTimer timer;
  timer.handle();
  fake::advance(100);
  timer.handle();
  CHECK(timer.getRemaining(0) == 0);
  CHECK(timer.getRemaining(123) == 0);
}

TEST(timeout_runs_only_when_handled_and_only_once) {
  AsyncTimer timer;
  int calls = 0;
  const auto id = timer.setTimeout([&]() { ++calls; }, 10);
  CHECK(id != 0);
  CHECK(calls == 0);
  CHECK(timer.getRemaining(id) == 10);
  fake::advance(9);
  timer.handle();
  CHECK(calls == 0);
  CHECK(timer.getRemaining(id) == 1);
  fake::advance(1);
  CHECK(calls == 0);
  CHECK(timer.getRemaining(id) == 0);
  timer.handle();
  CHECK(calls == 1);
  timer.handle();
  fake::advance(100);
  timer.handle();
  CHECK(calls == 1);
  CHECK(timer.getRemaining(id) == 0);
}

TEST(timeout_uses_registration_time) {
  AsyncTimer timer;
  fake::setTime(123456);
  int calls = 0;
  const auto id = timer.setTimeout([&]() { ++calls; }, 50);
  fake::advance(49);
  timer.handle();
  CHECK(calls == 0);
  CHECK(timer.getRemaining(id) == 1);
  fake::advance(1);
  timer.handle();
  CHECK(calls == 1);
}

TEST(zero_timeout_waits_for_handle) {
  AsyncTimer timer;
  int calls = 0;
  timer.setTimeout([&]() { ++calls; }, 0);
  CHECK(calls == 0);
  timer.handle();
  timer.handle();
  CHECK(calls == 1);
}

TEST(interval_repeats_at_boundaries) {
  AsyncTimer timer;
  int calls = 0;
  const auto id = timer.setInterval([&]() { ++calls; }, 10);
  for (int expected = 1; expected <= 20; ++expected) {
    fake::advance(9);
    timer.handle();
    CHECK(calls == expected - 1);
    CHECK(timer.getRemaining(id) == 1);
    fake::advance(1);
    timer.handle();
    timer.handle();
    CHECK(calls == expected);
    CHECK(timer.getRemaining(id) == 10);
  }
}

TEST(overdue_interval_fires_once_and_restarts_from_handle_time) {
  AsyncTimer timer;
  int calls = 0;
  const auto id = timer.setInterval([&]() { ++calls; }, 10);
  fake::advance(105);
  CHECK(timer.getRemaining(id) == 0);
  timer.handle();
  CHECK(calls == 1);
  CHECK(timer.getRemaining(id) == 10);
  fake::advance(9);
  timer.handle();
  CHECK(calls == 1);
  fake::advance(1);
  timer.handle();
  CHECK(calls == 2);
}

TEST(zero_interval_fires_once_per_handle) {
  AsyncTimer timer;
  int calls = 0;
  timer.setInterval([&]() { ++calls; }, 0);
  for (int expected = 1; expected <= 5; ++expected) {
    timer.handle();
    CHECK(calls == expected);
  }
}

TEST(simultaneous_and_staggered_timers) {
  AsyncTimer timer;
  int first = 0, second = 0, third = 0;
  timer.setTimeout([&]() { ++first; }, 10);
  timer.setInterval([&]() { ++second; }, 10);
  timer.setTimeout([&]() { ++third; }, 15);
  fake::advance(10);
  timer.handle();
  CHECK(first == 1 && second == 1 && third == 0);
  fake::advance(5);
  timer.handle();
  CHECK(first == 1 && second == 1 && third == 1);
  fake::advance(5);
  timer.handle();
  CHECK(first == 1 && second == 2 && third == 1);
}

TEST(change_delay_shortens_without_resetting_elapsed_time) {
  AsyncTimer timer;
  int calls = 0;
  const auto id = timer.setTimeout([&]() { ++calls; }, 100);
  fake::advance(40);
  timer.changeDelay(id, 50);
  CHECK(timer.getRemaining(id) == 10);
  fake::advance(10);
  timer.handle();
  CHECK(calls == 1);
}

TEST(change_delay_extends_current_and_future_intervals) {
  AsyncTimer timer;
  int calls = 0;
  const auto id = timer.setInterval([&]() { ++calls; }, 10);
  fake::advance(5);
  timer.changeDelay(id, 20);
  CHECK(timer.getRemaining(id) == 15);
  fake::advance(5);
  timer.handle();
  CHECK(calls == 0);
  fake::advance(10);
  timer.handle();
  CHECK(calls == 1);
  CHECK(timer.getRemaining(id) == 20);
  fake::advance(20);
  timer.handle();
  CHECK(calls == 2);
}

TEST(change_delay_can_make_timer_immediately_due) {
  AsyncTimer timer;
  int calls = 0;
  const auto id = timer.setTimeout([&]() { ++calls; }, 100);
  fake::advance(50);
  timer.changeDelay(id, 20);
  CHECK(timer.getRemaining(id) == 0);
  CHECK(calls == 0);
  timer.handle();
  CHECK(calls == 1);
}

TEST(change_delay_to_zero) {
  AsyncTimer timer;
  int calls = 0;
  const auto id = timer.setInterval([&]() { ++calls; }, 100);
  timer.changeDelay(id, 0);
  timer.handle();
  CHECK(calls == 1);
  timer.handle();
  CHECK(calls == 2);
}

TEST(delay_adds_to_timeout_deadline) {
  AsyncTimer timer;
  int calls = 0;
  const auto id = timer.setTimeout([&]() { ++calls; }, 20);
  fake::advance(5);
  timer.delay(id, 10);
  timer.delay(id, 15);
  timer.delay(id, 0);
  CHECK(timer.getRemaining(id) == 40);
  fake::advance(39);
  timer.handle();
  CHECK(calls == 0);
  fake::advance(1);
  timer.handle();
  CHECK(calls == 1);
}

TEST(delay_affects_only_next_interval) {
  AsyncTimer timer;
  int calls = 0;
  const auto id = timer.setInterval([&]() { ++calls; }, 10);
  timer.delay(id, 20);
  fake::advance(29);
  timer.handle();
  CHECK(calls == 0);
  fake::advance(1);
  timer.handle();
  CHECK(calls == 1);
  CHECK(timer.getRemaining(id) == 10);
  fake::advance(10);
  timer.handle();
  CHECK(calls == 2);
}

TEST(delay_can_postpone_an_overdue_timeout) {
  AsyncTimer timer;
  int calls = 0;
  const auto id = timer.setTimeout([&]() { ++calls; }, 10);
  fake::advance(15);
  timer.delay(id, 10);
  CHECK(timer.getRemaining(id) == 5);
  timer.handle();
  CHECK(calls == 0);
  fake::advance(5);
  timer.handle();
  CHECK(calls == 1);
}

TEST(change_delay_preserves_extra_postponement) {
  AsyncTimer timer;
  int calls = 0;
  const auto id = timer.setTimeout([&]() { ++calls; }, 20);
  timer.delay(id, 30);
  fake::advance(5);
  timer.changeDelay(id, 10);
  CHECK(timer.getRemaining(id) == 35);
  fake::advance(35);
  timer.handle();
  CHECK(calls == 1);
}

TEST(reset_restarts_timeout_and_clears_postponement) {
  AsyncTimer timer;
  int calls = 0;
  const auto id = timer.setTimeout([&]() { ++calls; }, 20);
  timer.delay(id, 50);
  fake::advance(15);
  timer.reset(id);
  CHECK(timer.getRemaining(id) == 20);
  fake::advance(19);
  timer.handle();
  CHECK(calls == 0);
  timer.reset(id);
  fake::advance(20);
  timer.handle();
  CHECK(calls == 1);
}

TEST(reset_uses_changed_interval_period) {
  AsyncTimer timer;
  int calls = 0;
  const auto id = timer.setInterval([&]() { ++calls; }, 10);
  fake::advance(10);
  timer.handle();
  timer.changeDelay(id, 25);
  fake::advance(5);
  timer.reset(id);
  CHECK(timer.getRemaining(id) == 25);
  fake::advance(25);
  timer.handle();
  CHECK(calls == 2);
}

TEST(unknown_ids_do_not_affect_active_timers) {
  AsyncTimer timer;
  int calls = 0;
  const auto id = timer.setTimeout([&]() { ++calls; }, 10);
  const unsigned short unknown = id == 1 ? 2 : 1;
  for (const auto invalid : {static_cast<unsigned short>(0), unknown}) {
    timer.changeDelay(invalid, 100);
    timer.delay(invalid, 100);
    timer.reset(invalid);
    timer.cancel(invalid);
    CHECK(timer.getRemaining(invalid) == 0);
  }
  CHECK(timer.getRemaining(id) == 10);
  fake::advance(10);
  timer.handle();
  CHECK(calls == 1);
}

TEST(expired_timer_cannot_be_revived) {
  AsyncTimer timer;
  int calls = 0;
  const auto id = timer.setTimeout([&]() { ++calls; }, 0);
  timer.handle();
  timer.changeDelay(id, 1);
  timer.delay(id, 1);
  timer.reset(id);
  fake::advance(100);
  timer.handle();
  CHECK(calls == 1);
  CHECK(timer.getRemaining(id) == 0);
}

// The legacy entry point remains callable without changing active timers.
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
TEST(legacy_setup_does_not_reset_active_timers) {
  AsyncTimer timer;
  int calls = 0;
  timer.setup();
  const auto id = timer.setTimeout([&]() { ++calls; }, 10);
  fake::advance(5);
  timer.setup();
  CHECK(timer.getRemaining(id) == 5);
  fake::advance(5);
  timer.handle();
  CHECK(calls == 1);
}
#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
