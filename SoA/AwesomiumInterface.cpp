

#include "stdafx.h"

#include "AwesomiumInterface.h"

#include "Errors.h"
#include "Options.h"

#ifndef AWESOMIUMINTERFACE_CPP_
#define AWESOMIUMINTERFACE_CPP_

namespace vorb {
namespace ui {

template <class C>
AwesomiumInterface<C>::AwesomiumInterface() :
    _isInitialized(false),
    _openglSurfaceFactory(nullptr),
    _renderedTexture(0),
    _width(0),
    _height(0),
    _vbo(0),
    _ibo(0),
    _color(255, 255, 255, 255) {
    // Empty
}

template <class C>
AwesomiumInterface<C>::~AwesomiumInterface(void) {
    destroy();
}

//Initializes the interface. Returns false on failure
template <class C>
bool AwesomiumInterface<C>::init(const char *inputDir, const char* indexName, ui32 width, ui32 height, C* api, IGameScreen* ownerScreen)
{
    // Set dimensions
    _width = width;
    _height = height;

    // Set the API
    _awesomiumAPI = api;

    // Set default draw rectangle
    i32v4 destRect(0, 0, _width, _height);
    setDrawRect(destRect);

    // Sets up the webCore, which is the main process
    _webCore = Awesomium::WebCore::instance();
    if (_webCore == nullptr){
        _webCore = Awesomium::WebCore::Initialize(Awesomium::WebConfig());
    }

    //Set up our custom surface factory for direct opengl rendering
    _openglSurfaceFactory = new Awesomium::OpenglSurfaceFactory;
    _webCore->set_surface_factory(_openglSurfaceFactory);

    //Sets up the session
    _webSession = _webCore->CreateWebSession(Awesomium::WSLit("WebSession"), Awesomium::WebPreferences());
    _webView = _webCore->CreateWebView((i32)_width, (i32)_height, _webSession, Awesomium::kWebViewType_Offscreen);

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

    // Load a certain URL into our WebView instance. In this case, it must always start with Index.html
    Awesomium::WebURL url(Awesomium::WSLit("asset://UI/index.html"));

    if (!url.IsValid()){
        pError("UI Initialization Error: URL was unable to be parsed.");
        return false;
    }

    // Set up the Game interface
    _gameInterface = _webView->CreateGlobalJavascriptObject(Awesomium::WSLit("App"));
    if (_gameInterface.IsObject()){
        _methodHandler.gameInterface = &_gameInterface.ToObject();

        //Initialize the callback API
        _awesomiumAPI->init(_methodHandler.gameInterface, ownerScreen);
    } else {
        pError("Awesomium Error: Failed to create app object.");
    }

    _webView->LoadURL(url);

    // Wait for our WebView to finish loading
    while (_webView->IsLoading()) _webCore->Update();

    // Sleep a bit and update once more to give scripts and plugins
    // on the page a chance to finish loading.
    Sleep(30);
    _webCore->Update();
    _webView->SetTransparent(true);
    _webView->set_js_method_handler(&_methodHandler);

    // Set the callback API
    _methodHandler.setAPI(_awesomiumAPI);

    Awesomium::Error error = _webView->last_error();
    if (error) {
        pError("Awesomium Error: " + std::to_string(error));
    }

    //Set up the JS interface
    _jsInterface = _webView->ExecuteJavascriptWithResult(Awesomium::WSLit("JSInterface"), Awesomium::WSLit(""));
    if (_jsInterface.IsObject()){
        _methodHandler.jsInterface = &_jsInterface.ToObject();
    }

    _isInitialized = true;
    return true;
}

template <class C>
void AwesomiumInterface<C>::destroy() {
    _webSession->Release();
    _webView->Destroy();
    delete _openglSurfaceFactory;
    delete _data_source;
    _isInitialized = false;
}

template <class C>
void AwesomiumInterface<C>::handleEvent(const SDL_Event& evnt) {
    float relX, relY;
    Awesomium::WebKeyboardEvent keyEvent;

    switch (evnt.type) {
        case SDL_MOUSEMOTION:
            relX = (evnt.motion.x - _drawRect.x) / (float)_drawRect.z;
            relY = (evnt.motion.y - _drawRect.y) / (float)_drawRect.w;

            _webView->Focus();
            _webView->InjectMouseMove(relX * _width, relY * _height);
            break;
        case SDL_MOUSEBUTTONDOWN:
            _webView->Focus();
            _webView->InjectMouseDown(getAwesomiumButtonFromSDL(evnt.button.button));
            break;
        case SDL_MOUSEBUTTONUP:
            _webView->Focus();
            _webView->InjectMouseUp(getAwesomiumButtonFromSDL(evnt.button.button));
            break;
        case SDL_MOUSEWHEEL:
            _webView->Focus();
            _webView->InjectMouseWheel(evnt.motion.y, evnt.motion.x);
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            //Have to construct a webKeyboardEvent from the SDL Event
            char* buf = new char[20];
            keyEvent.virtual_key_code = getWebKeyFromSDLKey(evnt.key.keysym.scancode);
            Awesomium::GetKeyIdentifierFromVirtualKeyCode(keyEvent.virtual_key_code,
                                                            &buf);
            strcpy(keyEvent.key_identifier, buf);

            delete[] buf;

            keyEvent.modifiers = 0;

            if (evnt.key.keysym.mod & KMOD_LALT || evnt.key.keysym.mod & KMOD_RALT)
                keyEvent.modifiers |= Awesomium::WebKeyboardEvent::kModAltKey;
            if (evnt.key.keysym.mod & KMOD_LCTRL || evnt.key.keysym.mod & KMOD_RCTRL)
                keyEvent.modifiers |= Awesomium::WebKeyboardEvent::kModControlKey;
            if (evnt.key.keysym.mod & KMOD_LSHIFT || evnt.key.keysym.mod & KMOD_RSHIFT)
                keyEvent.modifiers |= Awesomium::WebKeyboardEvent::kModShiftKey;
            if (evnt.key.keysym.mod & KMOD_NUM)
                keyEvent.modifiers |= Awesomium::WebKeyboardEvent::kModIsKeypad;

            keyEvent.native_key_code = evnt.key.keysym.scancode;

            if (evnt.type == SDL_KEYUP) {
                keyEvent.type = Awesomium::WebKeyboardEvent::kTypeKeyUp;
                _webView->InjectKeyboardEvent(keyEvent);
            } else if (evnt.type == SDL_TEXTINPUT) {
                unsigned int chr;

                chr = (int)evnt.text.text;
                keyEvent.text[0] = chr;
                keyEvent.unmodified_text[0] = chr;

                keyEvent.type = Awesomium::WebKeyboardEvent::kTypeKeyDown;
                _webView->InjectKeyboardEvent(keyEvent);

                if (chr) {
                    keyEvent.type = Awesomium::WebKeyboardEvent::kTypeChar;
                    keyEvent.virtual_key_code = chr;
                    keyEvent.native_key_code = chr;
                    _webView->InjectKeyboardEvent(keyEvent);
                }
            }
            break;
    }
}

template <class C>
void AwesomiumInterface<C>::update()
{
    _webCore->Update();

    Awesomium::OpenglSurface* surface = (Awesomium::OpenglSurface*)_webView->surface();

    _renderedTexture = surface->getTextureID();

    if (!surface){
        pError("User Interface Error: webView->surface() returned null! Most likely due to erroneous code in the javascript or HTML5.\n");
    }
}

template <class C>
void AwesomiumInterface<C>::draw(vcore::GLProgram* program)
{
    //Check if draw coords were set
    if (_vbo == 0 || _renderedTexture == 0) return;

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);

