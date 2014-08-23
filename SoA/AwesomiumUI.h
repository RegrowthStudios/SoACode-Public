#pragma once
#include <SDL/SDL.h>
#include <Windows.h>
#include <cstdio>
#include <cstdlib>
#include <thread>

#include <Awesomium/BitmapSurface.h>
#include <Awesomium/DataPak.h>
#include <Awesomium/DataSource.h>
#include <Awesomium/STLHelpers.h>
#include <Awesomium/WebCore.h>
#include <Awesomium/WebSession.h>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtx\quaternion.hpp>

#include "FileSystem.h"
#include "global.h"
#include "Texture2d.h"

// TODO: Remove This
using namespace Awesomium;

struct VariableUpdateData
{
	int offset;
	int type;
	string val;
};

struct UIUpdateData
{
	int code;
	int state;
	int id;
	int mode;
	JSArray args;
	string str;
	vector <VariableUpdateData *> variableUpdateData;
};

extern UIUpdateData uiUpdateData;

class CustomJSMethodHandler : public JSMethodHandler
{
public:
	void OnMethodCall (WebView *caller, unsigned int remote_object_id, const WebString &method_name, const JSArray &args);
	JSValue OnMethodCallWithReturnValue (WebView *caller, unsigned int remote_object_id, const WebString &method_name, const JSArray &args);
	JSObject *myObject;
};

class AwesomiumUI
{
public:
	AwesomiumUI();
	~AwesomiumUI();

	bool Initialize(const char *InputDir, int width, int height, int screenWidth, int screenHeight, int drawWidth = -1, int drawHeight = -1);
	void Draw();
	int update();
	void Destroy();
	void InjectMouseMove(int x, int y);
	//0 = left, 1 = middle, 2 = right
	void InjectMouseDown(int mouseButton);
	void InjectMouseUp(int mouseButton);
    void injectMouseWheel(int yMov);
	void InjectKeyboardEvent(const SDL_Event& event);

	bool IsInitialized(){ return isInitialized; }
	int GetWidth(){ return Width; }
	int GetHeight(){ return Height; }
	void SetColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
	void SetDrawPosition(int x, int y);
	void SetDrawDimensions(int w, int h);
	void changeState(int state);

	CustomJSMethodHandler methodHandler;
	Texture2D UItexture1;
	Texture2D ArrowTextureRight, ArrowTextureLeft;
	int mouseX, mouseY;
private:

	int posX, posY;
	int Width, Height;
	int DrawWidth, DrawHeight;
	int ScreenWidth, ScreenHeight;
	bool isInitialized;
	DataSource* data_source;
	WebSession* webSession;
	WebCore *webCore;
	WebView *webView;
	unsigned short nfiles;

	GLfloat vertexColor[16];
	GLfloat vertexPos[8];

	GLuint textureID;

	JSValue my_value;
};

extern AwesomiumUI *currentUserInterface;
