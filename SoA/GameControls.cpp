#include "stdafx.h"
#include "GameControls.h"

#include <regex>

#include "Options.h"

unsigned int BlackButtonTextureID;
unsigned int ListBoxDropButtonTextureID;
unsigned int SelectionTextureID;

string GetDisplayVarText(int jv){

	char buf[256];
	switch (jv){
	case jvNEWGAMENAME:
		return menuOptions.newGameString;
		break;
	case jvLOADGAMENAME:
		return menuOptions.loadGameString;
		break;
	case jvSELECTPLANETNAME:
		return menuOptions.selectPlanetName;
		break;
	case jvCURRTEXTURERES:
		return to_string(graphicsOptions.currTextureRes) + " / " + to_string(graphicsOptions.defaultTextureRes);
		break;
	case jvLODDETAIL:
		if (graphicsOptions.lodDetail == 0) return "Ultra";
		if (graphicsOptions.lodDetail == 1) return "High";
		if (graphicsOptions.lodDetail == 2) return "Low";
		if (graphicsOptions.lodDetail == 3) return "Very Low";
		break;
	case jvMARKERR:
		sprintf(buf, "%d", menuOptions.markerR);
		return buf;
	case jvMARKERG:
		sprintf(buf, "%d", menuOptions.markerG);
		return buf;
	case jvMARKERB:
		sprintf(buf, "%d", menuOptions.markerB);
		return buf;
	case jvGAMMA:
		sprintf(buf, "%.2f", graphicsOptions.gamma);
		return buf;
	case jvMAXFPS:
		if (graphicsOptions.maxFPS == 165){
			return "Unlimited";
		}
		sprintf(buf, "%d", graphicsOptions.maxFPS);
		return buf;
	case jvTEXTUREPACK:
		return graphicsOptions.texturePackString;
	case jvFOV:
		sprintf(buf, "%.2f", graphicsOptions.fov);
		return buf;
	case jvHDREXPOSURE:
		sprintf(buf, "%.2f", graphicsOptions.hdrExposure);
		return buf;
	case jvEFFECTSVOLUME:
		sprintf(buf, "%d", soundOptions.effectVolume);
		return buf;
	case jvMUSICVOLUME:
		sprintf(buf, "%d", soundOptions.musicVolume);
		return buf;
	case jvMOUSESENSITIVITY:
		sprintf(buf, "%d", (int)gameOptions.mouseSensitivity);
		return buf;
	case jvINVERTMOUSE:
		sprintf(buf, "%d", (int)gameOptions.invertMouse);
		return buf;
	case jvSCREENRESOLUTION:
		return to_string(graphicsOptions.screenWidth) + " x " + to_string(graphicsOptions.screenHeight);
		break;
	case jvENABLEPARTICLES:
		if (graphicsOptions.enableParticles) return "1";
		return "0";
		break;
	case jvFULLSCREEN:
		if (graphicsOptions.isFullscreen) return "1";
		return "0";
		break;
	case jvMOTIONBLUR:
		if (graphicsOptions.motionBlur == 12) return "High";
		if (graphicsOptions.motionBlur == 8) return "Medium";
		if (graphicsOptions.motionBlur == 4) return "Low";
		return "None";
		break;
	case jvMSAA:
		return to_string(graphicsOptions.msaa) + "x";
		break;
	case jvVSYNC:
		if (graphicsOptions.isVsync) return "1";
		return "0";
		break;
	case jvBORDERLESSWINDOW:
		if (graphicsOptions.isBorderless) return "1";
		return "0";
		break;
	case jvVOXELVIEWDISTANCE:
		if (graphicsOptions.voxelRenderDistance == 48) return "Eh (48)";
		if (graphicsOptions.voxelRenderDistance == 80) return "Puny (80)";
		if (graphicsOptions.voxelRenderDistance == 112) return "Tiny (112)";
		if (graphicsOptions.voxelRenderDistance == 114) return "Small (144)";
		if (graphicsOptions.voxelRenderDistance == 176) return "Normal (176)";
		if (graphicsOptions.voxelRenderDistance == 208) return "Large (208)";
		if (graphicsOptions.voxelRenderDistance == 240) return "Very Large (240)";
		if (graphicsOptions.voxelRenderDistance == 272) return "Huge (272)";
		if (graphicsOptions.voxelRenderDistance == 304) return "Gargantuan (304)";
		if (graphicsOptions.voxelRenderDistance == 336) return "Epic (336)";
		if (graphicsOptions.voxelRenderDistance == 368) return "Legendary (368)";
		if (graphicsOptions.voxelRenderDistance == 400) return "Too far (400)";
		return to_string(graphicsOptions.voxelRenderDistance);
		break;
	}
	return "";
}

