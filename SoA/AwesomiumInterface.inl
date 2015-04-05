
namespace vorb {
namespace ui {

#define DEFAULT_WEB_URL "asset://UI/"

template <class C>
AwesomiumInterface<C>::~AwesomiumInterface(void) {
    destroy();
}

//Initializes the interface. Returns false on failure
template <class C>
bool AwesomiumInterface<C>::init(const char* inputDir, const char* sessionName, const char* indexName, ui32 width, ui32 height, IGameScreen* ownerScreen)
{
    if (m_isInitialized) {
        puts("Awesomium Error: Tried to call AwesomiumInterface::init twice without destroying.");
        return false;
    }
    // Set dimensions
    m_width = width;
    m_height = height;

    // Set default draw rectangle
    i32v4 destRect(0, 0, m_width, m_height);
    setDrawRect(destRect);

    // Sets up the webCore, which is the main process
    m_webCore = Awesomium::WebCore::instance();
    if (m_webCore == nullptr){
        m_webCore = Awesomium::WebCore::Initialize(Awesomium::WebConfig());
    }

    //Set up our custom surface factory for direct opengl rendering
    m_openglSurfaceFactory = new Awesomium::OpenglSurfaceFactory;
    m_webCore->set_surface_factory(m_openglSurfaceFactory);

    //Sets up the session
    m_webSession = m_webCore->CreateWebSession(Awesomium::WSLit(sessionName), Awesomium::WebPreferences());
    m_webView = m_webCore->CreateWebView((i32)m_width, (i32)m_height, m_webSession, Awesomium::kWebViewType_Offscreen);

    if (!m_webView){
        puts("Awesomium WebView could not be created!\n");
        return false;
    }

    nString pakName = nString(sessionName) + ".pak";

    if (!Awesomium::WriteDataPak(Awesomium::WSLit(pakName.c_str()), Awesomium::WSLit(inputDir), Awesomium::WSLit(""), m_numFiles)){
        puts(("UI Initialization Error: Failed to write " + pakName).c_str());
        return false;
    }
    m_data_source = new Awesomium::DataPakSource(Awesomium::WSLit(pakName.c_str()));
    m_webSession->AddDataSource(Awesomium::WSLit("UI"), m_data_source);

    // Load a certain URL into our WebView instance.
    Awesomium::WebURL url(Awesomium::WSLit((DEFAULT_WEB_URL + nString(indexName)).c_str()));

    if (!url.IsValid()){
        puts("UI Initialization Error: URL was unable to be parsed.");
        return false;
    }
    // Sleep a bit to give time for initialization
    Sleep(50);
    // Set up the Game interface
    m_gameInterface = m_webView->CreateGlobalJavascriptObject(Awesomium::WSLit("App"));
    if (m_gameInterface.IsObject()){
        m_methodHandler.gameInterface = &m_gameInterface.ToObject();

        //Initialize the callback API
        m_awesomiumAPI.init(m_methodHandler.gameInterface, ownerScreen);
    } else {
        puts("Awesomium Error: Failed to create app object.");
    }

    m_webView->LoadURL(url);

    // Wait for our WebView to finish loading
    while (m_webView->IsLoading()) {
        Sleep(50);
        m_webCore->Update();
    }

    // Sleep a bit and update once more to give scripts and plugins
    // on the page a chance to finish loading.
    Sleep(50);
    m_webCore->Update();
    m_webView->SetTransparent(true);
    m_webView->set_js_method_handler(&m_methodHandler);

    // Set the callback API
    m_methodHandler.setAPI(&m_awesomiumAPI);

    Awesomium::Error error = m_webView->last_error();
    if (error) {
        puts(("Awesomium Error: " + std::to_string(error)).c_str());
    }

    // Retrieve the global 'window' object from the page
    m_window = m_webView->ExecuteJavascriptWithResult(Awesomium::WSLit("window"), Awesomium::WSLit(""));
    if (!m_window.IsObject()) {
        puts("Awesomium Error: No window object.");
    }

    // Add input registration
    m_delegatePool.addAutoHook(vui::InputDispatcher::mouse.onFocusGained, [&](Sender s, const MouseEvent& e) {
        m_webView->Focus();
    });
    m_delegatePool.addAutoHook(vui::InputDispatcher::mouse.onFocusLost, [&](Sender s, const MouseEvent& e) {
        m_webView->Unfocus();
    });
    m_delegatePool.addAutoHook(vui::InputDispatcher::mouse.onMotion, [&](Sender s, const MouseMotionEvent& e) {
        f32 relX = (e.x - m_drawRect.x) / (f32)m_drawRect.z;
        f32 relY = (e.y - m_drawRect.y) / (f32)m_drawRect.w;
        m_webView->InjectMouseMove((i32)(relX * m_width), (i32)(relY * m_height));
    });
    m_delegatePool.addAutoHook(vui::InputDispatcher::mouse.onButtonUp, [&](Sender s, const MouseButtonEvent& e) {
        m_webView->InjectMouseUp(getAwesomiumButton(e.button));
    });
    m_delegatePool.addAutoHook(vui::InputDispatcher::mouse.onButtonDown, [&](Sender s, const MouseButtonEvent& e) {
        m_webView->InjectMouseDown(getAwesomiumButton(e.button));
    });
    m_delegatePool.addAutoHook(vui::InputDispatcher::key.onKeyUp, [this](Sender s, const KeyEvent& e) { this->onKeyUp(s, e); });
    m_delegatePool.addAutoHook(vui::InputDispatcher::key.onKeyDown, [this](Sender s, const KeyEvent& e) { this->onKeyDown(s, e); });
    m_delegatePool.addAutoHook(vui::InputDispatcher::key.onText, [this](Sender s, const TextEvent& e) { this->onText(s, e); });

    m_isInitialized = true;
    return true;
}

template <class C>
void AwesomiumInterface<C>::destroy() {
    m_webSession->Release();
    m_webView->Destroy();

    // Unregister events
    m_delegatePool.dispose();

    delete m_openglSurfaceFactory;
    m_openglSurfaceFactory = nullptr;
    delete m_data_source;
    m_data_source = nullptr;
    m_isInitialized = false;
}

template <class C>
void AwesomiumInterface<C>::invokeFunction(const cString functionName, const Awesomium::JSArray& args) {
    m_window.ToObject().Invoke(Awesomium::WSLit(functionName), args);
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
    m_webView->InjectKeyboardEvent(keyEvent);
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
    m_webView->InjectKeyboardEvent(keyEvent);
}
template <class C>
void AwesomiumInterface<C>::onText(Sender sender, const TextEvent& e) {
    Awesomium::WebKeyboardEvent keyEvent;

    memcpy(keyEvent.text, e.wtext, 4 * sizeof(wchar_t));
    memcpy(keyEvent.unmodified_text, e.wtext, 4 * sizeof(wchar_t));

    keyEvent.type = Awesomium::WebKeyboardEvent::kTypeChar;
    m_webView->InjectKeyboardEvent(keyEvent);
}

template <class C>
void AwesomiumInterface<C>::update()
{
    m_webCore->Update();

    Awesomium::OpenglSurface* surface = (Awesomium::OpenglSurface*)m_webView->surface();

    m_renderedTexture = surface->getTextureID();

    if (!surface){
        puts("User Interface Error: webView->surface() returned null! Most likely due to erroneous code in the javascript or HTML5.\n");
    }
}

template <class C>
void AwesomiumInterface<C>::draw(vg::GLProgram* program) const
{
    //Check if draw coords were set
    if (m_vbo == 0 || m_renderedTexture == 0) return;

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

    // Bind shader
    program->use();
    program->enableVertexAttribArrays();

    glUniform2f(program->getUniform("unScreenSize"), (f32)m_width, (f32)m_height);
    glUniform2f(program->getUniform("unScreenDisplacement"), 0.0f, 0.0f);
    glUniform1i(program->getUniform("unTexMain"), 0);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_renderedTexture);

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

