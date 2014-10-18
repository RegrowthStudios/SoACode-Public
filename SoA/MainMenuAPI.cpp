#include "stdafx.h"
#include "MainMenuAPI.h"

#include "MainMenuScreen.h"
#include "GameManager.h"
#include "Planet.h"

void MainMenuAPI::init(Awesomium::JSObject* interfaceObject, IGameScreen* ownerScreen) {

    // Set up the interface object so we can talk to the JS
    _interfaceObject = interfaceObject;

    // Set up our screen handle so we can talk to the game
    setOwnerScreen(ownerScreen);

    // Add functions here
    addFunctionWithReturnValue("getCameraPosition", &MainMenuAPI::getCameraPosition);
    addFunctionWithReturnValue("getPlanetRadius", &MainMenuAPI::getPlanetRadius);

    addVoidFunction("setCameraFocalLength", &MainMenuAPI::setCameraFocalLength);
    addVoidFunction("setCameraPosition", &MainMenuAPI::setCameraPosition);
    addVoidFunction("setCameraTarget", &MainMenuAPI::setCameraTarget);

}

void MainMenuAPI::setOwnerScreen(IGameScreen* ownerScreen) {
    _ownerScreen = static_cast<MainMenuScreen*>(ownerScreen);
}

Awesomium::JSValue MainMenuAPI::getCameraPosition(const Awesomium::JSArray& args) {
    Awesomium::JSArray rv;
    const f64v3& pos = _ownerScreen->getCamera().position();
    rv.Push(Awesomium::JSValue(pos.x));
    rv.Push(Awesomium::JSValue(pos.y));
    rv.Push(Awesomium::JSValue(pos.z));
    return Awesomium::JSValue(rv);
}

Awesomium::JSValue MainMenuAPI::getPlanetRadius(const Awesomium::JSArray& args) {
    return Awesomium::JSValue(GameManager::planet->radius);
}

void MainMenuAPI::setCameraFocalLength(const Awesomium::JSArray& args) {
    _ownerScreen->getCamera().setFocalLength((float)args[0].ToDouble());
}

void MainMenuAPI::setCameraPosition(const Awesomium::JSArray& args) {
    _ownerScreen->getCamera().setPosition(f64v3(args[0].ToDouble(), args[1].ToDouble(), args[2].ToDouble()));
}

void MainMenuAPI::setCameraTarget(const Awesomium::JSArray& args) {
    f64v3 targetPos(args[0].ToDouble(), args[1].ToDouble(), args[2].ToDouble());
    float time = args[3].ToDouble();
    float focalLength = args[4].ToDouble();
    f64v3 targetDir(args[5].ToDouble(), args[6].ToDouble(), args[7].ToDouble());
    f64v3 targetRight(args[8].ToDouble(), args[9].ToDouble(), args[10].ToDouble());
    _ownerScreen->getCamera().zoomTo(targetPos, time, glm::normalize(targetDir), glm::normalize(targetRight), glm::dvec3(0.0), GameManager::planet->radius, focalLength);
}