#ifndef TEST_ARDUINO_H
#define TEST_ARDUINO_H

#include <cstdlib>

// Only the Arduino APIs used by the library and its examples are emulated.
unsigned long millis();
const int INPUT = 0;
const int OUTPUT = 1;
const int LOW = 0;
const int HIGH = 1;
const int LED_BUILTIN = 13;
void pinMode(int pin, int mode);
void digitalWrite(int pin, int value);
int digitalRead(int pin);

class FakeSerial {
public:
  void begin(unsigned long baud);
  void println(const char *message);
};
extern FakeSerial Serial;

#endif
