#pragma once

class CustomJSMethodHandler : public Awesomium::JSMethodHandler
{
public:
    void OnMethodCall(Awesomium::WebView *caller, unsigned int remote_object_id, const Awesomium::WebString &method_name, const Awesomium::JSArray &args);
    Awesomium::JSValue OnMethodCallWithReturnValue(Awesomium::WebView *caller, unsigned int remote_object_id, const Awesomium::WebString &method_name, const Awesomium::JSArray &args);

    Awesomium::JSObject *myObject;
};

class AwesomiumInterface {
public:
    AwesomiumInterface();
    ~AwesomiumInterface();

    bool init(const char *inputDir, int width, int height);

    void update();
    void draw();

    void setDrawCoords(int x, int y, int width, int height);
    void setColor(i32v4 color);

private:
    bool _isInitialized;

    int _width, _height;

    GLuint _renderedTexture;

    GLuint _vboID;
    GLuint _elementBufferID;

    ui8 _color[4];

    unsigned short _numFiles;

    Awesomium::DataSource* _data_source;
    Awesomium::WebSession* _webSession;
    Awesomium::WebCore* _webCore;
    Awesomium::WebView* _webView;

    Awesomium::JSValue _jsValue;

    CustomJSMethodHandler _methodHandler;

    struct Vertex2D {
        f32v2 pos;
        ui8 uv[2];
        ui8 pad[2];
        ui8 color[4];
    };
};