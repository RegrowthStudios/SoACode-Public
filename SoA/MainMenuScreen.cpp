#include "stdafx.h"
#include "MainMenuScreen.h"
#include "ChunkManager.h"
#include "Errors.h"
#include "Player.h"
#include "Planet.h"
#include "InputManager.h"
#include "GameManager.h"
#include "OpenglManager.h"
#include "Sound.h"
#include "Options.h"
#include "MessageManager.h"
#include "VoxelEditor.h"

#define THREAD ThreadName::PHYSICS

CTOR_APP_SCREEN_DEF(MainMenuScreen, App) ,
_updateThread(nullptr),
_threadRunning(true){
    // Empty
}

void MainMenuScreen::onEntry(const GameTime& gameTime) {
    _updateThread = new thread(&MainMenuScreen::updateThreadFunc, this);

}

void MainMenuScreen::onExit(const GameTime& gameTime) {
    _threadRunning = false;
    _updateThread->join();
}

void MainMenuScreen::updateThreadFunc() {

    _threadRunning = true;

    Message message;

    MessageManager* messageManager = GameManager::messageManager;

    messageManager->waitForMessage(THREAD, MessageID::DONE, message);
    if (message.id == MessageID::QUIT) {
        std::terminate();
    }

    FpsLimiter fpsLimiter;
    fpsLimiter.init(maxPhysicsFps);

    while (_threadRunning) {

        fpsLimiter.begin();

        GameManager::soundEngine->SetMusicVolume(soundOptions.musicVolume / 100.0f);
        GameManager::soundEngine->SetEffectVolume(soundOptions.effectVolume / 100.0f);
        GameManager::soundEngine->update();

        while (messageManager->tryDeque(THREAD, message)) {
            // Process the message
            switch (message.id) {
                case MessageID::NEW_PLANET:
                    messageManager->enqueue(THREAD, Message(MessageID::NEW_PLANET, NULL));
                    messageManager->enqueue(THREAD, Message(MessageID::DONE, NULL));
                    messageManager->waitForMessage(THREAD, MessageID::DONE, message);
                    break;
            }
        }

        GameManager::inputManager->update();

        glm::dvec3 camPos;

        camPos = glm::dvec3((glm::dmat4(GameManager::planet->invRotationMatrix)) * glm::dvec4(mainMenuCamera.position(), 1.0));
        GameManager::planet->rotationUpdate();
        GameManager::updatePlanet(camPos, 10);
        
        physicsFps = fpsLimiter.end();
    }
}