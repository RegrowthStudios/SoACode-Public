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
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/ui/InputDispatcher.h>

#include "IAwesomiumInterface.h"
#include "OpenglSurfaceFactory.h"

class IGameScreen;

namespace vorb {
namespace ui {

/// This class allows for custom JS method handling
template <class C>
class CustomJSMethodHandler : public Awesomium::JSMethodHandler
{
public:
    /// Sets the callback API
    /// @param api: the AwesomiumAPI to use
    void setAPI(C* api) { m_api = api; }

    /// Overloaded functions from Awesomium::JSMethodHandler
    void OnMethodCall(Awesomium::WebView *caller, unsigned int remote_object_id, const Awesomium::WebString &method_name, const Awesomium::JSArray &args);
    Awesomium::JSValue OnMethodCallWithReturnValue(Awesomium::WebView *caller, unsigned int remote_object_id, const Awesomium::WebString &method_name, const Awesomium::JSArray &args);

    /// The interface objects for the JS and the game
    Awesomium::JSObject *gameInterface;
    std::map<ui32, Awesomium::JSObject> m_customObjects;
private:
    C* m_api = nullptr; ///< The current function call API to use
};

/// This class provides an awesomium based UI implementation
/// The template argument should be the API class derived from
/// IAwesomiumAPI
template <class C>
class AwesomiumInterface : public IAwesomiumInterface {
public:
    virtual ~AwesomiumInterface();

    /// Initializes the UI
    /// @param inputDir: the directory where the HTML files are located.
    /// @param sessionName: The name of the session. Should be unique
    /// @param indexName: The name of the first file to open, ususally index.html
    /// @param width: The width of the UI window
    /// @param height: The height of the UI window
    /// @return true on success, false on failure
    bool init(const char* inputDir, const char* sessionName, const char* indexName, ui32 width, ui32 height, vui::IGameScreen* ownerScreen);

    /// Frees all resources
    void destroy() override;
    
    /// Invokes a JS function
    /// @param functionName: Name of the function
    /// @param args: the argument list
    void invokeFunction(const cString functionName, const Awesomium::JSArray& args = Awesomium::JSArray()) override;

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
    Awesomium::MouseButton getAwesomiumButton(const vui::MouseButton& button);
    /// Gets a web key from an SDL_Scancode
    /// @param key: the scancode
    /// @return the web key code
    int getWebKeyFromSDLKey(SDL_Scancode key);

    /************************************************************************/
    /* Event listeners                                                      */
    /************************************************************************/
    void onKeyUp(Sender sender, const KeyEvent& e);
    void onKeyDown(Sender sender, const KeyEvent& e);
    void onText(Sender sender, const TextEvent& e);
    AutoDelegatePool m_delegatePool; ///< Listeners that must be freed

    bool m_isInitialized = false; ///< true when initialized

    ui32 m_width = 0, m_height = 0; ///< dimensions of the window

    i32v4 m_drawRect; ///< dimensions of the destination rect for drawing

    ui32 m_renderedTexture = 0; ///< the opengl texture ID

    ui32 m_vbo = 0; ///< the opengl vboID
    ui32 m_ibo = 0; ///< the opengl iboID

    color4 m_color = color4(255, 255, 255, 255); ///< the UI color

    ui16 m_numFiles; ///< the number of files writen by the awesomium data pak

    Awesomium::OpenglSurfaceFactory* m_openglSurfaceFactory = nullptr; ///< custom surface factory for rendering

    C m_awesomiumAPI; ///< the API to use for custom function calls

    CustomJSMethodHandler<C> m_methodHandler; ///< the method handler for the function calls

    /// the vertex struct for rendering
    struct Vertex2D {
        f32v2 pos;
        ui8v2 uv;
        ui8v2 pad;
        color4 color;
    };
};

}
}

// Need to include the cpp file in the header for templates
#include "AwesomiumInterface.inl"

namespace vui = vorb::ui;

#endif // AWESOMIUMINTERFACE_H_