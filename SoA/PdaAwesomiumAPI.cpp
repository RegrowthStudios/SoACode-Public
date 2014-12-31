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
    ADDFUNC(selectItem);
  
}

void PdaAwesomiumAPI::setOwnerScreen(IGameScreen* ownerScreen) {
    _ownerScreen = static_cast<GamePlayScreen*>(ownerScreen);
}

Awesomium::JSValue PdaAwesomiumAPI::getInventory(const Awesomium::JSArray& args) {
 
    // Grab a handle to the inventory
    const std::vector<Item*>& inventory = _ownerScreen->m_player->inventory;

    Awesomium::JSArray entries;
    for (int i = 0; i < inventory.size(); i++) {
        entries.Push(Awesomium::WSLit(inventory[i]->name.c_str()));
        entries.Push(Awesomium::JSValue(inventory[i]->count));
    }
    return Awesomium::JSValue(entries);
}

#define HAND_LEFT 0
#define HAND_RIGHT 1

void PdaAwesomiumAPI::selectItem(const Awesomium::JSArray& args) {
    
    // Grab the arguments
    int hand = args[0].ToInteger();
    nString name = Awesomium::ToString(args[1].ToString());

    std::cout << "SELECT ITEM: " << hand << " " << name;

    // Search player inventory
    Player* player = _ownerScreen->m_player;
    // TODO(Ben): Not linear search
    for (int i = 0; i < player->inventory.size(); i++) {
        if (player->inventory[i]->name == name) {
            // Check which hand was clicked
            if (hand == HAND_LEFT) {
                if (player->leftEquippedItem == player->inventory[i]) {
                    // If the item is already equipped, unequip it.
                    player->leftEquippedItem = nullptr;
                } else {
                    // Equip the item
                    player->leftEquippedItem = player->inventory[i];
                }
            } else if (hand == HAND_RIGHT) {
                if (player->rightEquippedItem == player->inventory[i]) {
                    // If the item is already equipped, unequip it.
                    player->rightEquippedItem = nullptr;
                } else {
                    // Equip the item
                    player->rightEquippedItem = player->inventory[i];
                }
            }
            break;
        }
    }
}