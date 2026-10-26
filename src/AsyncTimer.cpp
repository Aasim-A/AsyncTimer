#include "AsyncTimer.h"

void AsyncTimer::setup() {}

unsigned short AsyncTimer::m_generateId() {
  // Called only when a slot is free, so a nonzero unused ID always exists.
  for (;;) {
    if (++m_nextId == 0)
      ++m_nextId;
    bool inUse = false;
    for (unsigned short i = 0; i < m_maxArrayLength; i++) {
      if (m_callsArray[i].active && m_callsArray[i].id == m_nextId) {
        inUse = true;
        break;
      }
    }
    if (!inUse)
      return m_nextId;
  }
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
  m_callsArray[availableIndex].postponedByMs = 0;
  m_callsArray[availableIndex].indefinite = indefinite;
  m_callsArray[availableIndex].active = true;
  m_arrayLength++;

  return id;
}

void AsyncTimer::m_cancelEntry(unsigned short index) {
  m_callsArray[index].active = false;
  m_callsArray[index].callback.clear();
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
    if (m_callsArray[i].active && m_callsArray[i].id == id) {
      unsigned long elapsed = now - m_callsArray[i].timestamp;
      if (elapsed < m_callsArray[i].postponedByMs)
        return m_callsArray[i].postponedByMs - elapsed +
               m_callsArray[i].delayByMs;
      elapsed -= m_callsArray[i].postponedByMs;
      if (elapsed < m_callsArray[i].delayByMs)
        return m_callsArray[i].delayByMs - elapsed;
      break;
    }
  }

  return 0;
}

void AsyncTimer::changeDelay(unsigned short id, unsigned long ms) {
  for (unsigned short i = 0; i < m_maxArrayLength; i++) {
    if (m_callsArray[i].active && m_callsArray[i].id == id)
      m_callsArray[i].delayByMs = ms;
  }
}

void AsyncTimer::delay(unsigned short id, unsigned long ms) {
  for (unsigned short i = 0; i < m_maxArrayLength; i++) {
    if (m_callsArray[i].active && m_callsArray[i].id == id)
      m_callsArray[i].postponedByMs += ms;
  }
}

void AsyncTimer::reset(unsigned short id) {
  for (unsigned short i = 0; i < m_maxArrayLength; i++) {
    if (m_callsArray[i].active && m_callsArray[i].id == id) {
      m_callsArray[i].timestamp = millis();
      m_callsArray[i].postponedByMs = 0;
    }
  }
}

void AsyncTimer::cancel(unsigned short id) {
  for (unsigned short i = 0; i < m_maxArrayLength; i++) {
    if (m_callsArray[i].active && m_callsArray[i].id == id)
      m_cancelEntry(i);
  }
}

void AsyncTimer::cancelAll(bool includeIntervals) {
  for (unsigned short i = 0; i < m_maxArrayLength; i++) {
    if (m_callsArray[i].active &&
        (includeIntervals || !m_callsArray[i].indefinite))
      m_cancelEntry(i);
  }
}

void AsyncTimer::handle() {
  if (m_arrayLength == 0)
    return;

  for (unsigned short i = 0; i < m_maxArrayLength; i++) {
    unsigned long timestamp = millis();
    if (!m_callsArray[i].active || m_callsArray[i].callback.empty())
      continue;
    // Unsigned subtraction keeps elapsed time correct when millis() wraps.
    unsigned long elapsed = timestamp - m_callsArray[i].timestamp;
    if (elapsed < m_callsArray[i].postponedByMs)
      continue;
    elapsed -= m_callsArray[i].postponedByMs;
    if (elapsed >= m_callsArray[i].delayByMs) {
      // A callback may cancel itself or reuse its slot. Keep its captures alive
      // until it returns, and leave the slot empty during nested handle() calls.
      Callback callback;
      callback = m_callsArray[i].callback;
      struct RestoreCallback {
        m_TimerInfo &entry;
        Callback &callback;
        unsigned short id;

        ~RestoreCallback() {
          if (entry.active && entry.id == id && entry.callback.empty())
            entry.callback = callback;
        }
      } restore{m_callsArray[i], callback, m_callsArray[i].id};

      if (m_callsArray[i].indefinite) {
        m_callsArray[i].timestamp = timestamp;
        m_callsArray[i].postponedByMs = 0;
      } else {
        m_cancelEntry(i);
      }
      callback();
    }
  }
}
