#pragma once

#ifndef IAwesomiumInterface_h__
#define IAwesomiumInterface_h__

#include "stdafx.h"
#include "GLProgram.h"

#include <Awesomium/BitmapSurface.h>
#include <Awesomium/DataPak.h>
#include <Awesomium/DataSource.h>
#include <Awesomium/STLHelpers.h>
#include <Awesomium/WebCore.h>
#include <SDL/SDL.h>

/// Abstract class that allows you to hold a handle to a a generic AwesomiumInterface<C>
class IAwesomiumInterface {
public:
    /// Frees all resources
    virtual void destroy() = 0;

    /// Invokes a JS function
    /// @param functionName: Name of the function
    /// @param args: the argument list
    virtual void invokeFunction(const cString functionName, const Awesomium::JSArray& args = Awesomium::JSArray()) = 0;

    /// Updates the UI
    virtual void update() = 0;

    /// Draws the UI
    /// @param program: the opengl program to use
    virtual void draw(vg::GLProgram* program) const = 0;

    /// Sets the dest rectangle to render the UI
    /// @param rect: Dest rectangle (x,y,w,h)
    virtual void setDrawRect(const i32v4& rect) = 0;

    /// Sets the color of the UI
    /// @param color: desired color
    virtual void setColor(const ColorRGBA8& color) = 0;

protected:
    /// Awesomium variables
    Awesomium::DataSource* _data_source;
    Awesomium::WebSession* _webSession;
    Awesomium::WebCore* _webCore;
    Awesomium::WebView* _webView;
    Awesomium::JSValue _gameInterface;
    Awesomium::JSValue _window;
};

#endif // IAwesomiumInterface_h__