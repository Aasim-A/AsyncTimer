// Cancel an interval after 10 seconds

#include <AsyncTimer.h>

AsyncTimer t;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  unsigned short intervalId = t.setInterval(
      []() { digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); }, 1000);

  t.setTimeout([]() { t.cancel(intervalId); }, 10000);
}

void loop() { t.handle(); }