// 
//  AwesomiumInterface.h
//  Vorb Engine
//
//  Created by Ben Arnold on 17 Oct 2014
//  Copyright 2014 Regrowth Studios
//  All Rights Reserved
//  
//  This file provides an awesomium UI interface for
//  HTML/CSS based UI.
//

#pragma once

#ifndef AWESOMIUMINTERFACE_H_
#define AWESOMIUMINTERFACE_H_

#include "SDL/SDL.h"
#include "IAwesomiumInterface.h"

#include "OpenglSurfaceFactory.h"

#include "GLProgram.h"

class IGameScreen;



namespace vorb {
namespace ui {

/// This class allows for custom JS method handling
template <class C>
class CustomJSMethodHandler : public Awesomium::JSMethodHandler
{
public:
    CustomJSMethodHandler() : _api(nullptr) {}

    /// Sets the callback API
    /// @param api: the AwesomiumAPI to use
    void setAPI(C* api) { _api = api; }

    /// Overloaded functions from Awesomium::JSMethodHandler
    void OnMethodCall(Awesomium::WebView *caller, unsigned int remote_object_id, const Awesomium::WebString &method_name, const Awesomium::JSArray &args);
    Awesomium::JSValue OnMethodCallWithReturnValue(Awesomium::WebView *caller, unsigned int remote_object_id, const Awesomium::WebString &method_name, const Awesomium::JSArray &args);

    /// The interface objects for the JS and the game
    Awesomium::JSObject *gameInterface, *jsInterface;

private:
    C* _api; ///< The current function call API to use
};

/// This class provides an awesomium based UI implementation
/// The template argument should be the API class derived from
/// IAwesomiumAPI
template <class C>
class AwesomiumInterface : public IAwesomiumInterface{
public:
    AwesomiumInterface();
    ~AwesomiumInterface();

    /// Initializes the UI
    /// @param inputDir: the directory where the HTML files are located.
    /// @param sessionName: The name of the session. Should be unique
    /// @param indexName: The name of the first file to open, ususally index.html
    /// @param width: The width of the UI window
    /// @param height: The height of the UI window
    /// @return true on success, false on failure
    bool init(const char* inputDir, const char* sessionName, const char* indexName, ui32 width, ui32 height, IGameScreen* ownerScreen);

    /// Frees all resources
    void destroy() override;
    
    /// Invokes a JS function
    /// @param functionName: Name of the function
    /// @param args: the argument list
    void invokeFunction(const cString functionName, const Awesomium::JSArray& args = Awesomium::JSArray()) override;

    /// Handles an SDL event
    /// @param evnt: the SDL event to handle
    void handleEvent(const SDL_Event& evnt) override;

    /// Updates the UI
    void update() override;

    /// Draws the UI
    /// @param program: the opengl program to use
    void draw(vg::GLProgram* program) const override;

    /// Sets the dest rectangle to render the UI
    /// @param rect: Dest rectangle (x,y,w,h)
    void setDrawRect(const i32v4& rect) override;

    /// Sets the color of the UI
    /// @param color: desired color
    void setColor(const ColorRGBA8& color) override;

private:
    /// Gets the awesomium mouse button from an SDL button
    /// @param SDLButton: the SDL button
    /// @return the awesomium mouse button
    Awesomium::MouseButton getAwesomiumButtonFromSDL(Uint8 SDLButton);
    /// Gets a web key from an SDL_Scancode
    /// @param key: the scancode
    /// @return the web key code
    int getWebKeyFromSDLKey(SDL_Scancode key);

    bool _isInitialized; ///< true when initialized

    ui32 _width, _height; ///< dimensions of the window

    i32v4 _drawRect; ///< dimensions of the destination rect for drawing

    ui32 _renderedTexture; ///< the opengl texture ID

    ui32 _vbo; ///< the opengl vboID
    ui32 _ibo; ///< the opengl iboID

    ColorRGBA8 _color; ///< the UI color

    ui16 _numFiles; ///< the number of files writen by the awesomium data pak

    Awesomium::OpenglSurfaceFactory* _openglSurfaceFactory; ///< custom surface factory for rendering

    C _awesomiumAPI; ///< the API to use for custom function calls

    CustomJSMethodHandler<C> _methodHandler; ///< the method handler for the function calls

    /// the vertex struct for rendering
    struct Vertex2D {
        f32v2 pos;
        ui8 uv[2];
        ui8 pad[2];
        ColorRGBA8 color;
    };
};

}
}

// Need to include the cpp file in the header for templates
#include "AwesomiumInterface.inl"

namespace vui = vorb::ui;

#endif // AWESOMIUMINTERFACE_H_