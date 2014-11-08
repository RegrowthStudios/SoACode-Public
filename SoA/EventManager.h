#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "types.h"

class EventData {
public:
    EventData(i32 eventID): eventID(eventID) {}
    i32 eventID;
};

class EventManager {
public:
    typedef void(*Listener)(EventData*);

    EventManager();
    ~EventManager();

    i32 registerEvent(std::string eventName);
    bool deregisterEvent(i32 eventID);
    bool deregisterEvent(std::string eventName);

    bool addEventListener(i32 eventID, Listener listener);
    bool addEventListener(std::string eventName, Listener listener);
    bool removeEventListener(i32 eventID, Listener listener);
    bool removeEventListener(std::string eventName, Listener listener);

    bool throwEvent(EventData* data) const;

private:
    std::unordered_map<std::string, i32> _eventIDs;
    std::vector<std::vector<Listener>> _events;
};