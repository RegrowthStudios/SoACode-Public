#include "stdafx.h"
#include "MessageManager.h"


MessageManager::MessageManager()
{
}


MessageManager::~MessageManager()
{
}


void MessageManager::enqueue(ThreadName thread, Message message) {
    if (thread == ThreadName::PHYSICS) {
        physicsToRender.enqueue(message);
    } else { // ThreadName::RENDERING
        renderToPhysics.enqueue(message);
    }
}

bool MessageManager::tryDeque(ThreadName thread, Message& result) {
    if (thread == ThreadName::PHYSICS) {
        return renderToPhysics.try_dequeue(result);
    } else { // ThreadName::RENDERING
        return physicsToRender.try_dequeue(result);
    }
}

void MessageManager::waitForMessage(ThreadName thread, MessageID idToWait, Message& result) {
    if (thread == ThreadName::PHYSICS) {
        while (true){
            if (renderToPhysics.try_dequeue(result)){
                if (result.id == idToWait || result.id == MessageID::QUIT){
                    return;
                }
            }
        }
    } else { // ThreadName::RENDERING
        while (true){
            if (physicsToRender.try_dequeue(result)){
                if (result.id == idToWait || result.id == MessageID::QUIT){
                    return;
                }
            }
        }
    }

}