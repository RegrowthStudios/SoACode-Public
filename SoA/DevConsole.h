#pragma once
#include <Vorb\RingBuffer.hpp>

typedef vorb::ring_buffer<nString> CommandRing;
typedef void(*FuncNewCommand)(void* metadata, const nString& command);


class DevConsole {
public:
    DevConsole(i32 maxHistory);

    void addListener(FuncNewCommand f, void* meta);
    void removeListener(FuncNewCommand f);

    void write(const nString& s);

    const nString& getCommand(const i32& index);
private:
    class EventBinding {
    public:
        FuncNewCommand function;
        void* metaData;

        bool operator== (const FuncNewCommand& f) {
            return function == f;
        }
    };

    CommandRing _commands;
    std::vector<EventBinding> _commandListeners;
};