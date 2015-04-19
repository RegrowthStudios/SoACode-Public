#ifndef IAWESOMIUMAPI_CPP_
#define IAWESOMIUMAPI_CPP_

#include <Awesomium/STLHelpers.h>

#include "Camera.h"
#include "GameManager.h"
#include "AwesomiumInterface.h"


template <class C>
IAwesomiumAPI<C>::IAwesomiumAPI() {
    // Empty
}

template <class C>
IAwesomiumAPI<C>::~IAwesomiumAPI() {
    // Empty
}

template <class C>
void IAwesomiumAPI<C>::addFunction(const nString& name, typename IAwesomiumAPI<C>::getptr func) {
    m_returnFunctions[name] = func;
    m_methodHandler->gameInterface->SetCustomMethod(Awesomium::WSLit(name.c_str()), true);
}

template <class C>
void IAwesomiumAPI<C>::addFunction(const nString& name, typename IAwesomiumAPI<C>::setptr func) {
    m_voidFunctions[name] = func;
    m_methodHandler->gameInterface->SetCustomMethod(Awesomium::WSLit(name.c_str()), false);
}

template <class C>
void IAwesomiumAPI<C>::addObject(const cString name, ui32 id) {
    Awesomium::JSValue val = m_webView->ExecuteJavascriptWithResult(Awesomium::WSLit(name), Awesomium::WSLit(""));
    if (val.IsObject()) {
        Awesomium::JSObject& object = val.ToObject();
        Awesomium::JSArray args = object.GetMethodNames();
        std::cout << "DERE IS " << args.size() << " ARGS IN " << name << std::endl;
        for (int i = 0; i < args.size(); i++) {
            std::cout << "ARG: " << args[i].ToString() << std::endl;
        }
        m_methodHandler->m_customObjects[id] = val.ToObject();
    } else {
        printf("Failed to make JS object %s\n", name);
    }
}

template <class C>
Awesomium::JSObject IAwesomiumAPI<C>::getObject(ui32 id) {
    return m_methodHandler->m_customObjects[id];
}

template <class C>
typename IAwesomiumAPI<C>::getptr IAwesomiumAPI<C>::getFunctionWithReturnValue(const nString& name) {
    auto it = m_returnFunctions.find(name);

    if (it != m_returnFunctions.end()) {
        return it->second;
    }

    return nullptr;
}

template <class C>
typename IAwesomiumAPI<C>::setptr IAwesomiumAPI<C>::getVoidFunction(const nString& name) {
    auto it = m_voidFunctions.find(name);

    if (it != m_voidFunctions.end()) {
        return it->second;
    }

    return nullptr;
}

template <class C>
void IAwesomiumAPI<C>::print(const Awesomium::JSArray& args) {
    if (!args.size()) return;

    if (args[0].IsDouble()) {
        printf("%lf\n", args[0].ToDouble());
    } else if (args[0].IsString()) {
        printf("%s\n", args[0].ToString());
    } else if (args[0].IsInteger()) {
        printf("%d\n", args[0].ToInteger());
    } else if (args[0].IsBoolean()) {
        printf("%d\n", (int)args[0].ToBoolean());
    }
}

#endif IAWESOMIUMAPI_CPP_