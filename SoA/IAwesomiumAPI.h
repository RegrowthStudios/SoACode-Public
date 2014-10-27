// 
//  IAwesomiumAPI.h
//  Vorb Engine
//
//  Created by Ben Arnold on 17 Oct 2014
//  Copyright 2014 Regrowth Studios
//  All Rights Reserved
//  
//  This file provides an abstract class for 
//  an awesomium API. Users should derive from
//  IAwesomiumAPI with their own APIs to use for
//  an AwesomiumInterface.
//

#pragma once

#ifndef AWESOMIUMAPI_H_
#define AWESOMIUMAPI_H_

#include <Awesomium/JSArray.h>
#include <Awesomium/JSValue.h>
#include <map>

#include "IGameScreen.h"

namespace vorb {
namespace ui {

/// class that implements the C++ callbacks for Awesomium
/// The template argument should be the derived class
template <class C>
class IAwesomiumAPI {
public:
    /// Typedefs for function pointers for getters and setters
    typedef void(C::*setptr)(const Awesomium::JSArray& args);
    typedef Awesomium::JSValue(C::*getptr)(const Awesomium::JSArray& args);

    IAwesomiumAPI();

    /// Initializes the API and hooks up all functions
    /// @oaram interfaceObject: The object that the API will talk to
    /// @param ownerScreen: The screen that owns this interface
    virtual void init(Awesomium::JSObject* interfaceObject, IGameScreen* ownerScreen) = 0;

    /// Sets the screen that owns this API
    /// @param ownerScreen: The screen
    virtual void setOwnerScreen(IGameScreen* ownerScreen) = 0;

    /// Gets the non-void function associated with a name
    /// @param name: The name of the function
    /// @return The getter function pointer
    virtual getptr getFunctionWithReturnValue(const nString& name);

    /// Gets the void function associated with a name
    /// @param name: The name of the function
    /// @return The setter function pointer
    virtual setptr getVoidFunction(const nString& name);

protected:

    /// Adds a function with return value to the API
    /// @param name: The name of the function
    /// @param func: The function pointer
    virtual void addFunction(const nString& name, getptr func);
    /// Adds a void function the API
    /// @param name: The name of the function
    /// @param func: The void function pointer
    virtual void addFunction(const nString& name, setptr func);

    /// Prints a message to the console folowed by a newline
    /// @param args: Argument can be a string, int, float, or bool
    virtual void print(const Awesomium::JSArray& args);

    std::map<nString, setptr> _voidFunctions; ///< map of void functions
    std::map<nString, getptr> _returnFunctions; ///< map of get functions

    Awesomium::JSObject* _interfaceObject; ///< the interface object to talk to
};

}
}

// Need to include the cpp file in the header for templates
#include "IAwesomiumAPI.cpp"

namespace vui = vorb::ui;

#endif // AWESOMIUMAPI_H_