GameControl::GameControl() :
		jsonVar(0),
		callback(NULL),
		displayVariable(0),
		textSize(-1),
		label(""),
		textColor(glm::vec4(1.0,1.0,1.0,1.0)),
		hoverTextColor(glm::vec4(0.0,0.0,0.0,1.0)),
		rightHoverTexColor(glm::vec4(39.0 / 255.0, 187.0 / 255.0, 228.0 / 255.0, 1.0)),
		generateFrom(0)
{
}

void GameControl::SetDisplayVarText(string txt)
{
	if (texts.size() > 1){
		texts[1].text = txt;
	}
}

void GameControl::Draw()
{
	
}

CheckBox::CheckBox() : GameControl(), active(0)
{
}

void CheckBox::Draw(int xmod, int ymod)
{
	float mod = 0.7;
	if (active) mod = 2.5;
	frontColor.update(mod);
	frontTexture.Initialize(BlankTextureID.ID, X+pad, Y+pad, width-pad*2, height-pad*2, frontColor);
	backColor.update(mod);
	backTexture.Initialize(BlankTextureID.ID, X, Y, width, height, backColor);
	
	backTexture.Draw();
	frontTexture.Draw();

	glDisable(GL_CULL_FACE);
    for (size_t i = 0; i < texts.size(); i++){
		if (texts[i].text != texts[i].texture.text){
			if (texts[i].justification == 0){
				SetText(texts[i].texture, texts[i].text.c_str(), X+xmod+width+1, Y+(int)(height/2.0f), textSize, 0, texts[i].justification);
			}else if (texts[i].justification == 2){
				SetText(texts[i].texture, texts[i].text.c_str(), X+xmod-1, Y+(int)(height/2.0f), textSize, 0, texts[i].justification);
			}else{
				SetText(texts[i].texture, texts[i].text.c_str(), X+xmod+(int)(width/2.0f), Y+height+1, textSize, 0, texts[i].justification);
			}
		}

		texts[i].Draw(xmod, ymod);
	}
	glEnable(GL_CULL_FACE);
}

bool CheckBox::IsClicked(int mx, int my)
{
	if (mx >= X && mx <= X + width &&
			my >= Y && my <= Y + height){
		active = !active;
		if (callback){
			callback(jsonVar, to_string((int)active));
		}
		return 1;
	}
	return 0;
}

Button::Button(string s, int w, int h, int x, int y, GLuint Image, int rv) :
		GameControl(),
		checkBox(NULL),
		scrollBar(NULL),
		alwaysOpen(0),
		maxHeight(0),
		hoverFunc(0),
		retVal(rv),
		anim(0),
		animSpeed(0.2),
		childPad(0),
		childHeight(0),
		active(0),
		justification(0),
		lColor(glm::vec4(0.0)),
		rColor(glm::vec4(0.0)),
		childBoxRColor(glm::vec4(0.0)),
		childBoxLColor(glm::vec4(0.0)),
		rHoverColor(glm::vec4(0.0)),
		lHoverColor(glm::vec4(0.0)),
		hover(0)
{
	if (s.size()) AddTextString(s);
	width = w;
	height = h;
	X = x;
	Y = y;
	InitializeTexture();
}

int Button::OnClick()
{
	if (active){
		SetActive(0);
	}else{
		SetActive(1);
	}
	if (childButtons.size() == 0){ //callback is inherited if we have children
		if (callback){
			return callback(jsonVar, texts[0].text);
		}
	}

	return 0;
}

void Button::SetCoordinates(int x, int y){
	X = x;
	Y = y;
}

bool Button::IsMouseOver(int mx, int my)
{
	return (mx > X && mx < X + width && my > Y && my < Y + height);
}

