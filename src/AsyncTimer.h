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
    unsigned long delayByMs;
    unsigned long timestamp;
    bool indefinite;
    bool active;

    m_TimerInfo() : active(false) {}
  };

  unsigned short m_newTimerInfo(void (*callback)(), unsigned long ms,
                                bool indefinite);

  unsigned short m_maxArrayLength;
  unsigned short m_arrayLength = 0;
  m_TimerInfo *m_callsArray;
  unsigned short m_availableIndicesLength;
  unsigned short *m_availableIndices;

public:
  AsyncTimer(unsigned short arrayLength = 10) {
    m_maxArrayLength = arrayLength;
    m_callsArray = new m_TimerInfo[arrayLength];
    m_availableIndicesLength = arrayLength;
    m_availableIndices = new unsigned short[arrayLength];
    for (unsigned short i = 0; i < m_availableIndicesLength; i++)
      m_availableIndices[i] = i;
  }
  ~AsyncTimer() {
    delete[] m_callsArray;
    delete[] m_availableIndices;
  }
  void setup();
  unsigned short setTimeout(void (*callback)(), unsigned long ms);
  unsigned short setInterval(void (*callback)(), unsigned long ms);
  void changeDelay(unsigned short id, unsigned long ms);
  void delay(unsigned short id, unsigned long ms);
  void reset(unsigned short id);
  void cancel(unsigned short id);
  void handle();
};

#endif
