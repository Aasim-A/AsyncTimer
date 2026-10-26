#include "Test.h"
#include "../../examples/DebounceUsingTimeout/DebounceUsingTimeout.ino"

TEST(debounce_example_waits_until_fifty_ms_after_last_high_sample) {
  setup();
  CHECK(fake::pinMode(BUTTON_PIN) == INPUT);
  CHECK(fake::serialBaud() == 9600);
  fake::setPin(BUTTON_PIN, HIGH);
  loop();
  fake::advance(20);
  loop();
  fake::advance(20);
  loop();
  fake::setPin(BUTTON_PIN, LOW);
  fake::advance(49);
  loop();
  CHECK(fake::serialLines().empty());
  fake::advance(1);
  loop();
  CHECK(fake::serialLines().size() == 1);
  CHECK(fake::serialLines()[0] == "Button pressed!");
  CHECK(timeoutId == 0);
  fake::advance(100);
  loop();
  CHECK(fake::serialLines().size() == 1);
  fake::setPin(BUTTON_PIN, HIGH);
  loop();
  fake::setPin(BUTTON_PIN, LOW);
  fake::advance(50);
  loop();
  CHECK(fake::serialLines().size() == 2);
}
