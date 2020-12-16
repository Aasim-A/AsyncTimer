#include "AsyncTimer.h"

void AsyncTimer::setup() { srand((unsigned)time(0)); }

unsigned short AsyncTimer::setTimeout(std::function<void()> callback,
                                      unsigned long ms) {
  unsigned short id = rand();
  m_TimerInfo timerInfo(callback, ms, millis(), false);
  m_callsMap.insert(std::make_pair(id, timerInfo));
  return id;
}

unsigned short AsyncTimer::setInterval(std::function<void()> callback,
                                       unsigned long ms) {
  unsigned short id = rand();
  m_TimerInfo timerInfo(callback, ms, millis(), true);
  m_callsMap.insert(std::make_pair(id, timerInfo));
  return id;
}

void AsyncTimer::cancel(unsigned short id) {
  auto it = m_callsMap.find(id);
  if (it != m_callsMap.end()) {
    it->second.cancel = true;
  }
}

void AsyncTimer::handle() {
  if (m_callsMap.empty())
    return;

  for (auto &it : m_callsMap) {
    if (it.second.cancel) {
      m_callsToRemove.push_back(it.first);
      continue;
    }

    if (millis() - it.second.timestamp > it.second.delayByMs) {
      it.second.callback();
      if (it.second.indefinite) {
        it.second.timestamp = millis();
      } else
        m_callsToRemove.push_back(it.first);
    }
  }

  if (!m_callsToRemove.empty()) {
    for (const unsigned short &id : m_callsToRemove) {
      m_callsMap.erase(id);
    }
    m_callsToRemove.clear();
  }
}