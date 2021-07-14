// Send a message to the serial after 10 seconds from booting

#include <AsyncTimer.h>

AsyncTimer t;

void setup() {
  t.setup();
  Serial.begin(9600);

  t.setTimeout(
      []() { Serial.println("Message after 10 seconds from booting"); }, 10000);
}

void loop() { t.handle(); }