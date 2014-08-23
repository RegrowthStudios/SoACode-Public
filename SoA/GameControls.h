#pragma once
#include "global.h"
#include "Rendering.h"
#include "Texture2d.h"

static enum TextModes { TEXT_FLOAT, TEXT_INT, TEXT_CUSTOM };

static enum JsonVarString {
	jvNONE, jvNEWGAMENAME, jvLOADGAMENAME, jvSELECTPLANETNAME, jvLODDETAIL, jvVOXELVIEWDISTANCE, jvSCREENRESOLUTION, jvENABLEPARTICLES,
	jvFULLSCREEN, jvMOTIONBLUR, jvMSAA, jvVSYNC, jvBORDERLESSWINDOW, jvGAMMA, jvHDREXPOSURE, jvEFFECTSVOLUME, jvMUSICVOLUME,
	jvFOV, jvMARKERNAME, jvMARKERR, jvMARKERG, jvMARKERB, jvTEXTUREPACK, jvCURRTEXTURERES, jvMOUSESENSITIVITY, jvINVERTMOUSE,
	jvMAXFPS
};

string GetDisplayVarText(int jv);

extern unsigned int BlackButtonTextureID;
extern unsigned int ListBoxDropButtonTextureID;
extern unsigned int SelectionTextureID;

enum HOVERTYPE {HOVERTYPE_NONE, HOVERTYPE_MAINMENU};

using namespace std;

class GameControl
{
public:
	GameControl();
	virtual void Draw();
	Texture2D texture;
	void SetX(int x) {X = x;}
	void SetY(int y) {Y = y;}
	void SetWidth(int Width) {width = Width;}
	void SetHeight(int Height) {height = Height;}
	void SetCallback(int cbType, int (*callBack)(int, string)){
		callback = callBack;
	}
	void AddTextString(string txt){
		texts.push_back(TextInfo(txt));
	}
	void SetDisplayVariable(int dv){
		displayVariable = dv;
		if (texts.size() == 1){
			texts.push_back(TextInfo(""));
			texts[1].justification = 2;
		}
	}
	void SetDisplayVarText(string txt);
	int GetWidth(){ return width; }
	int GetHeight(){ return height; }
	int GetX(){ return X; }
	int GetY(){ return Y; }
	vector <TextInfo> texts;
	Color textColor, hoverTextColor, rightHoverTexColor;
	int jsonVar;
	int generateFrom;
	string generateDir;
	int(*childCallback)(int, string);

	int displayVariable;
	string label;
	int textSize;
protected:
	int (*callback)(int, string);
	int callbackType;
	int X, Y, width, height;
	vector <int> inAnimations;
	vector <int> outAnimations;
};

class CheckBox : public GameControl
{
public:
	CheckBox();
	void Draw(int xmod=0, int ymod=0);
	//returns true if it was clicked
	bool IsClicked(int mx, int my);


	bool active;

	Color frontColor;
	Color backColor;

	int pad;

	Texture2D backTexture, frontTexture;
};

class Button : public GameControl
{
public:
	~Button()
	{
		for (size_t i = 0; i < childButtons.size(); i++){
			delete childButtons[i];
		}
	}
	Button(string s, int w, int h, int x, int y, GLuint Image, int rv);
	int OnClick();
	void SetCoordinates(int x, int y);
	bool IsMouseOver(int mx, int my);
	void wheelInput(int mx, int my, int yscroll);
	void SetJustification(int j){justification = j;}
	void SetTextJustification(int i, int j){if (i < (int)texts.size()) texts[i].justification = j;}
	void SetChildSettings(int ChildPad, int ChildHeight, glm::vec4 lColor){
		childPad = ChildPad;
		childHeight = ChildHeight;
		childBoxLColor = lColor;
	}
	void AddChildButton(string name, GLuint tex, int rv){
		int i = childButtons.size()+1;
		Button *nb = new Button(name, width + childPad, childHeight, X + childPad, Y - childHeight*i - childPad, tex, rv);
		nb->texture.SetColor(glm::vec4(0.0f));
		nb->SetJustification(0);
		nb->SetHovertype(HOVERTYPE_MAINMENU);
		childButtons.push_back(nb);
	}
	void SetRetVal(int rv){
		retVal = rv;
	}
	int GetChildBlockHeight(){
		int rv = (childButtons.size() * childHeight + childPad * 2)*anim;
		if (maxHeight != 0 && maxHeight < rv) rv = maxHeight;
		return rv;
	}

