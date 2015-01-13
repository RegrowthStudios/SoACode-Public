
namespace vorb {
namespace ui {

#define DEFAULT_WEB_URL "asset://UI/"

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
bool AwesomiumInterface<C>::init(const char* inputDir, const char* sessionName, const char* indexName, ui32 width, ui32 height, IGameScreen* ownerScreen)
{
    if (_isInitialized) {
        puts("Awesomium Error: Tried to call AwesomiumInterface::init twice without destroying.");
        return false;
    }
    // Set dimensions
    _width = width;
    _height = height;

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
    _webSession = _webCore->CreateWebSession(Awesomium::WSLit(sessionName), Awesomium::WebPreferences());
    _webView = _webCore->CreateWebView((i32)_width, (i32)_height, _webSession, Awesomium::kWebViewType_Offscreen);

    if (!_webView){
        puts("Awesomium WebView could not be created!\n");
        return false;
    }

    nString pakName = nString(sessionName) + ".pak";

    if (!Awesomium::WriteDataPak(Awesomium::WSLit(pakName.c_str()), Awesomium::WSLit(inputDir), Awesomium::WSLit(""), _numFiles)){
        puts(("UI Initialization Error: Failed to write " + pakName).c_str());
        return false;
    }
    _data_source = new Awesomium::DataPakSource(Awesomium::WSLit(pakName.c_str()));
    _webSession->AddDataSource(Awesomium::WSLit("UI"), _data_source);

    // Load a certain URL into our WebView instance.
    Awesomium::WebURL url(Awesomium::WSLit((DEFAULT_WEB_URL + nString(indexName)).c_str()));

    if (!url.IsValid()){
        puts("UI Initialization Error: URL was unable to be parsed.");
        return false;
    }
    // Sleep a bit to give time for initialization
    Sleep(50);
    // Set up the Game interface
    _gameInterface = _webView->CreateGlobalJavascriptObject(Awesomium::WSLit("App"));
    if (_gameInterface.IsObject()){
        _methodHandler.gameInterface = &_gameInterface.ToObject();

        //Initialize the callback API
        _awesomiumAPI.init(_methodHandler.gameInterface, ownerScreen);
    } else {
        puts("Awesomium Error: Failed to create app object.");
    }

    _webView->LoadURL(url);

    // Wait for our WebView to finish loading
    while (_webView->IsLoading()) {
        Sleep(50);
        _webCore->Update();
    }

    // Sleep a bit and update once more to give scripts and plugins
    // on the page a chance to finish loading.
    Sleep(50);
    _webCore->Update();
    _webView->SetTransparent(true);
    _webView->set_js_method_handler(&_methodHandler);

    // Set the callback API
    _methodHandler.setAPI(&_awesomiumAPI);

    Awesomium::Error error = _webView->last_error();
    if (error) {
        puts(("Awesomium Error: " + std::to_string(error)).c_str());
    }

    // Retrieve the global 'window' object from the page
    _window = _webView->ExecuteJavascriptWithResult(Awesomium::WSLit("window"), Awesomium::WSLit(""));
    if (!_window.IsObject()) {
        puts("Awesomium Error: No window object.");
    }

    // Add input registration
    m_delegatePool.push_back(vui::InputDispatcher::mouse.onFocusGained.addFunctor([=] (Sender s, const MouseEvent& e) { onMouseFocusGained(s, e); }));
    m_delegatePool.push_back(vui::InputDispatcher::mouse.onFocusLost.addFunctor([=] (Sender s, const MouseEvent& e) { onMouseFocusLost(s, e); }));
    m_delegatePool.push_back(vui::InputDispatcher::mouse.onMotion.addFunctor([=] (Sender s, const MouseMotionEvent& e) { onMouseMotion(s, e); }));
    m_delegatePool.push_back(vui::InputDispatcher::mouse.onButtonUp.addFunctor([=] (Sender s, const MouseButtonEvent& e) { onMouseButtonUp(s, e); }));
    m_delegatePool.push_back(vui::InputDispatcher::mouse.onButtonDown.addFunctor([=] (Sender s, const MouseButtonEvent& e) { onMouseButtonDown(s, e); }));
    m_delegatePool.push_back(vui::InputDispatcher::key.onKeyUp.addFunctor([=] (Sender s, const KeyEvent& e) { onKeyUp(s, e); }));
    m_delegatePool.push_back(vui::InputDispatcher::key.onKeyDown.addFunctor([=] (Sender s, const KeyEvent& e) { onKeyDown(s, e); }));
    m_delegatePool.push_back(vui::InputDispatcher::key.onText.addFunctor([=] (Sender s, const TextEvent& e) { onText(s, e); }));

    _isInitialized = true;
    return true;
}

template <class C>
void AwesomiumInterface<C>::destroy() {
    _webSession->Release();
    _webView->Destroy();

    // Unregister events
    vui::InputDispatcher::mouse.onFocusGained -= (IDelegate<const MouseEvent&>*)m_delegatePool[0];
    vui::InputDispatcher::mouse.onFocusLost -= (IDelegate<const MouseEvent&>*)m_delegatePool[1];
    vui::InputDispatcher::mouse.onMotion -= (IDelegate<const MouseMotionEvent&>*)m_delegatePool[2];
    vui::InputDispatcher::mouse.onButtonUp -= (IDelegate<const MouseButtonEvent&>*)m_delegatePool[3];
    vui::InputDispatcher::mouse.onButtonDown -= (IDelegate<const MouseButtonEvent&>*)m_delegatePool[4];
    vui::InputDispatcher::key.onKeyUp -= (IDelegate<const KeyEvent&>*)m_delegatePool[5];
    vui::InputDispatcher::key.onKeyDown -= (IDelegate<const KeyEvent&>*)m_delegatePool[6];
    vui::InputDispatcher::key.onText -= (IDelegate<const TextEvent&>*)m_delegatePool[7];
    for (auto& p : m_delegatePool) delete p;
    m_delegatePool.clear();

    delete _openglSurfaceFactory;
    delete _data_source;
    _isInitialized = false;
}

template <class C>
void AwesomiumInterface<C>::invokeFunction(const cString functionName, const Awesomium::JSArray& args) {
    _window.ToObject().Invoke(Awesomium::WSLit(functionName), args);
}

template <class C>
void AwesomiumInterface<C>::onMouseFocusGained(Sender sender, const MouseEvent& e) {
    _webView->Focus();
}
template <class C>
void AwesomiumInterface<C>::onMouseFocusLost(Sender sender, const MouseEvent& e) {
    _webView->Unfocus();
}
template <class C>
void AwesomiumInterface<C>::onMouseButtonUp(Sender sender, const MouseButtonEvent& e) {
    _webView->InjectMouseUp(getAwesomiumButton(e.button));
}
template <class C>
void AwesomiumInterface<C>::onMouseButtonDown(Sender sender, const MouseButtonEvent& e) {
    _webView->InjectMouseDown(getAwesomiumButton(e.button));
}
template <class C>
void AwesomiumInterface<C>::onMouseMotion(Sender sender, const MouseMotionEvent& e) {
    f32 relX = (e.x - _drawRect.x) / (f32)_drawRect.z;
    f32 relY = (e.y - _drawRect.y) / (f32)_drawRect.w;

    _webView->InjectMouseMove((i32)(relX * _width), (i32)(relY * _height));
}
template <class C>
void AwesomiumInterface<C>::onKeyDown(Sender sender, const KeyEvent& e) {
    Awesomium::WebKeyboardEvent keyEvent;

    keyEvent.virtual_key_code = (i32)SDL_GetKeyFromScancode((SDL_Scancode)e.keyCode);
    keyEvent.native_key_code = e.scanCode;
    
    keyEvent.modifiers = 0;
    if (e.mod.alt != 0) keyEvent.modifiers |= Awesomium::WebKeyboardEvent::kModAltKey;
    if (e.mod.ctrl != 0) keyEvent.modifiers |= Awesomium::WebKeyboardEvent::kModControlKey;
    if (e.mod.shift != 0) keyEvent.modifiers |= Awesomium::WebKeyboardEvent::kModShiftKey;
    if (e.mod.gui != 0) keyEvent.modifiers |= Awesomium::WebKeyboardEvent::kModMetaKey;
    if (e.repeatCount > 0) keyEvent.modifiers |= Awesomium::WebKeyboardEvent::kModIsAutorepeat;

    keyEvent.type = Awesomium::WebKeyboardEvent::kTypeKeyDown;
    _webView->InjectKeyboardEvent(keyEvent);
}
template <class C>
void AwesomiumInterface<C>::onKeyUp(Sender sender, const KeyEvent& e) {
    Awesomium::WebKeyboardEvent keyEvent;

    keyEvent.virtual_key_code = (i32)SDL_GetKeyFromScancode((SDL_Scancode)e.keyCode);
    keyEvent.native_key_code = e.scanCode;

    keyEvent.modifiers = 0;
    if (e.mod.alt != 0) keyEvent.modifiers |= Awesomium::WebKeyboardEvent::kModAltKey;
    if (e.mod.ctrl != 0) keyEvent.modifiers |= Awesomium::WebKeyboardEvent::kModControlKey;
    if (e.mod.shift != 0) keyEvent.modifiers |= Awesomium::WebKeyboardEvent::kModShiftKey;
    if (e.mod.gui != 0) keyEvent.modifiers |= Awesomium::WebKeyboardEvent::kModMetaKey;
    if (e.repeatCount > 0) keyEvent.modifiers |= Awesomium::WebKeyboardEvent::kModIsAutorepeat;

    keyEvent.type = Awesomium::WebKeyboardEvent::kTypeKeyUp;
    _webView->InjectKeyboardEvent(keyEvent);
}
template <class C>
void AwesomiumInterface<C>::onText(Sender sender, const TextEvent& e) {
    Awesomium::WebKeyboardEvent keyEvent;

    memcpy(keyEvent.text, e.wtext, 4 * sizeof(wchar_t));
    memcpy(keyEvent.unmodified_text, e.wtext, 4 * sizeof(wchar_t));

    keyEvent.type = Awesomium::WebKeyboardEvent::kTypeChar;
    _webView->InjectKeyboardEvent(keyEvent);
}

template <class C>
void AwesomiumInterface<C>::update()
{
    _webCore->Update();

    Awesomium::OpenglSurface* surface = (Awesomium::OpenglSurface*)_webView->surface();

    _renderedTexture = surface->getTextureID();

    if (!surface){
        puts("User Interface Error: webView->surface() returned null! Most likely due to erroneous code in the javascript or HTML5.\n");
    }
}

template <class C>
void AwesomiumInterface<C>::draw(vg::GLProgram* program) const
{
    //Check if draw coords were set
    if (_vbo == 0 || _renderedTexture == 0) return;

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);

