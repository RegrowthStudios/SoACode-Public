#include "stdafx.h"
#include "EventManager.h"

EventManager::EventManager():
_events(),
_eventIDs()
{}

EventManager::~EventManager() {}

i32 EventManager::registerEvent(std::string functionName) {
    if(_eventIDs.find(functionName) != _eventIDs.end()) return -1;

    i32 id = _events.size();
    _eventIDs[functionName] = id;

    _events.push_back(std::vector<Listener>());
}

bool EventManager::deregisterEvent(i32 eventID) {
    if(eventID < 0 || eventID > _events.size()) return false;

    _events.erase(_events.begin() + eventID);

    return true;
}

bool EventManager::addEventListener(i32 eventID, Listener listener) {
    if(eventID < 0 || eventID >= _events.size()) return false;

    for(auto iter = _events[eventID].begin(); iter != _events[eventID].end(); iter++) {
        if(*iter == listener) {
            _events[eventID].erase(iter);
            return true;
        }
    }

    return false;
}

bool EventManager::throwEvent(i32 eventID, EventData data) const {
    if(eventID < 0 || eventID >= _events.size()) return false;

    for(auto iter = _events[eventID].begin(); iter != _events[eventID].end(); iter++) {
        (*iter)(data);
    }
    return true;
}