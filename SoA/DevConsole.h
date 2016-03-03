#pragma once
#include <Vorb/RingBuffer.hpp>
#include <Vorb/ui/InputDispatcher.h>

typedef vorb::ring_buffer<nString> CommandRing;
typedef void(*FuncNewCommand)(void* metadata, const nString& command);

class DevConsole {
public:
    DevConsole() : m_history(50) {};
    static DevConsole& getInstance() { return m_instance; }

    void init(int maxHistory);

    // Adds listener for a specific command
    void addListener(const nString& command, FuncNewCommand f, void* meta);
    // Adds listener for any command
    void addListener(FuncNewCommand f, void* meta);
    // Removes listener for specific command
    bool removeListener(const nString& command, FuncNewCommand f);
    // Removes listener for any command
    bool removeListener(FuncNewCommand f);

    void addCommand(const nString& s);
    bool write(nString s);

    void toggleFocus();
    void setFocus(bool focus);
    const bool& isFocused() { return m_isFocused; }

    const nString& getHistory(const i32& index);
    const nString& getCurrentLine() { return m_currentLine; }

    // Utilities for tokenizing strings
    static nString getFirstToken(nString input);
    static void tokenize(nString& input, OUT std::vector<nString>& tokens);
private:
    class EventBinding {
    public:
        FuncNewCommand function;
        void* metaData;

        bool operator== (const FuncNewCommand& f) {
            return function == f;
        }
    };

    void onKeyDown(Sender s, const vui::KeyEvent& ev);
    void onTextInput(Sender s, const vui::TextEvent& ev);

    bool m_isFocused = false;
    nString m_currentLine = "";
    CommandRing m_history;
    std::unordered_map<nString, std::vector<EventBinding>> m_commandListeners; ///< For specific commands only
    std::vector<EventBinding> m_anyCommandListeners;

    static DevConsole m_instance; ///< Singleton
};
