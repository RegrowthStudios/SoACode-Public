#include "stdafx.h"
#include "DevConsole.h"

DevConsole DevConsole::m_instance;

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
    auto it = m_commandListeners.find(command);
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

bool DevConsole::write(nString s) {
    
    // Remove leading `
    while (s.size() && s.front() == '`') {
        s.erase(0, 1);
    }
    if (s.empty()) return false;
    // TODO(Ben): Concern about thread safety.
    // Ringbuffer push invalidates data... have to copy
    nString sCopy = s;
    m_history.push(sCopy);
    // Broadcast to listeners listening for any event
    for (auto& eb : m_anyCommandListeners) {
        eb.function(eb.metaData, s);
    }
    // Broadcast to specific listeners.
    nString command = getFirstToken(s);
    auto it = m_commandListeners.find(command);
    if (it == m_commandListeners.end()) return false;
    for (auto& eb : it->second) {
        eb.function(eb.metaData, s);
    }
    return true;
}

void DevConsole::toggleFocus() {
    m_isFocused = !m_isFocused;
    if (m_isFocused) {
        // Enable input
        vui::InputDispatcher::key.onKeyDown += makeDelegate(*this, &DevConsole::onKeyDown);
        vui::InputDispatcher::key.onText += makeDelegate(*this, &DevConsole::onTextInput);
    } else {
        // Disable input
        vui::InputDispatcher::key.onKeyDown -= makeDelegate(*this, &DevConsole::onKeyDown);
        vui::InputDispatcher::key.onText -= makeDelegate(*this, &DevConsole::onTextInput);
    }
}

void DevConsole::setFocus(bool focus) {
    if (focus != m_isFocused) {
        toggleFocus();  
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
            return input.substr(start, i - start);
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

void DevConsole::onKeyDown(Sender s VORB_UNUSED, const vui::KeyEvent& ev) {
    // TODO(Ben): Unicode is gonna be a nightmare
    if (ev.keyCode == VKEY_RETURN || ev.keyCode == VKEY_RETURN2) {
        write(m_currentLine);
        m_currentLine = "";
    } else if (ev.keyCode == VKEY_BACKSPACE || ev.keyCode == VKEY_DELETE) {
        if (m_currentLine.size()) m_currentLine.pop_back();
    }
}

void DevConsole::onTextInput(Sender s VORB_UNUSED, const vui::TextEvent& ev) {
    const char* t = ev.text;
    while (*t != '\0') {
        m_currentLine += (*t);
        t++;
    }
}