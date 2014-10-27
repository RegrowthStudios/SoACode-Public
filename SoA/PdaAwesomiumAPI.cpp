#include "stdafx.h"
#include "PdaAwesomiumAPI.h"

#include "GamePlayScreen.h"
#include "Player.h"


void PdaAwesomiumAPI::init(Awesomium::JSObject* interfaceObject, IGameScreen* ownerScreen) {

    // Helper macro for adding functions
    #define ADDFUNC(a) addFunction(""#a"", &PdaAwesomiumAPI::##a)

    // Set up the interface object so we can talk to the JS
    _interfaceObject = interfaceObject;

    // Set up our screen handle so we can talk to the game
    setOwnerScreen(ownerScreen);

    // Register functions here
    ADDFUNC(getInventory);
    ADDFUNC(print);
  
}

void PdaAwesomiumAPI::setOwnerScreen(IGameScreen* ownerScreen) {
    _ownerScreen = static_cast<GamePlayScreen*>(ownerScreen);
}

Awesomium::JSValue PdaAwesomiumAPI::getInventory(const Awesomium::JSArray& args) {
 
    // Grab a handle to the inventory
    const std::vector<Item*>& inventory = _ownerScreen->_player->inventory;

    Awesomium::JSArray entries;
    for (int i = 0; i < inventory.size(); i++) {
        entries.Push(Awesomium::WSLit(inventory[i]->name.c_str()));
        entries.Push(Awesomium::JSValue(inventory[i]->count));
    }
    return Awesomium::JSValue(entries);
}