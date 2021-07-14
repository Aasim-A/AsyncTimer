// Blink the led every 1 second using setInterval

#include <AsyncTimer.h>

AsyncTimer t;

void setup() {
  t.setup();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  t.setInterval([]() { digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); },
                1000);
}

void loop() { t.handle(); }