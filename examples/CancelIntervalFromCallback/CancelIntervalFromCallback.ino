// Print five messages, then cancel this interval using its callback argument.

#include <AsyncTimer.h>

AsyncTimer t;

void setup() {
  Serial.begin(9600);

  unsigned int remaining = 5;
  t.setInterval([remaining](unsigned short id) mutable {
    Serial.println("Tick");
    if (--remaining == 0) {
      t.cancel(id);
      return;
    }
  }, 1000);
}

void loop() { t.handle(); }