void Button::wheelInput(int mx, int my, int yscroll)
{
	if (mx > X && mx < X + width && my > Y - GetChildBlockHeight() && my < Y + height){
		scrollBar->value += yscroll * scrollBar->increment;
		if (scrollBar->value > scrollBar->maxValue) scrollBar->value = scrollBar->maxValue;
		if (scrollBar->value < scrollBar->minValue) scrollBar->value = scrollBar->minValue;
	}
	else{
		scrollBar->wheelInput(mx, my, yscroll);
	}
}

int Button::Control(int mx, int my, int ymod, Button **hoverButton)
{
	update();
	int isHover = 0;
	int scrollMod = 0;
	if (IsMouseOver(mx, my + ymod)){
		*hoverButton = this;
		//	if (b->hover == 0) soundManager.PlayExistingSound("MenuRollover", 0, 1.0f, 0);
		SetHover(1);
	}
	else{
		SetHover(0);
	}
	if (scrollBar){
		if (scrollBar->isGrabbed){
			scrollBar->Control(mx, my);
		}

		ymod -= (scrollBar->maxValue - scrollBar->value)*GetHeight();
		scrollMod = (scrollBar->maxValue - scrollBar->value)*GetHeight();
	}
	if (GetChildBlockHeight()){
        for (size_t j = 0; j < childButtons.size(); j++){
			int by = childButtons[j]->GetY() + scrollMod;
			if ((maxHeight == 0 || (by <= GetY() - GetHeight() + childPad && by - childButtons[j]->GetHeight() >= GetY() - maxHeight - GetHeight() + childPad)) && childButtons[j]->IsMouseOver(mx, my + ymod)){
				*hoverButton = childButtons[j];
				childButtons[j]->SetHover(1);
				isHover = j+1;
			}
			else{
				childButtons[j]->SetHover(0);
			}
		}
	}
	if (isHover){
		return isHover;
	}
	return 0;
}

void Button::Draw(int ymod, int xmod, GLfloat *uvs)
{
	if (!uvs) uvs = boxUVs;
	texture.Draw(xmod, ymod);
	
	Texture2D hoverTex;
	Texture2D childBox;
	Texture2D backGround;

	Color bTextColor = textColor;
	int by;
	
	int cymod = ymod;
	if (scrollBar){
		cymod += (scrollBar->maxValue - scrollBar->value)*GetHeight();
	}

	if (hover){
		hoverTex.Initialize(BlankTextureID.ID, X + xmod, Y + ymod, width, height);
		hoverTex.SetHalfColor(lHoverColor, 0);
		hoverTex.SetHalfColor(rHoverColor, 1);
		if (texts.size()) hoverTex.SetFade(texts[0].texture.fade);
		hoverTex.Draw();
	}

	int bh = GetChildBlockHeight();
	if (bh){
		childBox.Initialize(BlankTextureID.ID, X + xmod, Y + ymod - bh, width, bh);
		childBox.SetHalfColor(childBoxLColor, 0);
		childBox.SetHalfColor(childBoxRColor, 1);
		if (texts.size()) childBox.SetFade(texts[0].texture.fade);
		childBox.Draw();
		glDepthFunc(GL_EQUAL);
        for (size_t j = 0; j < childButtons.size(); j++){

			by = childButtons[j]->GetY() + cymod;
			if (maxHeight == 0 || (by <= Y - height + childPad && by - childButtons[j]->GetHeight() >= Y - maxHeight - height + childPad)){ //if its visible
				childButtons[j]->Draw(cymod, xmod);
			}
		}
		glDepthFunc(GL_LEQUAL);
	}

    for (size_t i = 0; i < texts.size(); i++){

		if (texts[i].text != texts[i].texture.text){
			if (texts[i].justification == 0){
				SetText(texts[i].texture, texts[i].text.c_str(), X+xmod, Y+(int)(height/2.0f), textSize, 0, texts[i].justification);
			}else if (texts[i].justification == 2){
				SetText(texts[i].texture, texts[i].text.c_str(), X+xmod+width, Y+(int)(height/2.0f), textSize, 0, texts[i].justification);
			}else{
				SetText(texts[i].texture, texts[i].text.c_str(), X+xmod+(int)(width/2.0f), Y+(int)(height/2.0f), textSize, 0, texts[i].justification);
			}
		}
	}

	if (hover){
		if (texts.size()) texts[0].texture.SetColor(hoverTextColor);
        for (size_t i = 1; i < texts.size(); i++) texts[i].texture.SetColor(rightHoverTexColor);
	}else{
        for (size_t i = 0; i < texts.size(); i++) texts[i].texture.SetColor(bTextColor);
	}

	glDisable(GL_CULL_FACE);
    for (size_t i = 0; i < texts.size(); i++) texts[i].Draw(xmod, ymod);
	glEnable(GL_CULL_FACE);
}

