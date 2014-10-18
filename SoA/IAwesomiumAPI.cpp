#include "stdafx.h"

#include "IAwesomiumAPI.h"

#include <Awesomium/STLHelpers.h>
#include "Camera.h"
#include "GameManager.h"
#include "Planet.h"

namespace vorb {
namespace ui {

template <class C>
IAwesomiumAPI<C>::IAwesomiumAPI() :
    _interfaceObject(nullptr) {
    // Empty
}

template <class C>
void IAwesomiumAPI<C>::addFunctionWithReturnValue(const nString& name, getptr func) {
    _returnFunctions[name] = func;
    _interfaceObject->SetCustomMethod(Awesomium::WSLit(name.c_str()), true);
}

template <class C>
void IAwesomiumAPI<C>::addVoidFunction(const nString& name, setptr func) {
    _voidFunctions[name] = func;
    _interfaceObject->SetCustomMethod(Awesomium::WSLit(name.c_str()), false);
}

template <class C>
typename IAwesomiumAPI<C>::getptr IAwesomiumAPI<C>::getFunctionWithReturnValue(const nString& name) {
    auto it = _returnFunctions.find(name);

    if (it != _returnFunctions.end()) {
        return it->second;
    }

    return nullptr;
}

template <class C>
typename IAwesomiumAPI<C>::setptr IAwesomiumAPI<C>::getVoidFunction(const nString& name) {
    auto it = _voidFunctions.find(name);

    if (it != _voidFunctions.end()) {
        return it->second;
    }

    return nullptr;
}

}
}