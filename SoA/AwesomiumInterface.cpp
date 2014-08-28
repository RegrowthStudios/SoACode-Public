#include "stdafx.h"

#include <Awesomium/BitmapSurface.h>
#include <Awesomium/DataPak.h>
#include <Awesomium/DataSource.h>
#include <Awesomium/STLHelpers.h>
#include <Awesomium/WebCore.h>

#include "AwesomiumInterface.h"

#include "SDL\SDL.h"
#include "Errors.h"
#include "shader.h"
#include "Options.h"

AwesomiumInterface::AwesomiumInterface() : _isInitialized(0), _width(0), _height(0), _vboID(0), _elementBufferID(0){}

AwesomiumInterface::~AwesomiumInterface(void) {}

//Initializes the interface. Returns false on failure
bool AwesomiumInterface::init(const char *inputDir, int width, int height)
{
    _width = width;
    _height = height;

    _webCore = Awesomium::WebCore::instance();
    if (_webCore == nullptr){
        _webCore = Awesomium::WebCore::Initialize(Awesomium::WebConfig());
    }

    _webSession = _webCore->CreateWebSession(Awesomium::WSLit("WebSession"), Awesomium::WebPreferences());
    _webView = _webCore->CreateWebView(_width, _height, _webSession, Awesomium::kWebViewType_Offscreen);

    if (!_webView){
        pError("Awesomium WebView could not be created!\n");
        return false;
    }

    if (!Awesomium::WriteDataPak(Awesomium::WSLit("UI_Resources.pak"), Awesomium::WSLit(inputDir), Awesomium::WSLit(""), _numFiles)){
        pError("UI Initialization Error: Failed to write UI_Resources.pak\n");
        return false;
    }

    _data_source = new Awesomium::DataPakSource(Awesomium::WSLit("UI_Resources.pak"));

    _webSession->AddDataSource(Awesomium::WSLit("UI"), _data_source);

    // Load a certain URL into our WebView instance
    Awesomium::WebURL url(Awesomium::WSLit("asset://UI/Index.html"));

    if (!url.IsValid()){
        pError("UI Initialization Error: URL was unable to be parsed.");
        return false;
    }

    _webView->LoadURL(url);

    // Wait for our WebView to finish loading
    while (_webView->IsLoading()) _webCore->Update();

    // Sleep a bit and update once more to give scripts and plugins
    // on the page a chance to finish loading.
    Sleep(30);
    _webCore->Update();
    _webView->SetTransparent(1);
    _webView->set_js_method_handler(&_methodHandler);


    glGenTextures(1, &_renderedTexture);

    glBindTexture(GL_TEXTURE_2D, _renderedTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    Awesomium::Error error = _webView->last_error();
    if (error) {
        pError("Awesomium error " + std::to_string(error));
    }

    Awesomium::BitmapSurface* surface = (Awesomium::BitmapSurface*)_webView->surface();
    if (!surface){
        showMessage("webView->surface() returned null!");
        exit(131521);
    }
    //surface->SaveToJPEG(WSLit("./UI/storedui.jpg")); //debug

    _jsValue = _webView->ExecuteJavascriptWithResult(Awesomium::WSLit("AppInterface"), Awesomium::WSLit(""));
    if (_jsValue.IsObject()){

    }
    _isInitialized = true;
    return true;
}

void AwesomiumInterface::update()
{
    _webCore->Update();

    Awesomium::BitmapSurface* surface = (Awesomium::BitmapSurface*)_webView->surface();

    if (surface && surface->is_dirty())
    {
        // renders the surface buffer to the opengl texture!
        glBindTexture(GL_TEXTURE_2D, _renderedTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->width(), surface->height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, surface->buffer());
        surface->set_is_dirty(0);
    }
    if (!surface){
        pError("User Interface Error: webView->surface() returned null! Most likely due to erroneous code in the javascript or HTML5.\n");
    }
}

void AwesomiumInterface::draw()
{
    //Check if draw coords were set
    if (_vboID == 0) return;

    glBindBuffer(GL_ARRAY_BUFFER, _vboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elementBufferID);

    // Bind shader
    texture2Dshader.Bind((GLfloat)graphicsOptions.screenWidth, (GLfloat)graphicsOptions.screenHeight);
    glUniform1f(texture2Dshader.Text2DUseRoundMaskID, 0.0f);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _renderedTexture);

    glUniform1i(texture2Dshader.Text2DUniformID, 0);
   
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribPointer(1, 2, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)12);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)16);

    // Draw call
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texture2Dshader.Text2DElementBufferID);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    texture2Dshader.UnBind();
}

