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

#ifndef IAwesomiumAPI_h__
#define IAwesomiumAPI_h__

#include <Awesomium/JSArray.h>
#include <Awesomium/JSValue.h>
#include <Vorb/ui/IGameScreen.h>
#include <Vorb/VorbPreDecl.inl>
#include "AwesomiumInterface.h"

/// class that implements the C++ callbacks for Awesomium
/// The template argument should be the derived class
template <class C>
class IAwesomiumAPI {
public:
    /// Typedefs for function pointers for getters and setters
    typedef void(C::*setptr)(const Awesomium::JSArray& args);
    typedef Awesomium::JSValue(C::*getptr)(const Awesomium::JSArray& args);

    IAwesomiumAPI();
    virtual ~IAwesomiumAPI();

    /// Initializes the API and hooks up all functions
    virtual void init(Awesomium::WebView* webView, vui::CustomJSMethodHandler<C>* methodHandler,
                      vui::IGameScreen* ownerScreen) = 0;

    /// Sets the screen that owns this API
    /// @param ownerScreen: The screen
    virtual void setOwnerScreen(vui::IGameScreen* ownerScreen) = 0;

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

    /// Adds a JS object to be tracked
    /// @param name: The name of the function
    /// @param id: ID of the object
    virtual void addExistingObject(const cString name, ui32 id);

    /// Returns the object associated with the id
    virtual Awesomium::JSObject getObject(ui32 id);

    /// Prints a message to the console followed by a newline
    /// @param args: Argument can be a string, int, float, or bool
    virtual void print(const Awesomium::JSArray& args);

    std::map<nString, setptr> m_voidFunctions; ///< map of void functions
    std::map<nString, getptr> m_returnFunctions; ///< map of get functions

    Awesomium::WebView* m_webView = nullptr;
    vui::CustomJSMethodHandler<C>* m_methodHandler = nullptr; ///< Has interface objects
};

#include "IAwesomiumAPI.inl"

#endif // IAwesomiumAPI_h__
