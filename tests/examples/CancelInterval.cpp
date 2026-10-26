#include "Test.h"
#include "../../examples/CancelInterval/CancelInterval.ino"

TEST(cancel_interval_example_stops_blinking_after_ten_seconds) {
  setup();
  for (int seconds = 1; seconds < 10; ++seconds) {
    fake::advance(1000);
    loop();
    CHECK(digitalRead(LED_BUILTIN) == (seconds % 2 ? LOW : HIGH));
  }
  fake::advance(1000);
  loop();
  // Ordering of two callbacks due at the same instant is unspecified.
  const int finalState = digitalRead(LED_BUILTIN);
  for (int i = 0; i < 5; ++i) {
    fake::advance(1000);
    loop();
    CHECK(digitalRead(LED_BUILTIN) == finalState);
  }
}
