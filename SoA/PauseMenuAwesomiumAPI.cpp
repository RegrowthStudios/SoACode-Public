#include "stdafx.h"
#include "PauseMenuAwesomiumAPI.h"

#include "GamePlayScreen.h"

void PauseMenuAwesomiumAPI::init(Awesomium::JSObject* interfaceObject, IGameScreen* ownerScreen) {

    // Helper macro for adding functions
    #define ADDFUNC(a) addFunction(""#a"", &PauseMenuAwesomiumAPI::##a)

    // Set up the interface object so we can talk to the JS
    _interfaceObject = interfaceObject;

    // Set up our screen handle so we can talk to the game
    setOwnerScreen(ownerScreen);
}

void PauseMenuAwesomiumAPI::setOwnerScreen(IGameScreen* ownerScreen) {
    _ownerScreen = static_cast<GamePlayScreen*>(ownerScreen);
}
