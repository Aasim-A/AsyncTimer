#include "Test.h"
#include "../../examples/CancelIntervalFromCallback/CancelIntervalFromCallback.ino"

TEST(self_cancelling_example_prints_exactly_five_messages) {
  setup();
  CHECK(fake::serialBaud() == 9600);
  for (unsigned int seconds = 1; seconds <= 10; ++seconds) {
    fake::advance(1000);
    loop();
    CHECK(fake::serialLines().size() == (seconds < 5 ? seconds : 5));
  }
  for (const auto &line : fake::serialLines()) CHECK(line == "Tick");
}
