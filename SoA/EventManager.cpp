#include "stdafx.h"
#include "EventManager.h"

#include <stdio.h>
#include <algorithm>

EventManager::EventManager() {}

EventManager::~EventManager() {}

i32 EventManager::registerEvent(std::string functionName) {
    if(_eventIDs.find(functionName) != _eventIDs.end()) return -1;

    i32 id = _events.size();
    _eventIDs[functionName] = id;

    _events.push_back(std::vector<Listener>());

    return id;
}

bool EventManager::deregisterEvent(i32 eventID) {
    if(eventID < 0 || eventID > _events.size()) return false;

    _events.erase(_events.begin() + eventID);

    return true;
}

bool EventManager::deregisterEvent(std::string eventName) {
    auto iter = _eventIDs.find(eventName);
    if(iter == _eventIDs.end()) return false;
    
    _eventIDs.erase(iter);

    return true;
}

bool EventManager::addEventListener(i32 eventID, Listener listener) {
    if(eventID < 0 || eventID >= _events.size()) return false;

    _events[eventID].push_back(listener);

    return true;
}

bool EventManager::addEventListener(std::string eventName, Listener listener) {
    auto iter = _eventIDs.find(eventName);
    if(iter == _eventIDs.end()) return false;
   
    _events[iter->second].push_back(listener);
    
    return true;
}

bool EventManager::removeEventListener(i32 eventID, Listener listener) {
    if(eventID < 0 || eventID >= _events.size()) return false;
    
    for(int i = _events[eventID].size() - 1; i >= 0; i--) {
        if(_events[eventID][i] == listener) {
            _events[eventID].erase(_events[eventID].begin() + i);
            return true;
        }
    }

    return false;
}

bool EventManager::removeEventListener(std::string eventName, Listener listener) {
    auto iter = _eventIDs.find(eventName);
    if(iter == _eventIDs.end()) return false;
    
    bool found = false;
    for(int i = _events[iter->second].size() - 1; i >= 0; i--) {
        if(_events[iter->second][i] == listener) {
            _events[iter->second].erase(_events[iter->second].begin() + i);
            found = true;
        }
    }
    
    return found;
}

bool EventManager::throwEvent(EventData* data) const {
    if(data->eventID < 0 || data->eventID >= _events.size()) return false;

    for(Listener listener : _events[data->eventID]) {
        listener(data);
    }

    return true;
}