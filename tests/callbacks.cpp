#include "Test.h"

#include <memory>

namespace {
int functionCalls;
void plainCallback() { ++functionCalls; }
struct Functor {
  int *calls;
  void operator()() { ++*calls; }
};
}

TEST(function_pointer_timeout_and_interval) {
  AsyncTimer timer;
  functionCalls = 0;
  timer.setTimeout(plainCallback, 10);
  timer.setInterval(&plainCallback, 10);
  fake::advance(10);
  timer.handle();
  CHECK(functionCalls == 2);
  fake::advance(10);
  timer.handle();
  CHECK(functionCalls == 3);
}

TEST(non_capturing_lambda) {
  AsyncTimer timer;
  functionCalls = 0;
  timer.setTimeout([]() { ++functionCalls; }, 0);
  timer.handle();
  CHECK(functionCalls == 1);
}

TEST(lambda_captures_values_and_references) {
  AsyncTimer timer;
  int input = 7, result = 0;
  timer.setTimeout([input, &result]() { result = input; }, 0);
  input = 99;
  timer.handle();
  CHECK(result == 7 && input == 99);
}

TEST(mutable_lambda_retains_state_between_intervals) {
  AsyncTimer timer;
  int state = 0, observed = 0;
  timer.setInterval([state, &observed]() mutable { observed = ++state; }, 1);
  for (int expected = 1; expected <= 5; ++expected) {
    fake::advance(1);
    timer.handle();
    CHECK(observed == expected);
  }
  CHECK(state == 0);
}

TEST(callable_object) {
  AsyncTimer timer;
  int calls = 0;
  timer.setTimeout(Functor{&calls}, 0);
  timer.handle();
  CHECK(calls == 1);
}

TEST(captured_object_lives_until_timeout_finishes) {
  AsyncTimer timer;
  auto object = std::make_shared<int>(42);
  const std::weak_ptr<int> weak = object;
  int observed = 0;
  timer.setTimeout([object, &observed]() { observed = *object; }, 1);
  object.reset();
  CHECK(!weak.expired());
  fake::advance(1);
  timer.handle();
  CHECK(observed == 42);
  CHECK(weak.expired());
}

TEST(cancellation_releases_captured_objects) {
  AsyncTimer timer;
  auto object = std::make_shared<int>(42);
  const std::weak_ptr<int> weak = object;
  const auto id = timer.setInterval([object]() {}, 1);
  object.reset();
  timer.cancel(id);
  CHECK(weak.expired());
}

TEST(bulk_cancellation_releases_only_cancelled_captures) {
  AsyncTimer timer(2);
  auto timeoutObject = std::make_shared<int>(1);
  auto intervalObject = std::make_shared<int>(2);
  const std::weak_ptr<int> timeoutWeak = timeoutObject;
  const std::weak_ptr<int> intervalWeak = intervalObject;
  timer.setTimeout([timeoutObject]() {}, 10);
  timer.setInterval([intervalObject]() {}, 10);
  timeoutObject.reset();
  intervalObject.reset();
  timer.cancelAll(false);
  CHECK(timeoutWeak.expired());
  CHECK(!intervalWeak.expired());
  timer.cancelAll();
  CHECK(intervalWeak.expired());
}

TEST(failed_registration_releases_its_capture) {
  AsyncTimer timer(0);
  auto object = std::make_shared<int>(42);
  const std::weak_ptr<int> weak = object;
  CHECK(timer.setTimeout([object]() {}, 0) == 0);
  CHECK(timer.setInterval([object]() {}, 0) == 0);
  object.reset();
  CHECK(weak.expired());
}

TEST(destruction_releases_pending_callbacks) {
  auto object = std::make_shared<int>(42);
  const std::weak_ptr<int> weak = object;
  {
    AsyncTimer timer;
    timer.setTimeout([object]() {}, 10);
    timer.setInterval([object]() {}, 10);
    object.reset();
    CHECK(!weak.expired());
  }
  CHECK(weak.expired());
}

TEST(timeout_can_schedule_replacement_and_keep_using_its_capture) {
  AsyncTimer timer(1);
  auto object = std::make_shared<int>(42);
  const std::weak_ptr<int> weak = object;
  int observed = 0, replacements = 0;
  unsigned short replacement = 0;
  timer.setTimeout([&timer, object, &observed, &replacements, &replacement]() {
    replacement = timer.setTimeout([&replacements]() { ++replacements; }, 5);
    observed = *object;
  }, 0);
  object.reset();
  timer.handle();
  CHECK(replacement != 0);
  CHECK(observed == 42);
  CHECK(weak.expired());
  CHECK(replacements == 0);
  fake::advance(5);
  timer.handle();
  CHECK(replacements == 1);
}

TEST(interval_can_cancel_itself_and_schedule_replacement) {
  AsyncTimer timer(1);
  auto object = std::make_shared<int>(42);
  int observed = 0, replacements = 0;
  unsigned short id = 0;
  id = timer.setInterval([&, object]() {
    timer.cancel(id);
    CHECK(timer.setTimeout([&replacements]() { ++replacements; }, 5) != 0);
    observed = *object;
  }, 1);
  object.reset();
  fake::advance(1);
  timer.handle();
  CHECK(observed == 42);
  fake::advance(5);
  timer.handle();
  CHECK(replacements == 1);
}

