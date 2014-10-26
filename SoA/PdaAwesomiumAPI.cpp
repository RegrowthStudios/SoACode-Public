#include "stdafx.h"
#include "PdaAwesomiumAPI.h"

#include "GamePlayScreen.h"


void PdaAwesomiumAPI::init(Awesomium::JSObject* interfaceObject, IGameScreen* ownerScreen) {

    // Helper macro for adding functions
#define ADDFUNC(a) addFunction(""#a"", &PdaAwesomiumAPI::##a)

    // Set up the interface object so we can talk to the JS
    _interfaceObject = interfaceObject;

    // Set up our screen handle so we can talk to the game
    setOwnerScreen(ownerScreen);

  
}

void PdaAwesomiumAPI::setOwnerScreen(IGameScreen* ownerScreen) {
    _ownerScreen = static_cast<GamePlayScreen*>(ownerScreen);
}
