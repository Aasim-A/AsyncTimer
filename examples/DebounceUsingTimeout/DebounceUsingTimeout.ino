// Debounce button using a timeout and delay function

#include <AsyncTimer.h>

#define BUTTON_PIN 4

AsyncTimer t;

void setup() {
  Serial.begin(9600);

  pinMode(BUTTON_PIN, INPUT);
}

unsigned short timeoutId = 0;

void loop() {
  t.handle();

  if (digitalRead(BUTTON_PIN) == HIGH) {
    if (timeoutId != 0)
      t.reset(timeoutId);
    else
      timeoutId = t.setTimeout(
          []() {
            Serial.println("Button pressed!");
            timeoutId = 0;
          },
          50);
  }
}