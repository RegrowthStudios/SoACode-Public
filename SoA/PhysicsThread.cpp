#include "stdafx.h"
#include "Errors.h"
#include "Player.h"
#include "Planet.h"
#include "PhysicsThread.h"
#include "InputManager.h"
#include "GameManager.h"
#include "OpenglManager.h"
#include "Sound.h"
#include "Options.h"
#include "MessageManager.h"

#define THREAD ThreadName::PHYSICS

// Processes a message for the physics thread
void processMessage(const Message& message) {

}

void physicsThread() {
    // Make sure only one instance of the thread is running
    static bool inUse = false;
    if (inUse) {
        pError("Physics thread was run twice!");
        std::terminate();
    }
    inUse = true;

    bool inGame = false;

    Message message;

    MessageManager* messageManager = GameManager::messageManager;

    messageManager->waitForMessage(THREAD, MessageID::DONE, message);
    if (message.id == MessageID::QUIT) {
        std::terminate();
    }

    FpsLimiter fpsLimiter;
    fpsLimiter.init(maxPhysicsFps);

    while (GameManager::gameState != GameStates::EXIT) {

        fpsLimiter.begin();
        
        GameManager::soundEngine->SetMusicVolume(soundOptions.musicVolume / 100.0f);
        GameManager::soundEngine->SetEffectVolume(soundOptions.effectVolume / 100.0f);
        GameManager::soundEngine->update();

        while (messageManager->tryDeque(THREAD, message)) {
            processMessage(message);
        }

        GameManager::inputManager->update();

        glm::dvec3 camPos;
        GLuint st;

        switch (GameManager::gameState) {

            case GameStates::PLAY:
                inGame = 1;

                GameManager::update(0.0, GameManager::player->gridPosition, NULL);
                break;
            case GameStates::MAINMENU:
                inGame = 0;
                camPos = glm::dvec3((glm::dmat4(GameManager::planet->invRotationMatrix)) * glm::dvec4(mainMenuCamera.position(), 1.0));
                GameManager::planet->rotationUpdate();
                GameManager::updatePlanet(camPos, 10);
                break;
            case GameStates::ZOOMINGIN:
            case GameStates::ZOOMINGOUT:
                inGame = 0;
                mainMenuCamera.update();
                camPos = glm::dvec3((glm::dmat4(GameManager::planet->invRotationMatrix)) * glm::dvec4(mainMenuCamera.position(), 1.0));
                GameManager::updatePlanet(camPos, 10);
                break;
            //case GameStates::WORLDEDITOR:
            //    worldEditorLoop(); //only return from this when we are done
            //    break;
        }

        physicsFps = fpsLimiter.end();
    }

    if (inGame) {
        GameManager::onQuit();
    }

    if (GameManager::planet) delete GameManager::planet;

    inUse = false;
}