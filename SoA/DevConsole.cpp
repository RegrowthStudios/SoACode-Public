#include "stdafx.h"
#include "DevConsole.h"

DevConsole::DevConsole(i32 maxHistory) :
_commands(maxHistory),
_commandListeners() {}

void DevConsole::addListener(FuncNewCommand f, void* meta) {
    EventBinding eb = { f, meta };
    _commandListeners.emplace_back(eb);
}
void DevConsole::removeListener(FuncNewCommand f) {
    auto foundListener = std::find(_commandListeners.begin(), _commandListeners.end(), f);
    if (foundListener != _commandListeners.end()) {
        _commandListeners.erase(foundListener, foundListener + 1);
    }
}

void DevConsole::write(const nString& s) {
    _commands.push(s);
    EventBinding eb;
    for (i32 i = 0; i < _commandListeners.size(); i++) {
        eb = _commandListeners[i];
        eb.function(eb.metaData, s);
    }
}

const nString& DevConsole::getCommand(const i32& index) {
    return _commands.at(index);
}