TEST(interval_can_cancel_itself_without_replacement) {
  AsyncTimer timer;
  int calls = 0;
  unsigned short id = 0;
  id = timer.setInterval([&]() { ++calls; timer.cancel(id); }, 1);
  fake::advance(1);
  timer.handle();
  fake::advance(10);
  timer.handle();
  CHECK(calls == 1);
}

TEST(callback_can_cancel_another_timer) {
  AsyncTimer timer;
  int cancelledCalls = 0;
  const auto victim = timer.setTimeout([&]() { ++cancelledCalls; }, 10);
  timer.setTimeout([&]() { timer.cancel(victim); }, 5);
  fake::advance(5);
  timer.handle();
  fake::advance(5);
  timer.handle();
  CHECK(cancelledCalls == 0);
}

TEST(timeout_can_cancel_all_and_schedule_again) {
  AsyncTimer timer(3);
  int cancelledCalls = 0, replacementCalls = 0;
  timer.setInterval([&]() { ++cancelledCalls; }, 100);
  timer.setTimeout([&]() {
    timer.cancelAll();
    CHECK(timer.setTimeout([&]() { ++replacementCalls; }, 5) != 0);
  }, 0);
  timer.handle();
  fake::advance(100);
  timer.handle();
  CHECK(cancelledCalls == 0 && replacementCalls == 1);
}

TEST(interval_can_change_its_own_delay) {
  AsyncTimer timer;
  int calls = 0;
  unsigned short id = 0;
  id = timer.setInterval([&]() { ++calls; timer.changeDelay(id, 20); }, 10);
  fake::advance(10);
  timer.handle();
  CHECK(calls == 1);
  CHECK(timer.getRemaining(id) == 20);
  fake::advance(20);
  timer.handle();
  CHECK(calls == 2);
}

TEST(reset_inside_callback_uses_current_clock) {
  AsyncTimer timer;
  int calls = 0;
  unsigned short id = 0;
  id = timer.setInterval([&]() {
    ++calls;
    fake::advance(5);
    timer.reset(id);
  }, 10);
  fake::advance(10);
  timer.handle();
  CHECK(calls == 1);
  CHECK(timer.getRemaining(id) == 10);
}

TEST(slot_can_switch_between_function_and_lambda_callbacks) {
  AsyncTimer timer(1);
  functionCalls = 0;
  int lambdaCalls = 0;
  for (int i = 0; i < 20; ++i) {
    CHECK(timer.setTimeout(plainCallback, 0) != 0);
    timer.handle();
    CHECK(timer.setTimeout([&]() { ++lambdaCalls; }, 0) != 0);
    timer.handle();
  }
  CHECK(functionCalls == 20 && lambdaCalls == 20);
}

TEST(nested_handle_does_not_reenter_the_running_interval) {
  AsyncTimer timer(1);
  int calls = 0;
  timer.setInterval([&]() {
    ++calls;
    if (calls == 1) timer.handle();
  }, 0);
  timer.handle();
  CHECK(calls == 1);
  timer.handle();
  CHECK(calls == 2);
}

TEST(running_interval_still_occupies_capacity) {
  AsyncTimer timer(1);
  int calls = 0;
  timer.setInterval([&]() {
    ++calls;
    CHECK(timer.setTimeout([]() {}, 0) == 0);
  }, 1);
  fake::advance(1);
  timer.handle();
  fake::advance(1);
  timer.handle();
  CHECK(calls == 2);
}

TEST(interval_survives_cancelling_only_timeouts_from_its_callback) {
  AsyncTimer timer(3);
  int intervals = 0, timeouts = 0;
  timer.setTimeout([&]() { ++timeouts; }, 100);
  timer.setInterval([&]() {
    ++intervals;
    timer.cancelAll(false);
  }, 10);
  for (int i = 0; i < 12; ++i) {
    fake::advance(10);
    timer.handle();
  }
  CHECK(intervals == 12 && timeouts == 0);
}

TEST(interval_capture_stays_alive_during_cancel_all) {
  AsyncTimer timer(1);
  auto object = std::make_shared<int>(42);
  const std::weak_ptr<int> weak = object;
  int observed = 0;
  timer.setInterval([&, object]() {
    timer.cancelAll();
    CHECK(!weak.expired());
    observed = *object;
  }, 0);
  object.reset();
  timer.handle();
  CHECK(observed == 42);
  CHECK(weak.expired());
}

TEST(throwing_timeout_releases_capture_and_capacity) {
  AsyncTimer timer(1);
  auto object = std::make_shared<int>(42);
  const std::weak_ptr<int> weak = object;
  timer.setTimeout([object]() { throw std::runtime_error("callback failure"); }, 0);
  object.reset();
  bool caught = false;
  try { timer.handle(); }
  catch (const std::runtime_error &) { caught = true; }
  CHECK(caught);
  CHECK(weak.expired());
  CHECK(timer.setTimeout([]() {}, 0) != 0);
}

TEST(throwing_interval_retains_callback_for_the_next_tick) {
  AsyncTimer timer(1);
  int calls = 0;
  const auto id = timer.setInterval([&]() {
    ++calls;
    if (calls == 1) throw std::runtime_error("callback failure");
  }, 10);
  fake::advance(10);
  bool caught = false;
  try { timer.handle(); }
  catch (const std::runtime_error &) { caught = true; }
  CHECK(caught);
  CHECK(timer.getRemaining(id) == 10);
  fake::advance(10);
  timer.handle();
  CHECK(calls == 2);
}
