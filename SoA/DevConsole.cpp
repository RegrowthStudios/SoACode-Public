#include "stdafx.h"
#include "DevConsole.h"

DevConsole::DevConsole(i32 maxHistory) :
m_commands(maxHistory),
m_commandListeners() {}

void DevConsole::addListener(FuncNewCommand f, void* meta) {
    EventBinding eb = { f, meta };
    m_commandListeners.emplace_back(eb);
}
void DevConsole::removeListener(FuncNewCommand f) {
    auto foundListener = std::find(m_commandListeners.begin(), m_commandListeners.end(), f);
    if (foundListener != m_commandListeners.end()) {
        m_commandListeners.erase(foundListener, foundListener + 1);
    }
}

void DevConsole::write(const nString& s) {
    m_commands.push(s);
    EventBinding eb;
    for (size_t i = 0; i < m_commandListeners.size(); i++) {
        eb = m_commandListeners[i];
        eb.function(eb.metaData, s);
    }
}

const nString& DevConsole::getCommand(const i32& index) {
    return m_commands.at(index);
}
