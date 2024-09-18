#include "AsyncTimer.h"

void AsyncTimer::setup() {}

unsigned short AsyncTimer::m_generateId() {
  unsigned short id = rand() + 1;

  for (unsigned short i = 0; i < m_maxArrayLength; i++) {
    if (m_callsArray[i].id == id)
      return m_generateId();
  }

  return id;
}

unsigned short AsyncTimer::m_newTimerInfo(Callback &callback, unsigned long ms,
                                          bool indefinite) {
  if (m_availableIndicesLength == 0 || m_arrayLength == m_maxArrayLength)
    return 0;

  unsigned short id = m_generateId();
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

void AsyncTimer::m_cancelEntry(unsigned short index) {
  m_callsArray[index].active = false;
  m_arrayLength--;
  m_availableIndices[m_availableIndicesLength] = index;
  m_availableIndicesLength++;
}

unsigned short AsyncTimer::_setTimeout(Callback &callback, unsigned long ms) {
  return m_newTimerInfo(callback, ms, false);
}

unsigned short AsyncTimer::_setInterval(Callback &callback, unsigned long ms) {
  return m_newTimerInfo(callback, ms, true);
}

unsigned long AsyncTimer::getRemaining(unsigned short id) {
  unsigned long now = millis();
  for (unsigned short i = 0; i < m_maxArrayLength; i++) {
    if (m_callsArray[i].id == id && m_callsArray[i].active) {
      unsigned long tsDelay =
          m_callsArray[i].timestamp + m_callsArray[i].delayByMs;
      // now can be bigger than timestamp + delayByMs because the code so far
      // has been executing synchronously
      if (now < tsDelay)
        return tsDelay - now;
      break;
    }
  }

  return 0;
}

void AsyncTimer::changeDelay(unsigned short id, unsigned long ms) {
  for (unsigned short i = 0; i < m_maxArrayLength; i++) {
    if (m_callsArray[i].id == id)
      m_callsArray[i].delayByMs = ms;
  }
}

void AsyncTimer::delay(unsigned short id, unsigned long ms) {
  for (unsigned short i = 0; i < m_maxArrayLength; i++) {
    if (m_callsArray[i].id == id)
      m_callsArray[i].timestamp += ms;
  }
}

void AsyncTimer::reset(unsigned short id) {
  for (unsigned short i = 0; i < m_maxArrayLength; i++) {
    if (m_callsArray[i].id == id)
      m_callsArray[i].timestamp = millis();
  }
}

void AsyncTimer::cancel(unsigned short id) {
  for (unsigned short i = 0; i < m_maxArrayLength; i++) {
    if (m_callsArray[i].id == id && m_callsArray[i].active)
      m_cancelEntry(i);
  }
}

void AsyncTimer::cancelAll(bool includeIntervals) {
  for (unsigned short i = 0; i < m_maxArrayLength; i++) {
    if (!includeIntervals) {
      if (!m_callsArray[i].indefinite)
        m_cancelEntry(i);
    } else
      m_cancelEntry(i);
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
        Callback &cb = m_callsArray[i].callback;
        m_cancelEntry(i);
        cb();
      }
    }
  }
}
