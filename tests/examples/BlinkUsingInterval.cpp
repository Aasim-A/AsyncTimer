#include "Test.h"
#include "../../examples/BlinkUsingInterval/BlinkUsingInterval.ino"

TEST(blink_example_toggles_led_every_second) {
  setup();
  CHECK(fake::pinMode(LED_BUILTIN) == OUTPUT);
  CHECK(digitalRead(LED_BUILTIN) == HIGH);
  fake::advance(999);
  loop();
  CHECK(digitalRead(LED_BUILTIN) == HIGH);
  fake::advance(1);
  loop();
  CHECK(digitalRead(LED_BUILTIN) == LOW);
  fake::advance(1000);
  loop();
  CHECK(digitalRead(LED_BUILTIN) == HIGH);
}