    // Bind shader
    program->use();
    program->enableVertexAttribArrays();

    glUniform2f(program->getUniform("unScreenSize"), (f32)_width, (f32)_height);
    glUniform2f(program->getUniform("unScreenDisplacement"), 0.0f, 0.0f);
    glUniform1i(program->getUniform("unTexMain"), 0);
    // TODO: Will this be removed?
    //glUniform1i(program->getUniform("unTexMask"), 1);
    //glUniform1f(program->getUniform("unMaskModifier"), 0.0f);
    //glUniform2f(program->getUniform("unUVMaskStart"), 0.0f, 0.0f);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _renderedTexture);

    glVertexAttribPointer(program->getAttribute("vPosition"), 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), offsetptr(Vertex2D, pos));
    glVertexAttribPointer(program->getAttribute("vUV"), 2, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex2D), offsetptr(Vertex2D, uv));
    glVertexAttribPointer(program->getAttribute("vTint"), 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex2D), offsetptr(Vertex2D, color));

    // Draw call
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    program->disableVertexAttribArrays();
    program->unuse();
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

    vertices[0].pos.x = (f32)_drawRect.x;
    vertices[0].pos.y = (f32)(_drawRect.y + _drawRect.w);

    vertices[1].pos.x = (f32)_drawRect.x;
    vertices[1].pos.y = (f32)_drawRect.y;

    vertices[2].pos.x = (f32)(_drawRect.x + _drawRect.z);
    vertices[2].pos.y = (f32)_drawRect.y;

    vertices[3].pos.x = (f32)(_drawRect.x + _drawRect.z);
    vertices[3].pos.y = (f32)(_drawRect.y + _drawRect.w);

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


