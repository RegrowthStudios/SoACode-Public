#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "types.h"

struct EventData {
    i32 eventID;
};

class EventManager {
public:
    typedef void(*Listener)(const EventData&);

    EventManager();
    ~EventManager();

    i32 registerEvent(std::string functionName);
    bool deregisterEvent(i32 eventID);

    bool addEventListener(i32 eventID, Listener listener);

    bool throwEvent(i32 eventID, EventData data) const;

private:
    std::unordered_map<std::string, i32> _eventIDs;
    std::vector<std::vector<Listener>> _events;
};