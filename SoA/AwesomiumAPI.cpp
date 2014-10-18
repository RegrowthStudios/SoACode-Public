#include "stdafx.h"

#include "AwesomiumAPI.h"

#include <Awesomium/STLHelpers.h>
#include "Camera.h"
#include "GameManager.h"
#include "Planet.h"

AwesomiumAPI::AwesomiumAPI() : _interfaceObject(nullptr) {}

void AwesomiumAPI::init(Awesomium::JSObject* interfaceObject) {

    _interfaceObject = interfaceObject;

    addFunctionWithReturnValue("getCameraPosition", &AwesomiumAPI::getCameraPosition);
    addFunctionWithReturnValue("getPlanetRadius", &AwesomiumAPI::getPlanetRadius);

    addVoidFunction("setCameraFocalLength", &AwesomiumAPI::setCameraFocalLength);
    addVoidFunction("setCameraPosition", &AwesomiumAPI::setCameraPosition);
    addVoidFunction("setCameraTarget", &AwesomiumAPI::setCameraTarget);

}

void AwesomiumAPI::addFunctionWithReturnValue(const nString& name, getptr func) {
    _returnFunctions[name] = func;
    _interfaceObject->SetCustomMethod(Awesomium::WSLit(name.c_str()), true);
}

void AwesomiumAPI::addVoidFunction(const nString& name, setptr func) {
    _voidFunctions[name] = func;
    _interfaceObject->SetCustomMethod(Awesomium::WSLit(name.c_str()), false);
}


AwesomiumAPI::getptr AwesomiumAPI::getFunctionWithReturnValue(const nString&  name) {
    auto it = _returnFunctions.find(name);

    if (it != _returnFunctions.end()) {
        return it->second;
    }

    return nullptr;
}

AwesomiumAPI::setptr AwesomiumAPI::getVoidFunction(const nString&  name) {
    auto it = _voidFunctions.find(name);

    if (it != _voidFunctions.end()) {
        return it->second;
    }

    return nullptr;
}

Awesomium::JSValue AwesomiumAPI::getCameraPosition(const Awesomium::JSArray& args) {
    Awesomium::JSArray rv;
  /*  const f64v3& pos = GameManager::mainMenuCamera->getPosition();
    rv.Push(Awesomium::JSValue(pos.x));
    rv.Push(Awesomium::JSValue(pos.y));
    rv.Push(Awesomium::JSValue(pos.z));*/
    return Awesomium::JSValue(rv);
}

Awesomium::JSValue AwesomiumAPI::getPlanetRadius(const Awesomium::JSArray& args) {
    return Awesomium::JSValue(GameManager::planet->radius);
}

void AwesomiumAPI::setCameraFocalLength(const Awesomium::JSArray& args) {
  //  GameManager::mainMenuCamera->setFocalLength((float)args[0].ToDouble());
}

void AwesomiumAPI::setCameraPosition(const Awesomium::JSArray& args) {
  //  GameManager::mainMenuCamera->setPosition(f64v3(args[0].ToDouble(), args[1].ToDouble(), args[2].ToDouble()));
}

void AwesomiumAPI::setCameraTarget(const Awesomium::JSArray& args) {
    f64v3 targetPos(args[0].ToDouble(), args[1].ToDouble(), args[2].ToDouble());
    float time = args[3].ToDouble();
    float focalLength = args[4].ToDouble();
    f64v3 targetDir(args[5].ToDouble(), args[6].ToDouble(), args[7].ToDouble());
    f64v3 targetRight(args[8].ToDouble(), args[9].ToDouble(), args[10].ToDouble());
  //  GameManager::mainMenuCamera->zoomTo(targetPos, time, glm::normalize(targetDir), glm::normalize(targetRight), focalLength);
}