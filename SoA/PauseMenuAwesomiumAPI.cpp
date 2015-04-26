#include "stdafx.h"
#include "PauseMenuAwesomiumAPI.h"

#include <Vorb/ui/MainGame.h>

#include "App.h"
#include "GameplayScreen.h"

void PauseMenuAwesomiumAPI::init(Awesomium::WebView* webView, vui::CustomJSMethodHandler<PauseMenuAwesomiumAPI>* methodHandler,
                                 vui::IGameScreen* ownerScreen, const Awesomium::JSValue& window) {
    m_window = window;
    // Helper macro for adding functions
    #define ADDFUNC(a) addFunction(""#a"", &PauseMenuAwesomiumAPI::##a)

    // Set up the interface object so we can talk to the JS
    m_methodHandler = methodHandler;
    m_webView = webView;

    // Set up our screen handle so we can talk to the game
    setOwnerScreen(ownerScreen);

    // Register functions here
    ADDFUNC(continueGame);
    ADDFUNC(exitGame);
    ADDFUNC(print);
}

void PauseMenuAwesomiumAPI::setOwnerScreen(vui::IGameScreen* ownerScreen) {
    _ownerScreen = static_cast<GameplayScreen*>(ownerScreen);
}

void PauseMenuAwesomiumAPI::continueGame(const Awesomium::JSArray& args) {
    _ownerScreen->unPause();
}

void PauseMenuAwesomiumAPI::exitGame(const Awesomium::JSArray& args) {
    // Remove the const qualifier and exit the game
    const_cast<App*>(_ownerScreen->_app)->exitGame();
}
