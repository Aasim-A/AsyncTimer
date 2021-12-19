#include "AsyncTimer.h"

void AsyncTimer::setup() { srand(millis()); }

unsigned short AsyncTimer::m_newTimerInfo(void (*callback)(), unsigned long ms,
                                          bool indefinite) {
  if (m_availableIndicesLength == 0 || m_arrayLength == m_maxArrayLength) {
    return 0;
  }
  unsigned short id = rand() + 1;
  m_availableIndicesLength--;
  unsigned short availableIndex = m_availableIndices[m_availableIndicesLength];
  m_callsArray[availableIndex].id = id;
  m_callsArray[availableIndex].callback = callback;
  m_callsArray[availableIndex].delayByMs = ms;
  m_callsArray[availableIndex].timestamp = millis();
  m_callsArray[availableIndex].indefinite = indefinite;
  m_callsArray[availableIndex].active = true;
  m_arrayLength++;

  return id;
}

unsigned short AsyncTimer::setTimeout(void (*callback)(), unsigned long ms) {
  return m_newTimerInfo(callback, ms, false);
}

unsigned short AsyncTimer::setInterval(void (*callback)(), unsigned long ms) {
  return m_newTimerInfo(callback, ms, true);
}

void AsyncTimer::changeDelay(unsigned short id, unsigned long ms) {
  for (unsigned short i = 0; i < m_maxArrayLength; i++)
    if (m_callsArray[i].id == id)
      m_callsArray[i].delayByMs = ms;
}

void AsyncTimer::delay(unsigned short id, unsigned long ms) {
  for (unsigned short i = 0; i < m_maxArrayLength; i++)
    if (m_callsArray[i].id == id)
      m_callsArray[i].timestamp += ms;
}

void AsyncTimer::reset(unsigned short id) {
  for (unsigned short i = 0; i < m_maxArrayLength; i++)
    if (m_callsArray[i].id == id)
      m_callsArray[i].timestamp = millis();
}

void AsyncTimer::cancel(unsigned short id) {
  for (unsigned short i = 0; i < m_maxArrayLength; i++) {
    if (m_callsArray[i].id == id && m_callsArray[i].active) {
      m_callsArray[i].active = false;
      m_arrayLength--;
      m_availableIndices[m_availableIndicesLength] = i;
      m_availableIndicesLength++;
    }
  }
}

void AsyncTimer::handle() {
  if (m_arrayLength == 0)
    return;

  for (unsigned short i = 0; i < m_maxArrayLength; i++) {
    unsigned long timestamp = millis();
    if (!m_callsArray[i].active || m_callsArray[i].timestamp > timestamp)
      continue;
    if (timestamp - m_callsArray[i].timestamp >= m_callsArray[i].delayByMs) {
      if (m_callsArray[i].indefinite) {
        m_callsArray[i].timestamp = timestamp;
        m_callsArray[i].callback();
      } else {
        void (*cb)() = m_callsArray[i].callback;
        m_callsArray[i].active = false;
        m_callsArray[i].callback = nullptr;
        m_arrayLength--;
        m_availableIndices[m_availableIndicesLength] = i;
        m_availableIndicesLength++;
        cb();
      }
    }
  }
}
