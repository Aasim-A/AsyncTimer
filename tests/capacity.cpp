#include "Test.h"

#include <climits>
#include <set>

TEST(default_capacity_is_ten) {
  AsyncTimer timer;
  std::set<unsigned short> ids;
  for (int i = 0; i < 10; ++i) {
    const auto id = i % 2 ? timer.setTimeout([]() {}, 10)
                          : timer.setInterval([]() {}, 10);
    CHECK(id != 0);
    CHECK(ids.insert(id).second);
  }
  CHECK(timer.setTimeout([]() {}, 10) == 0);
  CHECK(timer.setInterval([]() {}, 10) == 0);
}

TEST(custom_capacity_is_respected) {
  AsyncTimer timer(22);
  for (int i = 0; i < 22; ++i) CHECK(timer.setTimeout([]() {}, 10) != 0);
  CHECK(timer.setTimeout([]() {}, 10) == 0);
}

TEST(zero_capacity_rejects_all_timers) {
  AsyncTimer timer(0);
  CHECK(timer.setTimeout([]() {}, 0) == 0);
  CHECK(timer.setInterval([]() {}, 0) == 0);
  timer.handle();
  timer.cancelAll();
  timer.cancelAll(false);
  timer.cancel(0);
  timer.reset(0);
  timer.delay(0, 1);
  timer.changeDelay(0, 1);
  CHECK(timer.getRemaining(0) == 0);
}

TEST(failed_registration_does_not_change_existing_timer) {
  AsyncTimer timer(1);
  int accepted = 0, rejected = 0;
  const auto id = timer.setTimeout([&]() { ++accepted; }, 10);
  CHECK(timer.setTimeout([&]() { ++rejected; }, 0) == 0);
  CHECK(timer.getRemaining(id) == 10);
  fake::advance(10);
  timer.handle();
  CHECK(accepted == 1 && rejected == 0);
}

TEST(expired_timeouts_release_all_capacity) {
  AsyncTimer timer(3);
  for (int cycle = 0; cycle < 100; ++cycle) {
    int calls = 0;
    for (int i = 0; i < 3; ++i)
      CHECK(timer.setTimeout([&]() { ++calls; }, 0) != 0);
    CHECK(timer.setTimeout([]() {}, 0) == 0);
    timer.handle();
    CHECK(calls == 3);
  }
}

TEST(cancellation_releases_exactly_one_slot) {
  AsyncTimer timer(2);
  const auto first = timer.setInterval([]() {}, 10);
  const auto second = timer.setInterval([]() {}, 10);
  timer.cancel(first);
  timer.cancel(first);
  const auto replacement = timer.setTimeout([]() {}, 10);
  CHECK(replacement != 0 && replacement != second);
  CHECK(timer.setTimeout([]() {}, 10) == 0);
}

TEST(active_ids_remain_unique_during_slot_reuse) {
  AsyncTimer timer(32);
  std::vector<unsigned short> ids;
  for (int i = 0; i < 32; ++i) ids.push_back(timer.setInterval([]() {}, 10));
  for (int cycle = 0; cycle < 2000; ++cycle) {
    const auto slot = static_cast<std::size_t>(cycle) % ids.size();
    timer.cancel(ids[slot]);
    ids[slot] = timer.setInterval([]() {}, 10);
    const std::set<unsigned short> unique(ids.begin(), ids.end());
    CHECK(unique.size() == ids.size());
    CHECK(unique.count(0) == 0);
  }
}

TEST(ids_stay_nonzero_and_unique_across_the_full_id_range) {
  AsyncTimer timer(2);
  const auto permanent = timer.setInterval([]() {}, 100);
  // A live ID must remain protected even after the unsigned-short space wraps.
  for (unsigned long i = 0; i < static_cast<unsigned long>(USHRT_MAX) + 2; ++i) {
    const auto id = timer.setTimeout([]() {}, 0);
    CHECK(id != 0);
    CHECK(id != permanent);
    timer.cancel(id);
  }
  CHECK(timer.getRemaining(permanent) == 100);
}
