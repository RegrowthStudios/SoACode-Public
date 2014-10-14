#pragma once

#include <unordered_map>
#include "readerwriterqueue.h"

enum class MessageID {
    NONE,
    DONE,
    QUIT,
    NEW_PLANET
};

struct Message
{
    Message() : id(MessageID::NONE), data(NULL){}
    Message(MessageID i, void *d) : id(i), data(d) {}
    MessageID id;
    void *data;
};

enum class ThreadName {
    PHYSICS,
    RENDERING
};

class MessageManager
{
public:
    MessageManager();
    ~MessageManager();

    // Enqueues a messages
    // @param thread: The current thread name
    // @param message: The message to enqueue
    void enqueue(ThreadName thread, Message message);

    // Tries to dequeue a message
    // @param thread: The current thread name
    // @param result: The resulting message to store
    // returns true if a message was dequeued, false otherwise
    bool tryDeque(ThreadName thread, Message& result);

    // Waits for a specific message, discarding all other messages
    // it will return when idToWait is found, or MessageID::QUIT is found
    // @param thread: The current thread name
    // @param idToWait: The message ID to wait for
    // @param result: The resulting message to store
    void waitForMessage(ThreadName thread, MessageID idToWait, Message& result);

    // 1 way lock-free queues.
    moodycamel::ReaderWriterQueue <Message> physicsToRender;
    moodycamel::ReaderWriterQueue <Message> renderToPhysics;
};