void Button::update()
{
	if (!active && !alwaysOpen){
        for (size_t i = 0; i < childButtons.size(); i++){
			if (childButtons[i]->active){
				active = 1;
				break;
			}
		}
	}
	if (active){
		anim += animSpeed*glSpeedFactor;
		if (anim > 1.0f) anim = 1.0f;
	}else{
		anim -= animSpeed*glSpeedFactor;
		if (anim < 0.0f) anim = 0.0f;
	}
	if (alwaysOpen) anim = 1.0f;

    for (size_t i = 0; i < childButtons.size(); i++){
		childButtons[i]->update();
	}
}

//***************************************** BUTTON LIST ********************************************

void ButtonList::Draw(int scrambleStep, int xmod, int ymod)
{
	Button *b;
	int by;
	if (scrollBar){
		ymod = (scrollBar->maxValue - scrollBar->value)*GetHeight();
	}
    for (size_t k = 0; k < buttons.size(); k++){
		b = buttons[k];

		if (scrambleStep < 32){
            for (size_t j = 0; j < b->texts.size(); j++) {
				by = b->GetY() + ymod;
				if (maxHeight == -1 || (by <= Y && by-height >= Y-maxHeight)){ //if its visible
					string newText, oldText;
					oldText = b->texts[j].text;
					newText = b->texts[j].text;
                    int i = scrambleStep;
					for (i = scrambleStep-1; i < (int)newText.size() && i < scrambleStep*2-1; i++){
						newText[i] = rand()%93 + 33;
					}
					for (; i < newText.size(); i++){
						newText[i] = ' ';
					}
					b->texts[j].text = newText;
					b->texture.SetFade(fade);
					b->texts[j].texture.SetFade(fade);
				
					b->Draw(ymod, xmod);
				
					b->texts[j].text = oldText;
				}
			}
		}else{
			by = b->GetY() + ymod;
			if (maxHeight == -1 || (by <= Y && by-height >= Y-maxHeight)){ //if its visible
                for (size_t j = 0; j < b->texts.size(); j++) b->texts[j].texture.SetFade(fade);
				b->texture.SetFade(fade);
				b->Draw(ymod, xmod);
			}
		}
			
		ymod -= b->GetChildBlockHeight() + b->GetHeight() + padding;
	}
	if (scrollBar) scrollBar->Draw();
}

int ButtonList::Control(int mx, int my, Button **hoverButton)
{
	int ymod = 0;
    for (size_t j = 0; j < buttons.size(); j++){
		buttons[j]->Control(mx, my, ymod, hoverButton);
		ymod += buttons[j]->GetChildBlockHeight() + buttons[j]->GetHeight() + padding;
	}
	return 0;
}


//***************************************** LIST BOX ***********************************************

//ListBox::ListBox(string s, int w, int h, int x, int z)
//{
//	activeIndex = 0;
//	text = s;
//	cellWidth = w;
//	cellHeight = h;
//	X = x;
//	Z = z;
//	isOpen = 0;
//
//	buttonImage = ListBoxDropButtonTextureID;
//	cellImage = BlackButtonTextureID;
//
//	highlightIndex = -1;
//	listSize = 0;
//
//	cellTextures.resize(1);
//	cellTextures[0].Initialize(cellImage, X, Z, w, h);
//
//	dropButton = new Button("", h, h, X+w, Z, buttonImage, 0);
//
//	highlightTexture.Initialize(SelectionTextureID, 0, 0, cellWidth, cellHeight);
//
//}

//void ListBox::SetVariable()
//{
//	optionVar->SetValue(min + activeIndex*step);
//}

