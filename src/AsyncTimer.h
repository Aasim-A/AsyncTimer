/*
  Copyright (c) 2020 Aasim-A
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
#include <functional>
#include <map>

class AsyncTimer {
private:
  struct m_TimerInfo {
    std::function<void()> callback;
    unsigned long delayByMs;
    unsigned long timestamp;
    bool indefinite;
    bool cancel;

    m_TimerInfo(std::function<void()> callback, unsigned long ms,
                unsigned long timestamp, bool indefinite)
        : callback(callback), delayByMs(ms), timestamp(timestamp),
          indefinite(indefinite), cancel(false) {}
  };

  std::map<unsigned short, m_TimerInfo> m_callsMap;
  std::vector<unsigned short> m_callsToRemove;

public:
  AsyncTimer() { m_callsToRemove.reserve(10); }
  void setup();
  unsigned short setTimeout(std::function<void()> callback, unsigned long ms);
  unsigned short setInterval(std::function<void()> callback, unsigned long ms);
  void cancel(unsigned short id);
  void handle();
};

#endif
