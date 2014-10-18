#pragma once

#include "SDL\SDL.h"
#include "AwesomiumAPI.h"
#include <Awesomium/BitmapSurface.h>
#include <Awesomium/DataPak.h>
#include <Awesomium/DataSource.h>
#include <Awesomium/STLHelpers.h>
#include <Awesomium/WebCore.h>

#include "OpenglSurfaceFactory.h"

#include "GLProgram.h"

class CustomJSMethodHandler : public Awesomium::JSMethodHandler
{
public:
    CustomJSMethodHandler() : _api(nullptr) {}

    void setAPI(AwesomiumAPI* api) { _api = api; }

    void OnMethodCall(Awesomium::WebView *caller, unsigned int remote_object_id, const Awesomium::WebString &method_name, const Awesomium::JSArray &args);
    Awesomium::JSValue OnMethodCallWithReturnValue(Awesomium::WebView *caller, unsigned int remote_object_id, const Awesomium::WebString &method_name, const Awesomium::JSArray &args);

    Awesomium::JSObject *gameInterface, *jsInterface;


private:
    AwesomiumAPI* _api;
};

class AwesomiumInterface {
public:
    AwesomiumInterface();
    ~AwesomiumInterface();

    bool init(const char *inputDir, int width, int height);

    void handleEvent(const SDL_Event& evnt);

    void update();
    void draw(vcore::GLProgram* program);

    void setDrawRect(int x, int y, int width, int height);
    void setColor(const ColorRGBA8& color);

private:
    int getWebKeyFromSDLKey(SDL_Scancode key);

    bool _isInitialized;

    int _width, _height;

    int _drawWidth, _drawHeight;
    int _drawX, _drawY;

    GLuint _renderedTexture;

    GLuint _vboID;
    GLuint _elementBufferID;

    ColorRGBA8 _color;

    unsigned short _numFiles;

    Awesomium::OpenglSurfaceFactory* _openglSurfaceFactory;

    Awesomium::DataSource* _data_source;
    Awesomium::WebSession* _webSession;
    Awesomium::WebCore* _webCore;
    Awesomium::WebView* _webView;

    Awesomium::JSValue _gameInterface;
    Awesomium::JSValue _jsInterface;

    AwesomiumAPI _awesomiumAPI;

    CustomJSMethodHandler _methodHandler;

    struct Vertex2D {
        f32v2 pos;
        ui8 uv[2];
        ui8 pad[2];
        ColorRGBA8 color;
    };
};