//void ListBox::Reset()
//{
//	int val = (int)(optionVar->GetValue());
//	activeIndex = (val - min)/step; 
////	Close();
//}
//
//int ListBox::Control(int mx, int my, bool mouseClick)
//{
//	bool over = 0;
//	
//	if (mx >= X && mx <= X + cellWidth + cellHeight && my >= Z && my <= Z + cellHeight){ //first box
//		highlightIndex = 0;
//		if (mouseClick){
//			isOpen = !isOpen;
//		}
//		over = 1;
//	}else{ //rest of boxes
//		highlightIndex = -1; 
//		if (isOpen){ 
//			if (mx >= X && mx <= X + cellWidth){
//				for (int i = 0; i < listSize; i++){
//					if (my >= Z-(i+1)*cellHeight && my <= Z - i*cellHeight){
//						over = 1;
//						highlightIndex = (i+1);
//						if (mouseClick){
//							activeIndex = i;
//							return 2;
//						}
//						break;
//					}
//				}
//			}
//		}
//		if (highlightIndex == -1 && mouseClick){
//			isOpen = 0;
//		}
//	}
//
//	if (highlightIndex == -1) return 0;
//
//	return 1;
//}

//void ListBox::Draw()
//{
//	if (isOpen){
//		for (int i = 0; i < cellTextures.size(); i++){
//			cellTextures[i].Draw();
//		}
//	}else{ //draws only first box
//		cellTextures[0].Draw();
//	}
//	dropButton->Draw();
//
//	PrintText(text.c_str(), X-30, Z+(int)(cellHeight/2.5f), cellHeight-1, 0, 2);
//	PrintText(cellTexts[activeIndex].c_str(), X+(int)(cellWidth/2.0f), Z+(int)(cellHeight/2.5f), cellHeight-1, 0, 1);
//
//	if (isOpen){
//		for (unsigned int i = 0; i < cellTexts.size(); i++){
//			PrintText(cellTexts[i].c_str(), X+(int)(cellWidth/2.0f), Z+(int)(cellHeight/2.5f)-cellHeight*(i+1), cellHeight-1, 0, 1);
//		}
//	}
//	if (highlightIndex != -1){
//		highlightTexture.SetPosition(X, Z - highlightIndex*cellHeight);
//		highlightTexture.Draw();
//	}
//}

//void ListBox::InitializeOption(OptionInfo *option)
//{
//	optionVar = option;
//	listSize = optionVar->listNames.size();
//	step = optionVar->step;
//	min = optionVar->min;
//	max = optionVar->max;
//	Reset();
//
//	cellTextures.push_back(Texture2D()); //2 cell textures per listbox. Main cell and drop cell
//	cellTextures.back().Initialize(cellImage, X, Z-cellHeight*listSize, cellWidth, cellHeight*listSize);
//
//	for (int i = 0; i < listSize; i++){
//		cellTexts.push_back(optionVar->listNames[i]);
//	}
//}

//void ListBox::Close()
//{
//	highlightIndex = -1;
//	isOpen = 0;
//}

TextField::TextField(int w, int h, int x, int y, int FontSize, int BorderWidth, glm::vec4 foreGround, glm::vec4 backGround) : GameControl()
{
	X = x;
	Y = y;
	width = w;
	height = h;
	text = "";
	hasFocus = 0;
	fontSize = FontSize;
	borderWidth = BorderWidth;
	backColor = backGround;
	foreColor = foreGround;

	InitializeTextures();
	texts.push_back(TextInfo(""));
}

void TextField::InitializeTextures()
{
	backTexture.Initialize(backTexture.texInfo.ID, X, Y, width, height, backColor);
	frontTexture.Initialize(frontTexture.texInfo.ID, X+borderWidth, Y+borderWidth, width-borderWidth*2, height-borderWidth*2, foreColor);
}

void TextField::Draw()
{
	backTexture.Draw();
	frontTexture.Draw();

	if (hasFocus) text.push_back('|');
	
	if (text != texts[0].texture.text){
		if (borderWidth > 0){
			SetText(texts[0].texture, text.c_str(), X+1+borderWidth, Y+height/2, fontSize, 0, 0);
		}else{
			SetText(texts[0].texture, text.c_str(), X+1, Y+height/2, fontSize, 0, 0);
		}
	}
	glDisable(GL_CULL_FACE);
	texts[0].Draw();
	glEnable(GL_CULL_FACE);
	
	if (hasFocus) text.pop_back();
}

void TextField::AddChar(unsigned char c)
{
	if (text.size() < 16) text.push_back(c);
	OnChange();
}