	int Control(int mx, int my, int ymod, Button **hoverButton);
	void Draw(int ymod = 0, int xmod = 0, GLfloat *uvs = NULL);
	void update();
	void SetHover(bool h){hover = h;}
	void SetHovertype(int ht){hoverType = ht;}
	bool IsHover(){return hover;}

	vector <Button *> childButtons;
    void SetActive(bool a){ active = a; if (!active) for (size_t i = 0; i < childButtons.size(); i++) childButtons[i]->SetActive(0); }
	bool GetActive(){return active;}
	void InitializeTexture()
	{
		texture.Initialize(texture.texInfo.ID, X, Y, width, height);
		texture.SetHalfColor(lColor, 0);
		texture.SetHalfColor(rColor, 1);
	}
	void UpdateColor()
	{
		texture.SetHalfColor(lColor, 0);
		texture.SetHalfColor(rColor, 1);
	}

	Color lHoverColor, rHoverColor, lColor, rColor;
	class ScrollBar *scrollBar;
	CheckBox *checkBox;
//protected:
	float anim;
	float animSpeed;
	int justification;
	int hoverType;
	bool active;
	bool alwaysOpen;
	bool hover;
	int childPad;
	int hoverFunc;
	int childHeight;
	int maxHeight;
	int retVal;
	Color childBoxLColor;
	Color childBoxRColor;
};

class ButtonList : public GameControl
{
public:
	ButtonList() :
			lColor(glm::vec4(0.0)),
			rColor(glm::vec4(0.0)),
			rHoverColor(glm::vec4(0.0)),
			lHoverColor(glm::vec4(0.0)),
			generateFrom(0)
	{
		maxHeight = -1;
		yOffset = 0;
		displayInventory = 0;
		padding = 0;
		currFilter = "";
		scrollBar = NULL;
	}
	~ButtonList(){
		Destroy();
	}
	void Destroy(){
        for (size_t i = 0; i < buttons.size(); i++){
			delete buttons[i];
		}
		buttons.clear();
	}

	void Draw(int scrambleStep = 32, int xmod = 0, int ymod = 0);
	int Control(int mx, int my, Button **hoverButton);

	vector <Button *> buttons;
	int maxHeight;
	int yOffset;
	int padding;
	int generateFrom;
	string generateDir;
	float fade;
	string currFilter;
	bool displayInventory;
	Color lHoverColor, rHoverColor, lColor, rColor;
	class ScrollBar *scrollBar;
};

class TextField : public GameControl
{
public:
	TextField(int w, int h, int x, int y, int FontSize, int BorderWidth, glm::vec4 foreGround, glm::vec4 backGround);
	void InitializeTextures();
	void Draw();
	void AddChar(unsigned char c);
	void AddStr(char *str);
	void RemoveChar();
	bool IsMouseOver(int mx, int my);
	void Focus();
	void UnFocus();
	bool IsFocused();
	void Clear();
	void OnChange();

	string text;
//private:
	Color backColor, foreColor;
	int fontSize;
	int borderWidth;
	bool hasFocus;
	Texture2D frontTexture, backTexture;
};

class ScrollBar : public GameControl
{
public:
	ScrollBar(int w, int h, int x, int y, int SlideWidth, bool IsVertical, float MaxValue, float MinValue, float Increment, float startValue, glm::vec4 foreGround, glm::vec4 backGround);
	void Draw(int xmod=0, int ymod=0);
	void ChangeValue(float nval);
	void ChangeMax(float max);
	float GetValue();
	float Control(int mx, int my);
	void wheelInput(int mx, int my, int yscroll);
	void MouseDown(int mx, int my);
	void MouseUp();

	bool isGrabbed;
	bool isVertical;
	float value;
	float maxValue;
	float minValue;
	float increment;
	int slideWidth;
	int textMode;
	string maxValString;

	Color frontColor;
	Color backColor;

	Texture2D slideTexture, barTexture;

};