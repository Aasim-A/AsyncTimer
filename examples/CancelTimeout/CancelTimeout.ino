// Cancel a timeout before it executes

#include <AsyncTimer.h>

AsyncTimer t;

void setup() {
  Serial.begin(9600);

  unsigned short timeoutId = t.setTimeout(
      []() { Serial.println("Message after 10 seconds from booting"); }, 10000);

  // Cancel instantly:
  // t.cancel(timeoutId);

  // Or cancel using another timeout after 3 seconds:
  t.setTimeout([]() { t.cancel(timeoutId); }, 3000);
}

void loop() { t.handle(); }