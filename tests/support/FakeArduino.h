#ifndef FAKE_ARDUINO_H
#define FAKE_ARDUINO_H

#include <string>
#include <vector>

namespace fake {
void reset();
void setTime(unsigned long time);
void advance(unsigned long milliseconds);
void setPin(int pin, int value);
int pinMode(int pin);
unsigned long serialBaud();
const std::vector<std::string> &serialLines();
}

#endif