    // Bind shader
    program->use();
    program->enableVertexAttribArrays();

    glUniform1f(program->getUniform("xdim"), graphicsOptions.screenWidth);
    glUniform1f(program->getUniform("ydim"), graphicsOptions.screenHeight);

    glUniform1i(program->getUniform("roundMaskTexture"), 1);
    glUniform1f(program->getUniform("isRound"), 0.0f);

    glUniform1f(program->getUniform("xmod"), (GLfloat)0.0f);
    glUniform1f(program->getUniform("ymod"), (GLfloat)0.0f);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _renderedTexture);

    glUniform1i(program->getUniform("myTextureSampler"), 0);

    glVertexAttribPointer(program->getAttribute("vertexPosition_screenspace"), 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)0);
    glVertexAttribPointer(program->getAttribute("vertexUV"), 2, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex2D), (void*)8);
    glVertexAttribPointer(program->getAttribute("vertexColor"), 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex2D), (void*)12);

    // Draw call
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    program->disableVertexAttribArrays();
    program->use();
}

template <class C>
void AwesomiumInterface<C>::setDrawRect(const i32v4& rect) {

    const GLubyte uiBoxUVs[8] = { 0, 0, 0, 255, 255, 255, 255, 0 };

    _drawRect = rect;

    if (_vbo == 0) {
        glGenBuffers(1, &_vbo);
        glGenBuffers(1, &_ibo);

        GLushort elements[6] = { 0, 1, 2, 2, 3, 0 };
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    Vertex2D vertices[4];

    vertices[0].pos.x = _drawRect.x;
    vertices[0].pos.y = _drawRect.y + _drawRect.w;

    vertices[1].pos.x = _drawRect.x;
    vertices[1].pos.y = _drawRect.y;

    vertices[2].pos.x = _drawRect.x + _drawRect.z;
    vertices[2].pos.y = _drawRect.y;

    vertices[3].pos.x = _drawRect.x + _drawRect.z;
    vertices[3].pos.y = _drawRect.y + _drawRect.w;

    for (int i = 0; i < 4; i++) {
        vertices[i].uv[0] = uiBoxUVs[i * 2];
        vertices[i].uv[1] = uiBoxUVs[i * 2 + 1];

        vertices[i].color = _color;
    }

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

template <class C>
void AwesomiumInterface<C>::setColor(const ColorRGBA8& color) {
    _color = color;
    // Update the vbo
    setDrawRect(_drawRect);
}

template <class C>
void CustomJSMethodHandler<C>::OnMethodCall(Awesomium::WebView *caller, unsigned int remote_object_id, const Awesomium::WebString &method_name, const Awesomium::JSArray &args) {
   
    std::cout << "Method call: " << method_name << std::endl;
    IAwesomiumAPI<C>::setptr funcptr = _api->getVoidFunction(Awesomium::ToString(method_name));
    if (funcptr) {
        ((*_api).*funcptr)(args);
    }
}

template <class C>
Awesomium::JSValue CustomJSMethodHandler<C>::OnMethodCallWithReturnValue(Awesomium::WebView *caller, unsigned int remote_object_id, const Awesomium::WebString &method_name, const Awesomium::JSArray &args) {
   
    std::cout << "Method call with return value: " << method_name << std::endl;
    IAwesomiumAPI<C>::getptr funcptr = _api->getFunctionWithReturnValue(Awesomium::ToString(method_name));
    if (funcptr) {
        return ((*_api).*funcptr)(args);
    }
    return Awesomium::JSValue(0);
}

/// Helper Macro
#define mapKey(a, b) case SDLK_##a: return Awesomium::KeyCodes::AK_##b;


/// Converts SDL button to awesomium button
template <class C>
Awesomium::MouseButton AwesomiumInterface<C>::getAwesomiumButtonFromSDL(Uint8 SDLButton) {
    switch (SDLButton) {
        case SDL_BUTTON_LEFT:
            return Awesomium::MouseButton::kMouseButton_Left;
        case SDL_BUTTON_RIGHT:
            return Awesomium::MouseButton::kMouseButton_Right;
        case SDL_BUTTON_MIDDLE:
            return Awesomium::MouseButton::kMouseButton_Middle;
    }
    return Awesomium::MouseButton::kMouseButton_Left;
}

/// Get an Awesomium KeyCode from an SDLKey Code
template <class C>
int AwesomiumInterface<C>::getWebKeyFromSDLKey(SDL_Scancode key) {
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
            //	mapKey(DELETE, DELETE)
            /*	mapKey(KP0, NUMPAD0)
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
            //	mapKey(SCROLLOCK, SCROLL)
            mapKey(RSHIFT, RSHIFT)
            mapKey(LSHIFT, LSHIFT)
            mapKey(RCTRL, RCONTROL)
            mapKey(LCTRL, LCONTROL)
            mapKey(RALT, RMENU)
            mapKey(LALT, LMENU)
            //	mapKey(RMETA, LWIN)
            //	mapKey(LMETA, RWIN)
            //	mapKey(LSUPER, LWIN)
            //	mapKey(RSUPER, RWIN)
            mapKey(MODE, MODECHANGE)
            //	mapKey(COMPOSE, ACCEPT)
            mapKey(HELP, HELP)
            //	mapKey(PRINT, SNAPSHOT)
            mapKey(SYSREQ, EXECUTE)
        default:
            return Awesomium::KeyCodes::AK_UNKNOWN;
    }
}

}
}

#endif // AWESOMIUMINTERFACE_CPP_