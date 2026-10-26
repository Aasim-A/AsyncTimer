#include "Arduino.h"
#include "FakeArduino.h"

#include <map>

namespace {
unsigned long clockTime = 0;
unsigned long baudRate = 0;
// Function-local state also works for sketches with global AsyncTimer objects.
std::map<int, int> &pins() { static std::map<int, int> state; return state; }
std::map<int, int> &modes() { static std::map<int, int> state; return state; }
std::vector<std::string> &lines() { static std::vector<std::string> state; return state; }
}

unsigned long millis() { return clockTime; }
void pinMode(int pin, int mode) { modes()[pin] = mode; }
void digitalWrite(int pin, int value) { pins()[pin] = value; }
int digitalRead(int pin) { return pins()[pin]; }
FakeSerial Serial;
void FakeSerial::begin(unsigned long baud) { baudRate = baud; }
void FakeSerial::println(const char *message) { lines().push_back(message); }

namespace fake {
void reset() {
  clockTime = 0;
  baudRate = 0;
  pins().clear();
  modes().clear();
  lines().clear();
}
void setTime(unsigned long time) { clockTime = time; }
void advance(unsigned long milliseconds) { clockTime += milliseconds; }
void setPin(int pin, int value) { pins()[pin] = value; }
int pinMode(int pin) { return modes()[pin]; }
unsigned long serialBaud() { return baudRate; }
const std::vector<std::string> &serialLines() { return lines(); }
}
