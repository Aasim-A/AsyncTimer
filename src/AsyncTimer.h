/*
  Copyright (c) 2021 Aasim-A
  Respository: https://github.com/Aasim-A/AsyncTimer

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#ifndef ASYNC_TIMER_H
#define ASYNC_TIMER_H

#include <Arduino.h>

class AsyncTimer {
private:
  struct m_TimerInfo {
    unsigned short id;
    void (*callback)();
    unsigned int delayByMs;
    unsigned long timestamp;
    bool indefinite;
    bool active;

    m_TimerInfo() : active(false) {}
  };

  unsigned short m_newTimerInfo(void (*callback)(), unsigned int ms,
                                bool indefinite);

  unsigned short m_maxArrayLength;
  unsigned short m_arrayLength = 0;
  m_TimerInfo *m_callsArray;
  unsigned short m_availableIndecesLength;
  unsigned short *m_availableIndeces;

public:
  AsyncTimer(unsigned short arrayLength = 10) {
    m_maxArrayLength = arrayLength;
    m_callsArray = new m_TimerInfo[arrayLength];
    m_availableIndecesLength = arrayLength;
    m_availableIndeces = new unsigned short[arrayLength];
    for (short i = 0; i < m_availableIndecesLength; i++)
      m_availableIndeces[i] = i;
  }
  ~AsyncTimer() {
    delete[] m_callsArray;
    delete[] m_availableIndeces;
  }
  void setup();
  unsigned short setTimeout(void (*callback)(), unsigned int ms);
  unsigned short setInterval(void (*callback)(), unsigned int ms);
  void changeDelay(unsigned short id, unsigned int ms);
  void delay(unsigned short id, unsigned int ms);
  void reset(unsigned short id);
  void cancel(unsigned short id);
  void handle();
};

#endif
