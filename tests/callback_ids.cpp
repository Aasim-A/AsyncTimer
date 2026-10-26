#include "Test.h"

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

namespace {
unsigned short functionId;
void functionWithId(unsigned short id) { functionId = id; }

unsigned short scheduleFromLocalScope(AsyncTimer &timer, int &calls) {
  // The registration's local ID has gone out of scope when this callback runs.
  const auto localId = timer.setInterval([&timer, &calls](unsigned short self) {
    ++calls;
    timer.cancel(self);
  }, 10);
  return localId;
}

struct IdFunctor {
  unsigned short &observed;
  void operator()(unsigned short id) { observed = id; }
};

struct OverloadedFunctor {
  int &observed;
  void operator()() { observed = 1; }
  void operator()(unsigned short) { observed = 2; }
};

template <typename T> class AcceptsTimeout {
  template <typename U>
  static auto check(int)
      -> decltype(std::declval<AsyncTimer &>().setTimeout(std::declval<U>(), 0),
                  std::true_type());
  template <typename> static std::false_type check(...);

public:
  static const bool value = decltype(check<T>(0))::value;
};
}

TEST(timeout_accepts_only_zero_argument_callbacks) {
  static_assert(AcceptsTimeout<void (*)()>::value, "Accept no-argument functions");
  static_assert(!AcceptsTimeout<decltype(&functionWithId)>::value,
                "Reject functions requiring a timer ID");
  static_assert(!AcceptsTimeout<IdFunctor>::value, "Reject ID-only functors");
  static_assert(!AcceptsTimeout<std::function<void(unsigned short)>>::value,
                "Reject ID-only std::function callbacks");
  auto idLambda = [](unsigned short) {};
  static_assert(!AcceptsTimeout<decltype(idLambda)>::value, "Reject ID-only lambdas");
  AsyncTimer timer;
  int calls = 0;
  const auto id = timer.setTimeout([&]() { ++calls; }, 0);
  CHECK(id != 0 && calls == 0);
  timer.handle();
  timer.handle();
  CHECK(calls == 1);
}

TEST(interval_receives_the_same_id_on_each_tick) {
  AsyncTimer timer;
  std::vector<unsigned short> observed;
  const auto id = timer.setInterval([&](unsigned short self) { observed.push_back(self); }, 10);
  for (int i = 0; i < 5; ++i) {
    fake::advance(10);
    timer.handle();
  }
  CHECK(observed.size() == 5);
  for (const auto self : observed) CHECK(self == id);
}

TEST(interval_can_cancel_itself_after_registration_scope_exits) {
  AsyncTimer timer;
  int calls = 0;
  const auto id = scheduleFromLocalScope(timer, calls);
  fake::advance(10);
  timer.handle();
  CHECK(calls == 1);
  CHECK(timer.getRemaining(id) == 0);
  fake::advance(100);
  timer.handle();
  CHECK(calls == 1);
}

TEST(mutable_callback_can_stop_itself_after_several_ticks) {
  AsyncTimer timer;
  int calls = 0;
  int remaining = 3;
  timer.setInterval([&timer, &calls, remaining](unsigned short self) mutable {
    ++calls;
    if (--remaining == 0) timer.cancel(self);
  }, 1);
  for (int i = 0; i < 10; ++i) {
    fake::advance(1);
    timer.handle();
  }
  CHECK(calls == 3);
  CHECK(remaining == 3);
}

TEST(self_cancellation_leaves_other_intervals_running) {
  AsyncTimer timer;
  int selfCalls = 0, otherCalls = 0;
  timer.setInterval([&](unsigned short self) {
    ++selfCalls;
    timer.cancel(self);
  }, 1);
  timer.setInterval([&]() { ++otherCalls; }, 1);
  for (int i = 0; i < 3; ++i) {
    fake::advance(1);
    timer.handle();
  }
  CHECK(selfCalls == 1 && otherCalls == 3);
}

TEST(id_function_pointer_supports_intervals) {
  AsyncTimer timer;
  functionId = 0;
  const auto first = timer.setInterval(functionWithId, 0);
  timer.handle();
  CHECK(functionId == first);
  timer.cancel(first);
  const auto interval = timer.setInterval(&functionWithId, 1);
  fake::advance(1);
  timer.handle();
  CHECK(functionId == interval);
}

TEST(non_capturing_id_lambda) {
  AsyncTimer timer;
  functionId = 0;
  const auto id = timer.setInterval([](unsigned short self) { functionId = self; }, 0);
  timer.handle();
  CHECK(functionId == id);
}

