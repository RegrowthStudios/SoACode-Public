#include "stdafx.h"
#include "DevConsole.h"

void DevConsole::init(int maxHistory) {
    // TODO(Ben): Add setCapacity
    new (&m_history) CommandRing(maxHistory);
}

void DevConsole::addListener(const nString& command, FuncNewCommand f, void* meta) {
    EventBinding eb = { f, meta };
    m_commandListeners[command].emplace_back(eb);
}

// Adds listener for any command
void DevConsole::addListener(FuncNewCommand f, void* meta) {
    EventBinding eb = { f, meta };
    m_anyCommandListeners.emplace_back(eb);
}

bool DevConsole::removeListener(const nString& command, FuncNewCommand f) {
    auto& it = m_commandListeners.find(command);
    if (it == m_commandListeners.end()) return false;
    auto& listeners = it->second;
    auto foundListener = std::find(listeners.begin(), listeners.end(), f);
    if (foundListener != listeners.end()) {
        listeners.erase(foundListener, foundListener + 1);
        return true;
    }
    return false;
}

bool DevConsole::removeListener(FuncNewCommand f) {
    auto foundListener = std::find(m_anyCommandListeners.begin(), m_anyCommandListeners.end(), f);
    if (foundListener != m_anyCommandListeners.end()) {
        m_anyCommandListeners.erase(foundListener, foundListener + 1);
        return true;
    }
    return false;
}

void DevConsole::addCommand(const nString& s) {
    m_commandListeners.emplace(s, std::vector<EventBinding>());
}

bool DevConsole::write(const nString& s) {
    // TODO(Ben): Concern about thread safety.
    m_history.push(s);
    m_updateVersion++;
    // Broadcast to listeners listening for any event
    for (auto& eb : m_anyCommandListeners) {
        eb.function(eb.metaData, s);
    }
    // Broadcast to specific listeners.
    nString command = getFirstToken(s);
    auto& it = m_commandListeners.find(command);
    if (it == m_commandListeners.end()) return false;
    for (auto& eb : it->second) {
        eb.function(eb.metaData, s);
    }
    return true;
}

void DevConsole::setFocus(bool focus) {
    if (focus != m_isFocused) {
        m_isFocused = focus;
        if (m_isFocused) {
            // Enable input
            vui::InputDispatcher::key.onKeyDown += makeDelegate(*this, &DevConsole::onKeyDown);
        } else {
            // Disable input
            vui::InputDispatcher::key.onKeyDown -= makeDelegate(*this, &DevConsole::onKeyDown);
        }
    }
}

const nString& DevConsole::getHistory(const i32& index) {
    return m_history.at(index);
}

nString DevConsole::getFirstToken(nString input) {
    size_t i = 0;
    while (i < input.size()) {
        while (input[i] == ' ') i++;
        size_t start = i;
        while (input[i] != ' ' && i < input.size()) i++;
        if (i - start > 0) {
            return input.substr(i, i - start);
        }
    }
    return "";
}

void DevConsole::tokenize(nString& input, OUT std::vector<nString>& tokens) {
    // TODO(Ben): Pass in delimiters
    size_t i = 0;
    while (i < input.size()) {
        while (input[i] == ' ') i++;
        size_t start = i;
        while (input[i] != ' ' && i < input.size()) i++;
        if (i - start > 0) {
            tokens.emplace_back(input.substr(i, i - start));
        }
    }
}

void DevConsole::onKeyDown(const vui::KeyEvent& ev) {
    // TODO(Ben): Unicode is gonna be a nightmare
    if (ev.keyCode == VKEY_RETURN || ev.keyCode == VKEY_RETURN2) {
        write(m_currentLine);
        m_currentLine = "";
    } else if (ev.keyCode == VKEY_BACKSPACE || ev.keyCode == VKEY_DELETE) {
        if (m_currentLine.size()) m_currentLine.pop_back();
    } else {
        m_currentLine += ev.keyCode;
    }
}
