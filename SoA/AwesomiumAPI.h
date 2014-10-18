#pragma once

#include <Awesomium/JSArray.h>
#include <Awesomium/JSValue.h>
#include <map>

//Static class that implements the C++ callbacks for Awesomium
class AwesomiumAPI {
public:
    typedef void(AwesomiumAPI::*setptr)(const Awesomium::JSArray& args);
    typedef Awesomium::JSValue(AwesomiumAPI::*getptr)(const Awesomium::JSArray& args);

    AwesomiumAPI();

    void init(Awesomium::JSObject* interfaceObject);

    getptr getFunctionWithReturnValue(const nString& name);
    setptr getVoidFunction(const nString& name);

private:
    void addFunctionWithReturnValue(const nString& name, getptr func);
    void addVoidFunction(const nString& name, setptr func);

    Awesomium::JSValue getCameraPosition(const Awesomium::JSArray& args);
    Awesomium::JSValue getPlanetRadius(const Awesomium::JSArray& args);

    void setCameraFocalLength(const Awesomium::JSArray& args);
    void setCameraPosition(const Awesomium::JSArray& args);
    void setCameraTarget(const Awesomium::JSArray& args);

    std::map<nString, setptr> _voidFunctions;
    std::map<nString, getptr> _returnFunctions;

    Awesomium::JSObject* _interfaceObject;
};