const GLubyte uiBoxUVs[8] = { 0, 0, 0, 255, 255, 255, 255, 0 };

void AwesomiumInterface::setDrawCoords(int x, int y, int width, int height) {
    if (_vboID == 0) {
        glGenBuffers(1, &_vboID);
        glGenBuffers(1, &_elementBufferID);

        GLubyte elements[6] = { 0, 1, 2, 2, 3, 0 };
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elementBufferID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    Vertex2D vertices[4];
    

    vertices[0].pos.x = x;
    vertices[0].pos.y = y + height;

    vertices[1].pos.x = x;
    vertices[1].pos.y = y;

    vertices[2].pos.x = x + width;
    vertices[2].pos.y = y;

    vertices[3].pos.x = x + width;
    vertices[3].pos.y = y + height;

    for (int i = 0; i < 4; i++) {
        vertices[i].uv[0] = uiBoxUVs[i * 2];
        vertices[i].uv[1] = uiBoxUVs[i * 2 + 1];

        vertices[i].color[0] = _color[0];
        vertices[i].color[1] = _color[1];
        vertices[i].color[2] = _color[2];
        vertices[i].color[3] = _color[3];
    }

    glBindBuffer(GL_ARRAY_BUFFER, _vboID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void AwesomiumInterface::setColor(i32v4 color) {
    _color[0] = color[0];
    _color[1] = color[1];
    _color[2] = color[2];
    _color[3] = color[3];
}

/// Helper Macro
#define mapKey(a, b) case SDLK_##a: return Awesomium::KeyCodes::AK_##b;

/// Get an Awesomium KeyCode from an SDLKey Code
int getWebKeyFromSDLKey(SDL_Scancode key) {
    switch (key) {
        mapKey(BACKSPACE, BACK)
        mapKey(TAB, TAB)
        mapKey(CLEAR, CLEAR)
        mapKey(RETURN, RETURN)
        mapKey(PAUSE, PAUSE)
        mapKey(ESCAPE, ESCAPE)
        mapKey(SPACE, SPACE)
        mapKey(EXCLAIM, 1)
        mapKey(QUOTEDBL, 2)
        mapKey(HASH, 3)
        mapKey(DOLLAR, 4)
        mapKey(AMPERSAND, 7)
        mapKey(QUOTE, OEM_7)
        mapKey(LEFTPAREN, 9)
        mapKey(RIGHTPAREN, 0)
        mapKey(ASTERISK, 8)
        mapKey(PLUS, OEM_PLUS)
        mapKey(COMMA, OEM_COMMA)
        mapKey(MINUS, OEM_MINUS)
        mapKey(PERIOD, OEM_PERIOD)
        mapKey(SLASH, OEM_2)
        mapKey(0, 0)
        mapKey(1, 1)
        mapKey(2, 2)
        mapKey(3, 3)
        mapKey(4, 4)
        mapKey(5, 5)
        mapKey(6, 6)
        mapKey(7, 7)
        mapKey(8, 8)
        mapKey(9, 9)
        mapKey(COLON, OEM_1)
        mapKey(SEMICOLON, OEM_1)
        mapKey(LESS, OEM_COMMA)
        mapKey(EQUALS, OEM_PLUS)
        mapKey(GREATER, OEM_PERIOD)
        mapKey(QUESTION, OEM_2)
        mapKey(AT, 2)
        mapKey(LEFTBRACKET, OEM_4)
        mapKey(BACKSLASH, OEM_5)
        mapKey(RIGHTBRACKET, OEM_6)
        mapKey(CARET, 6)
        mapKey(UNDERSCORE, OEM_MINUS)
        mapKey(BACKQUOTE, OEM_3)
        mapKey(a, A)
        mapKey(b, B)
        mapKey(c, C)
        mapKey(d, D)
        mapKey(e, E)
        mapKey(f, F)
        mapKey(g, G)
        mapKey(h, H)
        mapKey(i, I)
        mapKey(j, J)
        mapKey(k, K)
        mapKey(l, L)
        mapKey(m, M)
        mapKey(n, N)
        mapKey(o, O)
        mapKey(p, P)
        mapKey(q, Q)
        mapKey(r, R)
        mapKey(s, S)
        mapKey(t, T)
        mapKey(u, U)
        mapKey(v, V)
        mapKey(w, W)
        mapKey(x, X)
        mapKey(y, Y)
        mapKey(z, Z)
        //    mapKey(DELETE, DELETE)
        /*    mapKey(KP0, NUMPAD0)
        mapKey(KP1, NUMPAD1)
        mapKey(KP2, NUMPAD2)
        mapKey(KP3, NUMPAD3)
        mapKey(KP4, NUMPAD4)
        mapKey(KP5, NUMPAD5)
        mapKey(KP6, NUMPAD6)
        mapKey(KP7, NUMPAD7)
        mapKey(KP8, NUMPAD8)
        mapKey(KP9, NUMPAD9)*/
        mapKey(KP_PERIOD, DECIMAL)
        mapKey(KP_DIVIDE, DIVIDE)
        mapKey(KP_MULTIPLY, MULTIPLY)
        mapKey(KP_MINUS, SUBTRACT)
        mapKey(KP_PLUS, ADD)
        mapKey(KP_ENTER, SEPARATOR)
        mapKey(KP_EQUALS, UNKNOWN)
        mapKey(UP, UP)
        mapKey(DOWN, DOWN)
        mapKey(RIGHT, RIGHT)
        mapKey(LEFT, LEFT)
        mapKey(INSERT, INSERT)
        mapKey(HOME, HOME)
        mapKey(END, END)
        mapKey(PAGEUP, PRIOR)
        mapKey(PAGEDOWN, NEXT)
        mapKey(F1, F1)
        mapKey(F2, F2)
        mapKey(F3, F3)
        mapKey(F4, F4)
        mapKey(F5, F5)
        mapKey(F6, F6)
        mapKey(F7, F7)
        mapKey(F8, F8)
        mapKey(F9, F9)
        mapKey(F10, F10)
        mapKey(F11, F11)
        mapKey(F12, F12)
        mapKey(F13, F13)
        mapKey(F14, F14)
        mapKey(F15, F15)
        //mapKey(NUMLOCK, NUMLOCK)
        mapKey(CAPSLOCK, CAPITAL)
        //    mapKey(SCROLLOCK, SCROLL)
        mapKey(RSHIFT, RSHIFT)
        mapKey(LSHIFT, LSHIFT)
        mapKey(RCTRL, RCONTROL)
        mapKey(LCTRL, LCONTROL)
        mapKey(RALT, RMENU)
        mapKey(LALT, LMENU)
        //    mapKey(RMETA, LWIN)
        //    mapKey(LMETA, RWIN)
        //    mapKey(LSUPER, LWIN)
        //    mapKey(RSUPER, RWIN)
        mapKey(MODE, MODECHANGE)
        //    mapKey(COMPOSE, ACCEPT)
        mapKey(HELP, HELP)
        //    mapKey(PRINT, SNAPSHOT)
        mapKey(SYSREQ, EXECUTE)
    default:
        return Awesomium::KeyCodes::AK_UNKNOWN;
    }
}
