#include "Test.h"
#include "../../examples/CancelTimeout/CancelTimeout.ino"

TEST(cancel_timeout_example_never_prints) {
  setup();
  CHECK(fake::serialBaud() == 9600);
  fake::advance(3000);
  loop();
  fake::advance(7000);
  loop();
  fake::advance(10000);
  loop();
  CHECK(fake::serialLines().empty());
}
