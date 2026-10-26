#include "Test.h"
#include "../../examples/SerialMsgUsingTimeout/SerialMsgUsingTimeout.ino"

TEST(serial_message_example_prints_once_after_ten_seconds) {
  setup();
  CHECK(fake::serialBaud() == 9600);
  fake::advance(9999);
  loop();
  CHECK(fake::serialLines().empty());
  fake::advance(1);
  loop();
  CHECK(fake::serialLines().size() == 1);
  CHECK(fake::serialLines()[0] == "Message after 10 seconds from booting");
  fake::advance(10000);
  loop();
  CHECK(fake::serialLines().size() == 1);
}