/// Converts SDL button to awesomium button
template <class C>
Awesomium::MouseButton AwesomiumInterface<C>::getAwesomiumButton(const vui::MouseButton& button) {
    switch (button) {
    case MouseButton::LEFT:
        return Awesomium::MouseButton::kMouseButton_Left;
    case MouseButton::RIGHT:
        return Awesomium::MouseButton::kMouseButton_Right;
    case MouseButton::MIDDLE:
        return Awesomium::MouseButton::kMouseButton_Middle;
    default:
        return Awesomium::MouseButton::kMouseButton_Left;
    }
}

/// Helper Macros
#define mapScanKey(a, b) case SDL_SCANCODE_##a: return Awesomium::KeyCodes::AK_##b;
#define mapKey(a, b) case SDLK_##a: return Awesomium::KeyCodes::AK_##b;

/// Get an Awesomium KeyCode from an SDLKey Code
template <class CC>
int AwesomiumInterface<CC>::getWebKeyFromSDLKey(SDL_Scancode key) {
    switch (key) {
        mapScanKey(BACKSPACE, BACK)
            mapScanKey(TAB, TAB)
            mapScanKey(CLEAR, CLEAR)
            mapScanKey(RETURN, RETURN)
            mapScanKey(PAUSE, PAUSE)
            mapScanKey(ESCAPE, ESCAPE)
            mapScanKey(SPACE, SPACE)
         //   mapKey(EXCLAIM, 1) // These are virtual keys so they don't belong here?
         //   mapKey(QUOTEDBL, 2)
         //   mapKey(HASH, 3)
        //    mapKey(DOLLAR, 4)
         //   mapKey(AMPERSAND, 7)
        //    mapKey(QUOTE, OEM_7)
        //    mapKey(LEFTPAREN, 9)
        //    mapKey(RIGHTPAREN, 0)
        //    mapKey(ASTERISK, 8)
        //    mapKey(PLUS, OEM_PLUS)
            mapScanKey(COMMA, OEM_COMMA)
            mapScanKey(MINUS, OEM_MINUS)
            mapScanKey(PERIOD, OEM_PERIOD)
            mapScanKey(SLASH, OEM_2)
            mapScanKey(0, 0)
            mapScanKey(1, 1)
            mapScanKey(2, 2)
            mapScanKey(3, 3)
            mapScanKey(4, 4)
            mapScanKey(5, 5)
            mapScanKey(6, 6)
            mapScanKey(7, 7)
            mapScanKey(8, 8)
            mapScanKey(9, 9)
       //     mapKey(COLON, OEM_1)
            mapScanKey(SEMICOLON, OEM_1)
        //    mapKey(LESS, OEM_COMMA)
            mapScanKey(EQUALS, OEM_PLUS)
       //     mapKey(GREATER, OEM_PERIOD)
        //    mapScanKey(QUESTION, OEM_2)
       //     mapScanKey(AT, 2)
            mapScanKey(LEFTBRACKET, OEM_4)
            mapScanKey(BACKSLASH, OEM_5)
            mapScanKey(RIGHTBRACKET, OEM_6)
      //      mapKey(CARET, 6)
      //      mapKey(UNDERSCORE, OEM_MINUS)
      //      mapKey(BACKQUOTE, OEM_3)
            mapScanKey(A, A)
            mapScanKey(B, B)
            mapScanKey(C, C)
            mapScanKey(D, D)
            mapScanKey(E, E)
            mapScanKey(F, F)
            mapScanKey(G, G)
            mapScanKey(H, H)
            mapScanKey(I, I)
            mapScanKey(J, J)
            mapScanKey(K, K)
            mapScanKey(L, L)
            mapScanKey(M, M)
            mapScanKey(N, N)
            mapScanKey(O, O)
            mapScanKey(P, P)
            mapScanKey(Q, Q)
            mapScanKey(R, R)
            mapScanKey(S, S)
            mapScanKey(T, T)
            mapScanKey(U, U)
            mapScanKey(V, V)
            mapScanKey(W, W)
            mapScanKey(X, X)
            mapScanKey(Y, Y)
            mapScanKey(Z, Z)
            //	mapScanKey(DELETE, DELETE)
            /*	mapScanKey(KP0, NUMPAD0)
            mapScanKey(KP1, NUMPAD1)
            mapScanKey(KP2, NUMPAD2)
            mapScanKey(KP3, NUMPAD3)
            mapScanKey(KP4, NUMPAD4)
            mapScanKey(KP5, NUMPAD5)
            mapScanKey(KP6, NUMPAD6)
            mapScanKey(KP7, NUMPAD7)
            mapScanKey(KP8, NUMPAD8)
            mapScanKey(KP9, NUMPAD9)*/
            mapScanKey(KP_PERIOD, DECIMAL)
            mapScanKey(KP_DIVIDE, DIVIDE)
            mapScanKey(KP_MULTIPLY, MULTIPLY)
            mapScanKey(KP_MINUS, SUBTRACT)
            mapScanKey(KP_PLUS, ADD)
            mapScanKey(KP_ENTER, SEPARATOR)
            mapScanKey(KP_EQUALS, UNKNOWN)
            mapScanKey(UP, UP)
            mapScanKey(DOWN, DOWN)
            mapScanKey(RIGHT, RIGHT)
            mapScanKey(LEFT, LEFT)
            mapScanKey(INSERT, INSERT)
            mapScanKey(HOME, HOME)
            mapScanKey(END, END)
            mapScanKey(PAGEUP, PRIOR)
            mapScanKey(PAGEDOWN, NEXT)
            mapScanKey(F1, F1)
            mapScanKey(F2, F2)
            mapScanKey(F3, F3)
            mapScanKey(F4, F4)
            mapScanKey(F5, F5)
            mapScanKey(F6, F6)
            mapScanKey(F7, F7)
            mapScanKey(F8, F8)
            mapScanKey(F9, F9)
            mapScanKey(F10, F10)
            mapScanKey(F11, F11)
            mapScanKey(F12, F12)
            mapScanKey(F13, F13)
            mapScanKey(F14, F14)
            mapScanKey(F15, F15)
            //mapScanKey(NUMLOCK, NUMLOCK)
            mapScanKey(CAPSLOCK, CAPITAL)
            //	mapScanKey(SCROLLOCK, SCROLL)
            mapScanKey(RSHIFT, RSHIFT)
            mapScanKey(LSHIFT, LSHIFT)
            mapScanKey(RCTRL, RCONTROL)
            mapScanKey(LCTRL, LCONTROL)
            mapScanKey(RALT, RMENU)
            mapScanKey(LALT, LMENU)
            //	mapScanKey(RMETA, LWIN)
            //	mapScanKey(LMETA, RWIN)
            //	mapScanKey(LSUPER, LWIN)
            //	mapScanKey(RSUPER, RWIN)
            mapScanKey(MODE, MODECHANGE)
            //	mapScanKey(COMPOSE, ACCEPT)
            mapScanKey(HELP, HELP)
            //	mapScanKey(PRINT, SNAPSHOT)
            mapScanKey(SYSREQ, EXECUTE)
        default:
            return Awesomium::KeyCodes::AK_UNKNOWN;
    }
}

}
}