void TextField::AddStr(char *str){
	if (texts[0].texture.width > width-fontSize-borderWidth) return; //prevent them from adding too much
	text += str;

	//check if the str is a valid filename 
 //   regex e("^(?!^(PRN|AUX|CLOCK\$|NUL|CON|COM\d|LPT\d|\..*)(\..+)?$)[^\x00-\x1f\\?*:\";|/]+$", regex_constants::extended);
 //   bool m = (regex_match(text.c_str(), e));

	OnChange();
}

void TextField::RemoveChar()
{
	if (text.size()) text.pop_back();
	OnChange();
}

bool TextField::IsMouseOver(int mx, int my)
{
	return (mx > X && mx < X + width && my > Y && my < Y + height);
}

void TextField::Focus()
{
	hasFocus = 1;
}

void TextField::UnFocus()
{
	hasFocus = 0;
}

bool TextField::IsFocused()
{
	return hasFocus;
}

void TextField::Clear()
{
	text.clear();
}

void TextField::OnChange()
{
	if (callback){
		callback(jsonVar, text);
	}
}

ScrollBar::ScrollBar(int w, int h, int x, int y, int SlideWidth, bool IsVertical, float MaxValue, float MinValue, float Increment, float startValue, glm::vec4 foreGround, glm::vec4 backGround) :
		GameControl(),
		slideWidth(SlideWidth),
		isVertical(IsVertical),
		maxValue(MaxValue),
		minValue(MinValue),
		increment(Increment),
		value(startValue),
		backColor(backGround),
		frontColor(foreGround),
		isGrabbed(0),
		textMode(TEXT_FLOAT),
		maxValString("")
{
	width = w;
	height = h;
	X = x;
	Y = y;
}

void ScrollBar::Draw(int xmod, int ymod)
{
	float valDiff = maxValue-minValue;
	float ratio;
	if (valDiff){
		ratio = (value-minValue)/(maxValue-minValue);
	}else{
		ratio = 0;
	}

	if (isVertical){
		slideTexture.Initialize(BlankTextureID.ID, X-slideWidth*0.5+width*0.5, Y+height*ratio-slideWidth*0.5, slideWidth, slideWidth, frontColor);
		barTexture.Initialize(BlankTextureID.ID, X, Y, width, height, backColor);
	}else{
		slideTexture.Initialize(BlankTextureID.ID, X+width*ratio-slideWidth*0.5, Y+height*0.5-slideWidth*0.5, slideWidth, slideWidth, frontColor);
		barTexture.Initialize(BlankTextureID.ID, X, Y, width, height, backColor);
	}
	
	barTexture.Draw();
	slideTexture.Draw();

	glDisable(GL_CULL_FACE);
    for (size_t i = 0; i < texts.size(); i++){
		if (texts[i].text != texts[i].texture.text){
			if (isVertical){
				if (texts[i].justification == 0){
					SetText(texts[i].texture, texts[i].text.c_str(), X+xmod+width+1, Y+(int)(height/2.0f)+ymod, textSize, 0, texts[i].justification);
				}else if (texts[i].justification == 2){
					SetText(texts[i].texture, texts[i].text.c_str(), X+xmod-1, Y+(int)(height/2.0f)+ymod, textSize, 0, texts[i].justification);
				}else{
					SetText(texts[i].texture, texts[i].text.c_str(), X+xmod+(int)(width/2.0f)+ymod, Y+height+1, textSize, 0, texts[i].justification);
				}
			}else{
				if (texts[i].justification == 0){
					SetText(texts[i].texture, texts[i].text.c_str(), X+xmod, Y+height+1+ymod+textSize*0.5, textSize, 0, texts[i].justification);
				}else if (texts[i].justification == 2){
					SetText(texts[i].texture, texts[i].text.c_str(), X+xmod+width, Y+height+1+ymod+textSize*0.5, textSize, 0, texts[i].justification);
				}else{
					SetText(texts[i].texture, texts[i].text.c_str(), X+xmod+(int)(width/2.0f), Y+height+1+ymod+textSize*0.5, textSize, 0, texts[i].justification);
				}
			}
		}

		texts[i].Draw(xmod, ymod);
	}
	glEnable(GL_CULL_FACE);
}

