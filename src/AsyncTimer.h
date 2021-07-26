#ifndef ASYNC_TIMER_H
#define ASYNC_TIMER_H

#include <Arduino.h>

class AsyncTimer {
private:
  union m_CB {
    void (*cbNoId)();
    void (*cbId)(unsigned short);
  };
  struct m_TimerInfo {
    unsigned short id;
    m_CB callback;
    unsigned int delayByMs;
    unsigned long timestamp;
    bool indefinite;
    bool active;

    m_TimerInfo() : active(false) {}
  };

  unsigned short m_newTimerInfo(void (*callback)(), unsigned int ms,
                                bool indefinite);
  unsigned short m_newTimerInfo(void (*callback)(unsigned short),
                                unsigned int ms, bool indefinite);

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
    for (short i = 0; i < m_availableIndicesLength; i++)
      m_availableIndices[i] = i;
  }
  ~AsyncTimer() {
    delete[] m_callsArray;
    delete[] m_availableIndices;
  }
  void setup();
  unsigned short setTimeout(void (*callback)(), unsigned int ms);
  unsigned short setTimeout(void (*callback)(unsigned short), unsigned int ms);
  unsigned short setInterval(void (*callback)(), unsigned int ms);
  unsigned short setInterval(void (*callback)(unsigned short), unsigned int ms);
  void changeDelay(unsigned short id, unsigned int ms);
  void delay(unsigned short id, unsigned int ms);
  void reset(unsigned short id);
  void cancel(unsigned short id);
  void handle();
};

#endif