TEST(id_callable_object_and_std_function) {
  AsyncTimer timer;
  unsigned short observed = 0;
  const auto first = timer.setInterval(IdFunctor{observed}, 0);
  timer.handle();
  CHECK(observed == first);
  timer.cancel(first);
  std::function<void(unsigned short)> callback = [&](unsigned short self) { observed = self; };
  const auto second = timer.setInterval(callback, 0);
  timer.handle();
  CHECK(observed == second);
}

TEST(existing_overloaded_callable_keeps_zero_argument_behavior) {
  AsyncTimer timer;
  int observed = 0;
  timer.setTimeout(OverloadedFunctor{observed}, 0);
  timer.handle();
  CHECK(observed == 1);
  observed = 0;
  timer.setInterval(OverloadedFunctor{observed}, 0);
  timer.handle();
  CHECK(observed == 1);
}

TEST(existing_default_argument_callable_keeps_its_default) {
  AsyncTimer timer;
  unsigned short observed = 0;
  timer.setTimeout([&](unsigned short self = 99) { observed = self; }, 0);
  timer.handle();
  CHECK(observed == 99);
  observed = 0;
  timer.setInterval([&](unsigned short self = 99) { observed = self; }, 0);
  timer.handle();
  CHECK(observed == 99);
}

TEST(self_cancel_and_slot_reuse_keep_id_and_capture_valid) {
  AsyncTimer timer(1);
  auto object = std::make_shared<int>(42);
  const std::weak_ptr<int> weak = object;
  unsigned short observed = 0, replacement = 0, replacementObserved = 0;
  int valueAfterCancel = 0;
  const auto original = timer.setInterval([&, object](unsigned short self) {
    timer.cancel(self);
    replacement = timer.setInterval([&](unsigned short next) {
      replacementObserved = next;
      timer.cancel(next);
    }, 5);
    observed = self;
    valueAfterCancel = *object;
    timer.cancel(self); // The old ID must not cancel the replacement.
  }, 0);
  object.reset();
  timer.handle();
  CHECK(observed == original);
  CHECK(replacement != 0 && replacement != original);
  CHECK(valueAfterCancel == 42);
  CHECK(weak.expired());
  fake::advance(5);
  timer.handle();
  CHECK(replacementObserved == replacement);
}

TEST(timeout_is_already_removed_when_zero_argument_callback_runs) {
  AsyncTimer timer(1);
  int calls = 0;
  unsigned short replacement = 0, observed = 0;
  unsigned short original = 0;
  original = timer.setTimeout([&]() {
    observed = original;
    CHECK(timer.getRemaining(original) == 0);
    replacement = timer.setTimeout([&]() { ++calls; }, 1);
    timer.cancel(original);
  }, 0);
  timer.handle();
  CHECK(observed == original);
  CHECK(replacement != 0 && replacement != original);
  fake::advance(1);
  timer.handle();
  CHECK(calls == 1);
}

TEST(passed_id_can_change_and_postpone_its_interval) {
  AsyncTimer timer;
  int calls = 0;
  const auto id = timer.setInterval([&](unsigned short self) {
    ++calls;
    timer.changeDelay(self, 20);
    timer.delay(self, 5);
  }, 10);
  fake::advance(10);
  timer.handle();
  CHECK(calls == 1);
  CHECK(timer.getRemaining(id) == 25);
  fake::advance(24);
  timer.handle();
  CHECK(calls == 1);
  fake::advance(1);
  timer.handle();
  CHECK(calls == 2);
}

TEST(nested_callbacks_receive_their_own_ids) {
  AsyncTimer timer(2);
  unsigned short innerObserved = 0, outerObserved = 0;
  const auto inner = timer.setInterval([&](unsigned short self) {
    innerObserved = self;
    timer.cancel(self);
  }, 5);
  const auto outer = timer.setInterval([&](unsigned short self) {
    fake::advance(5);
    timer.handle();
    outerObserved = self;
    timer.cancel(self);
  }, 0);
  timer.handle();
  CHECK(innerObserved == inner);
  CHECK(outerObserved == outer);
  CHECK(timer.getRemaining(outer) == 0);
}

TEST(failed_id_callback_registration_releases_capture) {
  AsyncTimer timer(0);
  auto object = std::make_shared<int>(42);
  const std::weak_ptr<int> weak = object;
  int calls = 0;
  CHECK(timer.setInterval([&, object](unsigned short) { ++calls; }, 0) == 0);
  object.reset();
  timer.handle();
  CHECK(calls == 0 && weak.expired());
}

TEST(id_callback_self_cancellation_survives_rollover) {
  AsyncTimer timer;
  unsigned short observed = 0;
  fake::setTime(~0UL - 4);
  const auto id = timer.setInterval([&](unsigned short self) {
    observed = self;
    timer.cancel(self);
  }, 10);
  fake::advance(10);
  timer.handle();
  CHECK(observed == id);
  CHECK(timer.getRemaining(id) == 0);
}