void ScrollBar::ChangeValue(float nval)
{
	value = nval;
	if (value > maxValue){
		value = maxValue;
	}else if (value < minValue){
		value = minValue;
	}
}

void ScrollBar::ChangeMax(float max)
{
	maxValue = max;
	if (value > max) value = max;
}

float ScrollBar::GetValue()
{
	return value;
}

float ScrollBar::Control(int mx, int my)
{
	char buf[256];
	float ratio;
	float oldVal = value;
	if (isVertical){
		ratio = (float)(my - Y)/(height); 
	}else{
		ratio = (float)(mx - X)/(width); 
	}
	if (ratio > 1.0){ ratio = 1.0; }else if (ratio < 0){ ratio = 0.0; }
	value = ratio * (maxValue-minValue) + minValue;
	//round to nearest increment
	value /= increment;
	value = glm::round(value);
	value *= increment;

	if (value != oldVal && callback){
		if (textMode == TEXT_CUSTOM){
			callback(jsonVar, to_string(value));
			string s = GetDisplayVarText(jsonVar);
			if (texts.size() > 1) texts[1].text = s;
		}else{
			callback(jsonVar, to_string(value));
			if (maxValString != "" && value == maxValue){
				sprintf(buf, "%s", maxValString.c_str());
			}
			else if (textMode == TEXT_INT){
				sprintf(buf, "%d", (int)value);
			}
			else{
				sprintf(buf, "%.2f", value);
			}
			if (texts.size() > 1) texts[1].text = buf;
		}
	}

	return value;
}

void ScrollBar::wheelInput(int mx, int my, int yscroll)
{
	char buf[256];
	bool inbounds = 0;
	float oldVal = value;
	float valDiff = maxValue - minValue;
	float ratio;
	if (valDiff){
		ratio = (value - minValue) / (maxValue - minValue);
	}
	else{
		ratio = 0;
	}

	if (isVertical){
		if ((mx >= X - slideWidth*0.5 + width*0.5 && mx <= X + slideWidth*0.5 + width*0.5 &&
			my >= Y + height*ratio - slideWidth*0.5 && my <= Y + height*ratio + slideWidth*0.5) || //grab the slide or the bar
			(mx >= X && mx <= X + width && my >= Y && my <= Y + height)){
			inbounds = 1;
		}
	}
	else{
		if ((mx >= X + width*ratio - slideWidth*0.5 && mx <= X + slideWidth*0.5 + width*ratio &&
			my >= Y - slideWidth*0.5 + height*0.5 && my <= Y + slideWidth*0.5 + height*0.5) || //grab the slide or the bar
			(mx >= X && mx <= X + width && my >= Y && my <= Y + height)){
			inbounds = 1;
		}
	}

	if (inbounds){
		value += increment * yscroll;
		if (value > maxValue) value = maxValue;
		if (value < minValue) value = minValue;
	}

	if (value != oldVal && callback){
		callback(jsonVar, to_string(value));
		if (textMode == TEXT_INT){
			sprintf(buf, "%d", (int)value);
		}
		else{
			sprintf(buf, "%.2f", value);
		}
		if (texts.size() > 1) texts[1].text = buf;
	}

}

void ScrollBar::MouseDown(int mx, int my)
{
	float valDiff = maxValue - minValue;
	float ratio;
	if (valDiff){
		ratio = (value - minValue) / (maxValue - minValue);
	}else{
		ratio = 0;
	}

	if (isVertical){
		if ((mx >= X - slideWidth*0.5 + width*0.5 && mx <= X + slideWidth*0.5 + width*0.5 &&
			my >= Y + height*ratio - slideWidth*0.5 && my <= Y + height*ratio + slideWidth*0.5) || //grab the slide or the bar
			(mx >= X && mx <= X + width && my >= Y && my <= Y + height)){
			isGrabbed = 1;
		}
	}
	else{
		if ((mx >= X + width*ratio - slideWidth*0.5 && mx <= X + slideWidth*0.5 + width*ratio &&
			my >= Y - slideWidth*0.5 + height*0.5 && my <= Y + slideWidth*0.5 + height*0.5) || //grab the slide or the bar
			(mx >= X && mx <= X + width && my >= Y && my <= Y + height)){
			isGrabbed = 1;
		}
	}
}

void ScrollBar::MouseUp()
{
	isGrabbed = 0;
}