    m_drawRect = rect;

    if (m_vbo == 0) {
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ibo);

        GLushort elements[6] = { 0, 1, 2, 2, 3, 0 };
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    Vertex2D vertices[4];

    vertices[0].pos.x = (f32)m_drawRect.x;
    vertices[0].pos.y = (f32)(m_drawRect.y + m_drawRect.w);

    vertices[1].pos.x = (f32)m_drawRect.x;
    vertices[1].pos.y = (f32)m_drawRect.y;

    vertices[2].pos.x = (f32)(m_drawRect.x + m_drawRect.z);
    vertices[2].pos.y = (f32)m_drawRect.y;

    vertices[3].pos.x = (f32)(m_drawRect.x + m_drawRect.z);
    vertices[3].pos.y = (f32)(m_drawRect.y + m_drawRect.w);

    for (int i = 0; i < 4; i++) {
        vertices[i].uv[0] = uiBoxUVs[i * 2];
        vertices[i].uv[1] = uiBoxUVs[i * 2 + 1];

        vertices[i].color = m_color;
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

template <class C>
void AwesomiumInterface<C>::setColor(const ColorRGBA8& color) {
    m_color = color;
    // Update the vbo
    setDrawRect(m_drawRect);
}

template <class C>
void CustomJSMethodHandler<C>::OnMethodCall(Awesomium::WebView *caller, unsigned int remote_object_id, const Awesomium::WebString &method_name, const Awesomium::JSArray &args) {
   
    std::cout << "Method call: " << method_name << std::endl;
    IAwesomiumAPI<C>::setptr funcptr = m_api->getVoidFunction(Awesomium::ToString(method_name));
    if (funcptr) {
        ((*m_api).*funcptr)(args);
    }
}

template <class C>
Awesomium::JSValue CustomJSMethodHandler<C>::OnMethodCallWithReturnValue(Awesomium::WebView *caller, unsigned int remote_object_id, const Awesomium::WebString &method_name, const Awesomium::JSArray &args) {
   
    std::cout << "Method call with return value: " << method_name << std::endl;
    IAwesomiumAPI<C>::getptr funcptr = m_api->getFunctionWithReturnValue(Awesomium::ToString(method_name));
    if (funcptr) {
        return ((*m_api).*funcptr)(args);
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
