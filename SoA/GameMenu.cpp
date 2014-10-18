#include "stdafx.h"
#include "GameMenu.h"

#include <algorithm>

#include <glm\gtx\quaternion.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "BlockData.h"
#include "ChunkManager.h"
#include "FileSystem.h"
#include "GameManager.h"
#include "InputManager.h"
#include "Item.h"
#include "Options.h"
#include "Sound.h"
#include "Texture2d.h"

// TODO: Remove This
using namespace std;

//menus
GameMenu *creditsMenu;
GameMenu *mainMenu;
//ControlsMenu *controlsMenu;
//OptionsMenu *optionsMenu;
GameMenu *pauseMenu;
GameMenu *videoOptionsMenu, *audioOptionsMenu, *gameOptionsMenu;
GameMenu *controlsMenu;
GameMenu *newMarkerMenu, *markerMenu, *deleteMarkerMenu;
InventoryMenu *inventoryMenu;
//InventoryMenu *inventoryMenu;
GameMenu *newGameMenu;
GameMenu *loadGameMenu;
GameMenu *texturePackMenu;
GameMenu *worldEditorSelectMenu;

string mostRecent;

static GLfloat pauseMenuVertices[] = {200, 550, 200, 50, 600, 50, 600, 550};
static GLfloat optionsMenuVertices[] = {150, 550, 150, 50, 650, 50, 650, 550};
static GLfloat inventoryMenuVertices[] = {200, 595, 200, 109, 830, 109, 830, 595};
static GLfloat equipMenuVertices[] = {0, 595, 0, 90, 200, 90, 200, 595};

GameMenu::GameMenu(string jsonfilePath){
    scrambleStep = 1000;
    overlayActive = -1;
    fadingOut = 0.0f;
    player = NULL;
    jsonFilePath = jsonfilePath;
    if (jsonFilePath.size()){
        LoadJsonFile(jsonFilePath.c_str());
    }
    if (scrambleAnimation.active) scrambleStep = 1;
}

void GameMenu::Draw()
{
    float fade = 1.0f;
    if (fadeInAnimation.active){
        fadeInAnimation.anim += animSpeed;
        if (fadeInAnimation.anim > 1.0f) fadeInAnimation.anim = 1.0f;
        fade = fadeInAnimation.anim;
    }
    if (fadingOut && fadeOutAnimation.active){
        fadeOutAnimation.anim += animSpeed;
        if (fadeOutAnimation.anim > 1.0f) fadeOutAnimation.anim = 1.0f;
        fade = 1.0f - fadeOutAnimation.anim;
    }

    glDisable(GL_CULL_FACE);
    for (unsigned int i = 0; i < textures.size(); i++){
        textures[i].SetFade(fade);
        textures[i].Draw();
    }
    glEnable(GL_CULL_FACE);

    if (scrambleStep < 32){
        scrambleAnimation.anim += animSpeed*glSpeedFactor;
        if (scrambleAnimation.anim > 1.0f){
            scrambleAnimation.anim = 0.0f;
            scrambleStep = scrambleStep << 1;
        }
    }else{
        scrambleAnimation.anim = 0.0f;
    }

    Button *b;
    for (unsigned int i = 0; i < buttons.size(); i++){
        b = buttons[i];
        for (size_t j = 0; j < b->texts.size(); j++) b->texts[j].texture.SetFade(fade);
        if (b->displayVariable != jvNONE){
            b->SetDisplayVarText(GetDisplayVarText(b->displayVariable));
        }
        b->texture.SetFade(fade);
        b->Draw();
    }

    for (size_t i = 0; i < buttonLists.size(); i++){
        buttonLists[i]->fade = fade;
        for (size_t k = 0; k < buttonLists[i]->buttons.size(); k++){
            b = buttonLists[i]->buttons[k];
            if (b->displayVariable != jvNONE){
                b->SetDisplayVarText(GetDisplayVarText(b->displayVariable));
            }
        }
        buttonLists[i]->Draw(scrambleStep);

    }

    for (size_t i = 0; i < textFields.size(); i++){
        textFields[i]->Draw();
    }

    for (size_t i = 0; i < scrollBars.size(); i++){
        scrollBars[i]->Draw();
    }

    for (size_t i = 0; i < checkBoxes.size(); i++){
        checkBoxes[i]->Draw();
    }

    if (overlayActive != -1){
        overlays[overlayActive]->Draw();
    }
}

map <string, JsonString> jMap; //keys
map <string, JsonVarString> jvMap; //vals for type variable. used in options

map <JsonString, int (*)(int, string)> jCallBacks;
map <string, Color> uiColors;

int SetVariable(int jvs, string val)
{
    int numRes;
    float fval = 0.0;
    int dval = 0;
    switch (jvs){
        case jvMARKERNAME:
            menuOptions.markerName = val;
            break;
        case jvNEWGAMENAME:
            menuOptions.newGameString = val;
            break;
        case jvLOADGAMENAME:
            menuOptions.loadGameString = val;
            break;
        case jvSELECTPLANETNAME:
            menuOptions.selectPlanetName = val;
            break;
        case jvLODDETAIL:
            if (val == "Ultra"){ graphicsOptions.lodDetail = 0; break; }
            if (val == "High"){ graphicsOptions.lodDetail = 1; break; }
            if (val == "Low"){ graphicsOptions.lodDetail = 2; break; }
            if (val == "Very Low"){ graphicsOptions.lodDetail = 3; break; }
            break;
        case jvMARKERR:
            sscanf(val.c_str(), "%d", &dval);
            menuOptions.markerR = dval;
            break;
        case jvTEXTUREPACK:
            if (graphicsOptions.texturePackString != val){
                graphicsOptions.texturePackString = val;
                if (GameManager::chunkManager) GameManager::chunkManager->remeshAllChunks();
                fileManager.loadTexturePack("Textures/TexturePacks/" + graphicsOptions.texturePackString);
            }
            break;
        case jvMAXFPS:
            sscanf(val.c_str(), "%d", &dval);
            graphicsOptions.maxFPS = dval;
            break;
        case jvMARKERG:
            sscanf(val.c_str(), "%d", &dval);
            menuOptions.markerG = dval;
            break;
        case jvMARKERB:
            sscanf(val.c_str(), "%d", &dval);
            menuOptions.markerB = dval;
            break;
        case jvGAMMA:
            sscanf(val.c_str(), "%f", &fval);
            graphicsOptions.gamma = fval;
            break;
        case jvFOV:
            sscanf(val.c_str(), "%f", &fval);
            graphicsOptions.fov = fval;
            break;
        case jvEFFECTSVOLUME:
            sscanf(val.c_str(), "%f", &fval);
            soundOptions.effectVolume = fval;
            break;
        case jvMUSICVOLUME:
            sscanf(val.c_str(), "%f", &fval);
            soundOptions.musicVolume = fval;
            break;
        case jvMOUSESENSITIVITY:
            sscanf(val.c_str(), "%f", &fval);
            gameOptions.mouseSensitivity = fval;
            break;
        case jvINVERTMOUSE:
            sscanf(val.c_str(), "%d", &dval);
            gameOptions.invertMouse = dval;
            break;
        case jvHDREXPOSURE:
            sscanf(val.c_str(), "%f", &fval);
            graphicsOptions.hdrExposure = fval;
            break;
        case jvSCREENRESOLUTION:
            numRes = SCREEN_RESOLUTIONS.size();
            if (val.substr(0, 6) == "Native"){
                if (graphicsOptions.screenWidth != graphicsOptions.nativeWidth || graphicsOptions.screenHeight != graphicsOptions.nativeHeight){
                    if (graphicsOptions.isFullscreen == 0){ //if its fullscreen then we dont need to rebuild the whole window since size didn't change
                        graphicsOptions.needsWindowReload = 1;
                    }
                    graphicsOptions.needsFboReload = 1;
                    graphicsOptions.screenWidth = graphicsOptions.nativeWidth;
                    graphicsOptions.screenHeight = graphicsOptions.nativeHeight;
                }
                break;
            }
            for (int i = 0; i < numRes; i++){
                if (val == to_string(SCREEN_RESOLUTIONS[i][0]) + " x " + to_string(SCREEN_RESOLUTIONS[i][1])){
            
                    if (graphicsOptions.screenWidth != SCREEN_RESOLUTIONS[i][0] || graphicsOptions.screenHeight != SCREEN_RESOLUTIONS[i][1]){
                        if (graphicsOptions.isFullscreen == 0){
                            graphicsOptions.needsWindowReload = 1;
                        }
                        graphicsOptions.needsFboReload = 1;
                        graphicsOptions.screenWidth = SCREEN_RESOLUTIONS[i][0];
                        graphicsOptions.screenHeight = SCREEN_RESOLUTIONS[i][1];
                    
                    }
                    break;
                }
            }
            break;
        case jvENABLEPARTICLES:
            if (val == "1"){graphicsOptions.enableParticles = 1; break; }
            if (val == "0"){graphicsOptions.enableParticles = 0; break; }
            break;
        case jvFULLSCREEN:
            if (val == "1"){graphicsOptions.isFullscreen = 1; graphicsOptions.needsFullscreenToggle = 1; break; }
            if (val == "0"){ graphicsOptions.isFullscreen = 0; graphicsOptions.needsFullscreenToggle = 1; break; }
            break;
        case jvMOTIONBLUR:
            sscanf(val.c_str(), "%f", &fval);
            dval = fval;
            graphicsOptions.motionBlur = dval; 
            break;
        case jvMSAA:
            sscanf(val.c_str(), "%f", &fval);
            dval = fval;
            if (dval != graphicsOptions.msaa) graphicsOptions.needsFboReload = 1;
            graphicsOptions.msaa = dval;
            break;
        case jvVSYNC:
            if (val == "1"){graphicsOptions.isVsync = 1; SDL_GL_SetSwapInterval(1); break; }
            if (val == "0"){graphicsOptions.isVsync = 0; SDL_GL_SetSwapInterval(0); break; }
            break;
        case jvBORDERLESSWINDOW:
            if (val == "1"){graphicsOptions.isBorderless = 1; graphicsOptions.needsWindowReload=1; break; }
            if (val == "0"){graphicsOptions.isBorderless = 0; graphicsOptions.needsWindowReload=1; break; }
            break;
        case jvVOXELVIEWDISTANCE:
            if (val == "Eh (48)"){graphicsOptions.voxelRenderDistance = 48; break; }
            if (val == "Puny (80)"){graphicsOptions.voxelRenderDistance = 80; break; }
            if (val == "Tiny (112)"){graphicsOptions.voxelRenderDistance = 112; break; }
            if (val == "Small (144)"){graphicsOptions.voxelRenderDistance = 144; break; }
            if (val == "Normal (176)"){graphicsOptions.voxelRenderDistance = 176; break; }
            if (val == "Large (208)"){graphicsOptions.voxelRenderDistance = 208; break; }
            if (val == "Very Large (240)"){graphicsOptions.voxelRenderDistance = 240; break; }
            if (val == "Huge (272)"){graphicsOptions.voxelRenderDistance = 272; break; }
            if (val == "Gargantuan (304)"){graphicsOptions.voxelRenderDistance = 304; break; }
            if (val == "Epic (336)"){graphicsOptions.voxelRenderDistance = 336; break; }
            if (val == "Legendary (368)"){graphicsOptions.voxelRenderDistance = 368; break; }
            if (val == "Too far (400)"){graphicsOptions.voxelRenderDistance = 400; break; }
            break;
    }
    return 0;
}

void BuildButtonFromVariable(Button *b)
{
    int jv = b->jsonVar;

    vector <string> childStrings;
    int numRes;

    switch (jv){
        case jvLODDETAIL:
            childStrings.push_back("Very Low");
            childStrings.push_back("Low");
            childStrings.push_back("High");
            childStrings.push_back("Ultra");
            break;
        case jvSCREENRESOLUTION:
            numRes = SCREEN_RESOLUTIONS.size();
            childStrings.push_back("Native (" + to_string(graphicsOptions.nativeWidth) + " x " + to_string(graphicsOptions.nativeHeight) + ")");
            for (int i = 0; i < numRes; i++){
                childStrings.push_back(to_string(SCREEN_RESOLUTIONS[i][0]) + " x " + to_string(SCREEN_RESOLUTIONS[i][1]));
            }
            break;
        case jvENABLEPARTICLES:
            childStrings.push_back("Enabled");
            childStrings.push_back("Disabled");
            break;
        case jvVOXELVIEWDISTANCE:
            childStrings.push_back("Eh (48)");
            childStrings.push_back("Puny (80)");
            childStrings.push_back("Tiny (112)");
            childStrings.push_back("Small (144)");
            childStrings.push_back("Normal (176)");
            childStrings.push_back("Large (208)");
            childStrings.push_back("Very Large (240)");
            childStrings.push_back("Huge (272)");
            childStrings.push_back("Gargantuan (304)");
            childStrings.push_back("Epic (336)");
            childStrings.push_back("Legendary (368)");
            childStrings.push_back("Too far (400)");
            break;
    }

    Button *nb;

    for (size_t i = 0; i < childStrings.size(); i++){
        nb = new Button(childStrings[i], b->GetWidth()-b->childPad, b->childHeight, b->GetX()+b->childPad, b->GetY()-b->GetHeight()*0.5-b->childPad-i*(b->childHeight), b->texture.texInfo.ID, 0);
        int dI = 0;
        nb->lHoverColor = b->lHoverColor;
        nb->rHoverColor =  b->rHoverColor;
        nb->lColor = b->lColor;
        nb->rColor = b->rColor;
        nb->SetCallback(1, SetVariable);
        nb->textSize = nb->GetHeight()-5;
        nb->jsonVar = jv;
        b->childButtons.push_back(nb);
    }
}

void BuildScrollBarFromVariable(ScrollBar *sb)
{
    int jv = sb->jsonVar;
    switch (jv){
    case jvMARKERR:
        sb->minValue = 0.0f;
        sb->maxValue = 255.0f;
        sb->increment = 1.00f;
        sb->value = menuOptions.markerR;
        sb->AddTextString(GetDisplayVarText(jv));
        sb->texts.back().justification = 2;
        sb->textMode = TEXT_INT;
        break;
    case jvMARKERG:
        sb->minValue = 0.0f;
        sb->maxValue = 255.0f;
        sb->increment = 1.00f;
        sb->textMode = TEXT_INT;
        sb->value = menuOptions.markerG;
        sb->AddTextString(GetDisplayVarText(jv));
        sb->texts.back().justification = 2;
        break;
    case jvMARKERB:
        sb->minValue = 0.0f;
        sb->maxValue = 255.0f;
        sb->increment = 1.00f;
        sb->textMode = TEXT_INT;
        sb->value = menuOptions.markerB;
        sb->AddTextString(GetDisplayVarText(jv));
        sb->texts.back().justification = 2;
        break;
    case jvMAXFPS:
        sb->minValue = 30.0f;
        sb->maxValue = 165.0f;
        sb->increment = 5.0f;
        sb->maxValString = "Unlimited";
        sb->textMode = TEXT_INT;
        sb->value = graphicsOptions.maxFPS;
        sb->AddTextString(GetDisplayVarText(jv));
        sb->texts.back().justification = 2;
        break;
    case jvGAMMA:
        sb->minValue = 0.1f;
        sb->maxValue = 2.0f;
        sb->increment = 0.01f;
        sb->value = graphicsOptions.gamma;
        sb->AddTextString(GetDisplayVarText(jv));
        sb->texts.back().justification = 2;
        break;
    case jvHDREXPOSURE:
        sb->minValue = 0.1f;
        sb->maxValue = 4.0f;
        sb->increment = 0.01f;
        sb->value = graphicsOptions.hdrExposure;
        sb->AddTextString(GetDisplayVarText(jv));
        sb->texts.back().justification = 2;
        break;
    case jvFOV:
        sb->minValue = 60.0f;
        sb->maxValue = 120.0f;
        sb->increment = 1.0f;
        sb->value = graphicsOptions.fov;
        sb->AddTextString(GetDisplayVarText(jv));
        sb->texts.back().justification = 2;
        break;
    case jvEFFECTSVOLUME:
        sb->minValue = 0.0f;
        sb->maxValue = 100.0f;
        sb->increment = 1.0f;
        sb->value = soundOptions.effectVolume;
        sb->AddTextString(GetDisplayVarText(jv));
        sb->texts.back().justification = 2;
        break;
    case jvMUSICVOLUME:
        sb->minValue = 0.0f;
        sb->maxValue = 100.0f;
        sb->increment = 1.0f;
        sb->value = soundOptions.musicVolume;
        sb->AddTextString(GetDisplayVarText(jv));
        sb->texts.back().justification = 2;
        break;
    case jvMOUSESENSITIVITY:
        sb->minValue = 0.0f;
        sb->maxValue = 100.0f;
        sb->increment = 1.0f;
        sb->value = gameOptions.mouseSensitivity;
        sb->textMode = TEXT_INT;
        sb->AddTextString(GetDisplayVarText(jv));
        sb->texts.back().justification = 2;
        break;
    case jvMOTIONBLUR:
        sb->minValue = 0.0f;
        sb->maxValue = 12.0f;
        sb->increment = 4.0f;
        sb->value = graphicsOptions.motionBlur;
        sb->textMode = TEXT_CUSTOM;
        sb->AddTextString(GetDisplayVarText(jv));
        sb->texts.back().justification = 2;
        break;
    case jvMSAA:
        sb->minValue = 0.0f;
        sb->maxValue = graphicsOptions.maxMsaa;
        sb->increment = 1.0f;
        sb->value = graphicsOptions.msaa;
        sb->textMode = TEXT_CUSTOM;
        sb->AddTextString(GetDisplayVarText(jv));
        sb->texts.back().justification = 2;
        break;
    }

}

void InitializeJMap()
{
    jMap["colorDef"] = jCOLORDEF;
    jMap["type"] = jTYPE;
    jMap["color"] = jCOLOR;
    jMap["lColor"] = jLCOLOR;
    jMap["rColor"] = jRCOLOR;
    jMap["x"] = jX;
    jMap["y"] = jY;
    jMap["width"] = jWIDTH;
    jMap["height"] = jHEIGHT;
    jMap["padding"] = jPADDING;
    jMap["lHoverColor"] = jLHOVERCOLOR;
    jMap["rHoverColor"] = jRHOVERCOLOR;
    jMap["func"] = jFUNC;
    jMap["text"] = jTEXT;
    jMap["textField"] = jTEXTFIELD;
    jMap["childPad"] = jCHILDPAD;
    jMap["childHeight"] = jCHILDHEIGHT;
    jMap["buttons"] = jBUTTONS;
    jMap["buttonList"] = jBUTTONLIST;
    jMap["button"] = jBUTTON;
    jMap["texture"] = jTEXTURE;
    jMap["r"] = jR;
    jMap["g"] = jG;
    jMap["b"] = jB;
    jMap["a"] = jA;
    jMap["_comment"] = jCOMMENT;
    jMap["source"] = jSOURCE;
    jMap["hoverColor"] = jHOVERCOLOR;
    jMap["childBoxLColor"] = jCHILDBOXLCOLOR;
    jMap["childBoxRColor"] = jCHILDBOXRCOLOR;
    jMap["childBoxColor"] = jCHILDBOXCOLOR;
    jMap["textColor"] = jTEXTCOLOR;
    jMap["frontColor"] = jFRONTCOLOR;
    jMap["backColor"] = jBACKCOLOR;
    jMap["hoverTextColor"] = jHOVERTEXTCOLOR;
    jMap["animation"] = jANIMATION;
    jMap["animationSpeed"] = jANIMATIONSPEED;
    jMap["justification"] = jJUSTIFICATION;
    jMap["textSize"] = jTEXTSIZE;
    jMap["generateButtonsFrom"] = jGENERATEBUTTONSFROM;
    jMap["slider"] = jSLIDER;
    jMap["callback"] = jCALLBACK;
    jMap["loadGame"] = jLOADGAME;
    jMap["borderSize"] = jBORDERSIZE;
    jMap["variable"] = jVARIABLE;
    jMap["displayVariable"] = jDISPLAYVARIABLE;
    jMap["filter"] = jFILTER;
    jMap["slideSize"] = jSLIDESIZE;
    jMap["isVertical"] = jISVERTICAL;
    jMap["maxHeight"] = jMAXHEIGHT;
    jMap["texCoords"] = jTEXCOORDS;
    jMap["VARIABLE"] = jCAPSVARIABLE;
    jMap["checkBox"] = jCHECKBOX;
    jMap["textJustification"] = jTEXTJUSTIFICATION;
    jMap["alwaysOpen"] = jALWAYSOPEN;
    jMap["maxWidth"] = jMAXWIDTH;
    jMap["hoverFunc"] = jHOVERFUNC;

    jvMap["newGameName"] = jvNEWGAMENAME;
    jvMap["loadGameName"] = jvLOADGAMENAME;
    jvMap["selectPlanetName"] = jvSELECTPLANETNAME;
    jvMap["terrainQuality"] = jvLODDETAIL;
    jvMap["screenResolution"] = jvSCREENRESOLUTION;
    jvMap["voxelViewDistance"] = jvVOXELVIEWDISTANCE;
    jvMap["enableParticles"] = jvENABLEPARTICLES;
    jvMap["fullscreen"] = jvFULLSCREEN;
    jvMap["motionBlur"] = jvMOTIONBLUR;
    jvMap["msaa"] = jvMSAA;
    jvMap["vsync"] = jvVSYNC;
    jvMap["borderlessWindow"] = jvBORDERLESSWINDOW;
    jvMap["gamma"] = jvGAMMA;
    jvMap["hdrExposure"] = jvHDREXPOSURE;
    jvMap["effectsVolume"] = jvEFFECTSVOLUME;
    jvMap["musicVolume"] = jvMUSICVOLUME;
    jvMap["invertMouse"] = jvINVERTMOUSE;
    jvMap["mouseSensitivity"] = jvMOUSESENSITIVITY;
    jvMap["fov"] = jvFOV;
    jvMap["markerName"] = jvMARKERNAME;
    jvMap["markerR"] = jvMARKERR;
    jvMap["markerG"] = jvMARKERG;
    jvMap["markerB"] = jvMARKERB;
    jvMap["texturePackName"] = jvTEXTUREPACK;
    jvMap["currTextureRes"] = jvCURRTEXTURERES;
    jvMap["maxFps"] = jvMAXFPS;
}

void RegisterJCallback(JsonString j, int (*func)(int, string))
{
    jCallBacks[j] = func;
}

inline JsonString GetJval(string s)
{
    auto it = jMap.find(s);
    if (it == jMap.end()){
        return jNONE;
    }else{
        return it->second;
    }
}

inline JsonVarString GetJVval(string s)
{
    auto it = jvMap.find(s);
    if (it == jvMap.end()){
        return jvNONE;
    }else{
        return it->second;
    }
}

int *GetColorPointer(int jvs)
{
    switch (jvs){
    case jvMARKERR:
        return &(menuOptions.markerR);
        break;
    case jvMARKERG:
        return &(menuOptions.markerG);
        break;
    case jvMARKERB:
        return &(menuOptions.markerB);
        break;
    }
    return NULL;
}

bool InitializeMenus()
{
    InitializeJMap();

    creditsMenu = new GameMenu("UI/Credits.json");
    mainMenu = new GameMenu("UI/MainMenu.json");

    GameMenu *ngm = new GameMenu("UI/ExitOverlay.json");
    mainMenu->overlays.push_back(ngm); //exit game
    //ngm = new GameMenu();
    
    newGameMenu = new GameMenu("UI/NewGame.json");
    ngm = new GameMenu("UI/MessageOverlay.json");
    newGameMenu->overlays.push_back(ngm);

    loadGameMenu = new GameMenu("UI/LoadGame.json");    
    ngm = new GameMenu("UI/ConfirmationOverlay.json");
    loadGameMenu->overlays.push_back(ngm);

    videoOptionsMenu = new GameMenu("UI/VideoOptions.json");
    audioOptionsMenu = new GameMenu("UI/AudioOptions.json");
    gameOptionsMenu = new GameMenu("UI/GameOptions.json");
    controlsMenu = new GameMenu("UI/ControlsMenu.json");

    markerMenu = new GameMenu("UI/Markers/MarkerMenu.json");
    newMarkerMenu = new GameMenu("UI/Markers/NewMarkerMenu.json");
    deleteMarkerMenu = new GameMenu("UI/Markers/DeleteMarkerMenu.json");

    //controlsMenu = new ControlsMenu();
    //optionsMenu = new OptionsMenu();
    pauseMenu = new GameMenu("UI/PauseMenu.json");
    inventoryMenu = new InventoryMenu("UI/InventoryMenu.json");

    texturePackMenu = new GameMenu("UI/TexturePackMenu.json");

    worldEditorSelectMenu = new GameMenu("UI/WorldEditorSelect.json");

    return 0;
}

void GameMenu::FreeControls()
{

    for (size_t i = 0; i < buttonLists.size(); i++){
        for (size_t j = 0; j < buttonLists[i]->buttons.size(); j++){
            delete buttonLists[i]->buttons[j];
        }
    }
    buttonLists.clear();

    for (size_t j = 0; j < buttons.size(); j++){
        delete buttons[j];
    }
    buttons.clear();
    textures.clear();

    for (size_t i = 0; i < textFields.size(); i++){
        delete textFields[i];
    }
    textFields.clear();

    for (size_t i = 0; i < checkBoxes.size(); i++){
        delete checkBoxes[i];
    }
    checkBoxes.clear();

    for (size_t i = 0; i < scrollBars.size(); i++){
        delete scrollBars[i];
    }
    scrollBars.clear();
}

void GameMenu::SetOverlayText(string txt)
{
    SetText(textures[1], txt.c_str(), 683, textures[1].ypos, textures[1].textSize, 0, 1);
}

void GameMenu::Open()
{
    Button *nb;
    ButtonList *bl;
    for (size_t i = 0; i < buttonLists.size(); i++){
        //generate from markers
        if (buttonLists[i]->generateFrom == 1){
            bl = buttonLists[i];
            bl->Destroy(); //remake this button list
            for (size_t i = 0; i < GameManager::markers.size(); i++){
                nb = new Button((to_string(i) + ") " + GameManager::markers[i].name), bl->GetWidth(), bl->GetHeight(), bl->GetX(), bl->GetY(), BlankTextureID.ID, 1000 + i);
                int dI = 0;
                nb->texture.texInfo = BlankTextureID;
                nb->lHoverColor = GameManager::markers[i].color;
                nb->rHoverColor = GameManager::markers[i].color;
                nb->lColor = GameManager::markers[i].color;
                nb->rColor = bl->rColor;
                nb->SetTextJustification(0, 0);
                nb->textSize = nb->GetHeight() - 5;
                nb->InitializeTexture();
                bl->buttons.push_back(nb);
            }
        }
    }
    Button *b;
    int childn = 0;
    for (size_t i = 0; i < buttons.size(); i++){
        b = buttons[i];
        vector <string> fnames;
        vector <string> descriptions;
        if (b->displayVariable != jvNONE){
            b->SetDisplayVarText(GetDisplayVarText(b->displayVariable));
        }
        if (b->generateFrom == 2){
            for (size_t j = 0; j < b->childButtons.size(); j++){
                delete b->childButtons[j];
            }
            b->childButtons.clear();
            fileManager.getDirectoryEntries(fnames, descriptions, b->generateDir);
            for (size_t j = 0; j < descriptions.size(); j++){
                nb = new Button("", b->GetWidth() - b->childPad, b->childHeight, b->GetX() + b->childPad, b->GetY() - b->GetHeight()*0.5 - b->childPad - childn*(b->childHeight), b->texture.texInfo.ID, b->retVal);
                int dI = 0;
                nb->lHoverColor = b->lHoverColor;
                nb->rHoverColor = b->rHoverColor;
                nb->lColor = b->lColor;
                nb->rColor = b->rColor;
                nb->AddTextString(fnames[j]);
                nb->SetTextJustification(0, 0);
                nb->AddTextString(descriptions[j]);
                nb->SetTextJustification(1, 2);
                nb->SetCallback(1, b->childCallback);
                nb->jsonVar = b->jsonVar;
                nb->textSize = nb->GetHeight() - 5;
                b->childButtons.push_back(nb);
                childn++;
            }
        }
    }
}

void GameMenu::InitializeControls()
{
    scrambleAnimation = MenuAnimation();
    fadeOutAnimation = MenuAnimation();
    fadeInAnimation = MenuAnimation();
    scrambleStep = 1000;
    LoadJsonFile(jsonFilePath.c_str());
    if (scrambleAnimation.active) scrambleStep = 1;
}

void GameMenu::ReInitializeControls()
{
    FreeControls();
    InitializeControls();
}

//1.28
void GameMenu::FlushControlStates()
{
    fadingOut = 0;
    fadeInAnimation.anim = 0.0f;
    fadeOutAnimation.anim = 0.0f;
    for (size_t i = 0; i < buttons.size(); i++) { buttons[i]->SetActive(0); buttons[i]->SetHover(0); }
    for (size_t i = 0; i < buttonLists.size(); i++){
        for (size_t j = 0; j < buttonLists[i]->buttons.size(); j++){
            buttonLists[i]->buttons[j]->SetActive(0);
            buttonLists[i]->buttons[j]->SetHover(0);
        }
    }
}

int GameMenu::Control()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //force out of wireframe
    drawMode = 0;

    if (overlayActive != -1){
        FlushControlStates();

        return overlays[overlayActive]->Control();
    }

    SDL_PumpEvents();

    SDL_Event evnt;    
    int xpos, ypos;
    SDL_GetMouseState(&xpos, &ypos);

    int mx = (int)(xpos / (float)graphicsOptions.screenWidth * (float)screenWidth2d);
    int my = (int)(screenHeight2d - (ypos / (float)graphicsOptions.screenHeight * (float)screenHeight2d));
    bool updateListBoxes = 1;
    TextField *tf;

    int ymod = 0;
    int bh = 0;
    int rv = 0;
    SDL_StartTextInput();

    hoverButton = NULL;
    for (size_t i = 0; i < buttonLists.size(); i++){
        buttonLists[i]->Control(mx, my, &hoverButton);
    }

    int scrollMod = 0;
    //check hover
    bh = 0;
    string description = "";
    static int oldHover = -1;
    bool descHover = 0;
    for (size_t i = 0; i < buttons.size(); i++){
        rv = buttons[i]->Control(mx, my, ymod, &hoverButton);
        if (rv && buttons[i]->hoverFunc == 1){
            descHover = 1;
            if (oldHover != rv){
                oldHover = rv;
                for (size_t j = 0; j < textures.size(); j++){
                    if (textures[j].label == "descriptionText"){
                        cout << "Textures/TexturePacks/" + buttons[i]->childButtons[rv - 1]->texts[0].text << endl;
                        description = fileManager.loadTexturePackDescription("Textures/TexturePacks/" + buttons[i]->childButtons[rv - 1]->texts[0].text);
                        SetText(textures[j], description.c_str(), textures[j].xpos, textures[j].ypos, textures[j].textSize, 0, 0, textures[j].maxWidth, 0);
                    }
                }
            }
        }
    }    

    if (descHover == 0){
        oldHover = -1;
        for (size_t j = 0; j < textures.size(); j++){
            if (textures[j].label == "descriptionText"){
                cout << "Textures/TexturePacks/" + graphicsOptions.texturePackString << endl;
                description = fileManager.loadTexturePackDescription("Textures/TexturePacks/" + graphicsOptions.texturePackString);
                SetText(textures[j], description.c_str(), textures[j].xpos, textures[j].ypos, textures[j].textSize, 0, 0, textures[j].maxWidth, 0);
            }
        }
    }
    rv = 0;

    for (size_t i = 0; i < scrollBars.size(); i++){
        if (scrollBars[i]->isGrabbed){
            scrollBars[i]->Control(mx, my);
        }
    }

    if (fadingOut){
        if (fadeOutAnimation.anim == 1.0f){
            return 20;
        }else{
            return 0;
        }
    }

    while(SDL_PollEvent(&evnt))
    {
        switch(evnt.type)
        {
        case SDL_QUIT:
            return -6;
        case SDL_MOUSEWHEEL:
            for (size_t i = 0; i < buttons.size(); i++){
                if (buttons[i]->scrollBar != NULL){
                    buttons[i]->wheelInput(mx, my, evnt.wheel.y);
                }
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            GameManager::inputManager->pushEvent(evnt);
            for (size_t i = 0; i < checkBoxes.size(); i++) checkBoxes[i]->IsClicked(mx, my);
            for (size_t i = 0; i < scrollBars.size(); i++) scrollBars[i]->MouseDown(mx, my);

            for (size_t i = 0; i < buttons.size(); i++){
                if (buttons[i]->hover == 0 && !(buttons[i]->scrollBar && buttons[i]->scrollBar->isGrabbed)) buttons[i]->SetActive(0);
            }
            for (size_t i = 0; i < buttonLists.size(); i++){
                for (size_t j = 0; j < buttonLists[i]->buttons.size(); j++){
                    if (buttonLists[i]->buttons[j]->hover == 0) buttonLists[i]->buttons[j]->SetActive(0);
                }
            }

            if (hoverButton){
                if (hoverButton->retVal) rv = hoverButton->retVal;
                hoverButton->OnClick();
            }

            for (size_t i = 0; i < textFields.size(); i++){
                if (textFields[i]->IsMouseOver(mx, my)){
                    textFields[i]->Focus();
                }else{
                    textFields[i]->UnFocus();
                }
            }

            break;
        case SDL_MOUSEBUTTONUP:
            GameManager::inputManager->pushEvent(evnt);
            for (size_t i = 0; i < scrollBars.size(); i++) scrollBars[i]->MouseUp();
            break;
        case SDL_KEYDOWN:
            GameManager::inputManager->pushEvent(evnt);
            if (evnt.key.keysym.sym == SDLK_F10){ //reload the controls
                ReInitializeControls();
            }else if (evnt.key.keysym.sym == SDLK_F11){
                return 945;
            }else if (evnt.key.keysym.sym == SDLK_ESCAPE){
                return -5;
            }

            tf = NULL;
            for (size_t i = 0; i < textFields.size(); i++){
                if (textFields[i]->IsFocused()){
                    tf = textFields[i];
                    break;
                }
            }
            if (tf != NULL){
                if (evnt.key.keysym.sym == SDLK_TAB || evnt.key.keysym.sym == SDLK_ESCAPE){
                    tf->UnFocus();
                }else if (evnt.key.keysym.sym == SDLK_BACKSPACE){
                    tf->RemoveChar();
                }else if (evnt.key.keysym.sym == SDLK_RETURN){
                    tf->UnFocus();
                }
            }
            break;
        case SDL_TEXTINPUT:
        //    printf("%s",evnt.text.text);
            tf = NULL;
            for (size_t i = 0; i < textFields.size(); i++){
                if (textFields[i]->IsFocused()){
                    tf = textFields[i];
                    break;
                }
            }
            
            if (tf) tf->AddStr(evnt.text.text);
            
            break;
        case SDL_KEYUP:
            GameManager::inputManager->pushEvent(evnt);
            break;
        }
    }
    if (rv == 20){
        fadingOut = 1.0f;
        rv = 0;
    }
    for (size_t i = 0; i < textures.size(); i++){
        textures[i].update();
    }
    return rv;
}

#define TYPECHECK if (type == jNONE) { pError(("Error in field " + obj[i].name_ + " type must be first field in object. ").c_str()); throw; }

//recursively process a json object for a game menu
void GameMenu::ProcessJsonObject(json_spirit::Object &obj, string label)
{
    JsonString type = jNONE;
    JsonString jv, jv2;
    JsonVarString jvs;
    string typeS = "";
    string s;
    Color newColor;
    ButtonList *nbl;
    CheckBox *ncb;
    Button *nb;
    int *colp;
    ScrollBar *nsb;
    json_spirit::Object colorObject;
    Texture2D newTex;
    TextField *newTexf;
    for (int i = 0; i < (int)obj.size(); i++){
        mostRecent = obj[i].name_;
        jv = GetJval(mostRecent);
        if (type == jNONE){
            switch (jv){
                case jCOMMENT:
                    break;
                case jTYPE:
                    typeS = obj[i].value_.get_str();
                    type = GetJval(typeS);
                    if (type == NULL){
                        pError("JSON read error. Type field is not a valid type. ");
                        throw 1;
                    }
                    break;
                default:
                    TYPECHECK
            }
        }else{
            switch (type) {
                case jCOLORDEF:
                    jv2 = GetJval(mostRecent);
                    if (jv2 == jCOMMENT) break;
                    if (jv2 != jNONE){
                        pError(("JSON read error. Object name \"" + mostRecent + "\" not allowed with type \"" + typeS + "\"\n").c_str());
                        throw 1;
                    }
                    newColor = Color();
                    colorObject = obj[i].value_.get_obj();
                    for (int i = 0; i < (int)colorObject.size(); i++){
                        if (colorObject[i].value_.type() == json_spirit::str_type){  //dynamic colors from variable
                            s = colorObject[i].value_.get_str();
                            jvs = GetJVval(s);
                            colp = GetColorPointer(jvs);
                            if (colp == NULL){
                                newColor.color[i] = 1.0f;
                            }else{
                                newColor.dynCol[i] = colp;
                            }
                        }else{
                            newColor.color[i] = (colorObject[i].value_.get_int() / 255.0f);
                        }
                    }
                    uiColors[mostRecent] = newColor;
                    break;
                case jTEXTURE:
                    newTex = Texture2D();
                    MakeTexture(obj, &newTex, i);
                    textures.push_back(newTex);
                    break;
                case jBUTTONLIST:
                    nbl = new ButtonList();
                    MakeButtonList(obj, nbl, i);
                    buttonLists.push_back(nbl);
                    break;
                case jBUTTON:
                    nb = new Button("", 10, 10, 100, 100, 0, 0);
                    MakeButton(obj, nb, i);
                    buttons.push_back(nb);
                case jTEXT:
                    newTex = Texture2D();
                    MakeText(obj, newTex, i);
                    newTex.label = label;
                    textures.push_back(newTex);
                    break;
                case jTEXTFIELD:
                    newTexf = new TextField(0, 0, 0, 0, 30, 5, glm::vec4(1.0), glm::vec4(1.0));
                    newTexf->label = label;
                    MakeTextField(obj, newTexf, i);
                    textFields.push_back(newTexf);
                    break;
                case jCHECKBOX:
                    ncb = new CheckBox();
                    MakeCheckBox(obj, ncb, i);
                    checkBoxes.push_back(ncb);
                    break;
                case jSLIDER:
                    nsb = new ScrollBar(0, 0, 0, 0, 10, 1, 1, 0, 1, 0, glm::vec4(1.0), glm::vec4(1.0));
                    MakeScrollBar(obj, nsb, i);
                    scrollBars.push_back(nsb);
                    break;
            }
        }
    }
}

void GameMenu::MakeButtonList(json_spirit::Object &obj, ButtonList *nbl, int &i)
{
    map<string, Color>::iterator cit;
    json_spirit::Array buttonArray;
    Button *nb;
    JsonString jv;
    ScrollBar *ns;
    int jsonVar;
    int (*callBack)(int, string) = NULL;
    int callBackType = 1;
    int tmp;
    int pad;
    int func = 0;
    TextureInfo ti = BlankTextureID;
    pad = 0;

    int justification = 0;
    int size = 0;
    string s;
    
    for ( ; i < (int)obj.size(); i++){
        mostRecent = obj[i].name_;
        jv = GetJval(mostRecent);
        switch (jv){
        case jX:
            nbl->SetX(obj[i].value_.get_int());
            break;
        case jY:
            nbl->SetY(obj[i].value_.get_int());
            break;
        case jWIDTH:
            nbl->SetWidth(obj[i].value_.get_int());
            break;
        case jHEIGHT:
            nbl->SetHeight(obj[i].value_.get_int());
            break;
        case jPADDING:
            pad = obj[i].value_.get_int();
            nbl->padding = pad;
            break;
        case jMAXHEIGHT:
            nbl->maxHeight = obj[i].value_.get_int();
            break;
        case jLHOVERCOLOR:
            cit = uiColors.find(obj[i].value_.get_str());
            if (cit != uiColors.end()) nbl->lHoverColor = cit->second;
            break;
        case jRHOVERCOLOR:
            cit = uiColors.find(obj[i].value_.get_str());
            if (cit != uiColors.end()) nbl->rHoverColor = cit->second;
            break;
        case jLCOLOR:
            cit = uiColors.find(obj[i].value_.get_str());
            if (cit != uiColors.end()) nbl->lColor = cit->second;
            break;
        case jRCOLOR:
            cit = uiColors.find(obj[i].value_.get_str());
            if (cit != uiColors.end()) nbl->rColor = cit->second;
            break;
        case jCOLOR:
            cit = uiColors.find(obj[i].value_.get_str());
            if (cit != uiColors.end()) nbl->rColor = nbl->lColor = cit->second;
            break;
        case jFUNC:
            func = obj[i].value_.get_int();
            break;
        case jSOURCE:
            ti = getTextureInfo(obj[i].value_.get_str());
            break;
        case jFILTER:
            nbl->currFilter = obj[i].value_.get_str();
            break;
        case jSLIDER:
            ns = new ScrollBar(10, 10, 0, 0, 10, 1, 0, 1, 0.1, 0, glm::vec4(1.0), glm::vec4(1.0));
            if (nbl->scrollBar != NULL){
                delete nbl->scrollBar;
            }
            tmp = 0;
            MakeScrollBar(obj[i].value_.get_obj(), ns, tmp);
            nbl->scrollBar = ns;
            break;
        case jJUSTIFICATION:
            if (obj[i].value_.get_str() == "left"){
                justification = 0;
            }
            else if (obj[i].value_.get_str() == "right"){
                justification = 2;
            }
            else{ //centered
                justification = 1;
            }
            break;
        case jBUTTONS:
            buttonArray = obj[i].value_.get_array();
            for (int j = 0; j < buttonArray.size(); j++){
                nb = new Button("", nbl->GetWidth(), nbl->GetHeight(), nbl->GetX(), nbl->GetY(), ti.ID, func);
                int dI = 0;
                nb->texture.texInfo = ti;
                nb->lHoverColor = nbl->lHoverColor;
                nb->rHoverColor = nbl->rHoverColor;
                nb->lColor = nbl->lColor;
                nb->rColor = nbl->rColor;
                MakeButton(buttonArray[j].get_obj(), nb, dI);
                nb->SetTextJustification(0, justification); //this is wrongggg
                nb->SetCallback(1, callBack);
                nbl->buttons.push_back(nb);
                size++;
            }
            break;
        case jCALLBACK:
            jv = GetJval(obj[i].value_.get_str());
            switch (jv){
            case jLOADGAME:
                callBack = jCallBacks[jLOADGAME];
                break;
            }
            break;
        case jVARIABLE:
            jsonVar = GetJVval(obj[i].value_.get_str());
            if (jsonVar != jvNONE){
                callBack = SetVariable;
                callBackType = 1;
            }
            break;
        case jGENERATEBUTTONSFROM:
            s = obj[i].value_.get_str();
            if (s == "INVENTORY"){ //this is an inventory list
                nbl->displayInventory = 1;
            }else if (s == "MARKERS"){
                nbl->generateFrom = 1; 
            }else{ //generate from directory
                vector <string> fnames;
                vector <string> descriptions;
                nbl->generateFrom = 2;
                nbl->generateDir = s;
                fileManager.getDirectoryEntries(fnames, descriptions, s);
                for (int i = 0; i < descriptions.size(); i++){
                    nb = new Button("", nbl->GetWidth(), nbl->GetHeight(), nbl->GetX(), nbl->GetY(), ti.ID, func);
                    int dI = 0;
                    nb->texture.texInfo = ti;
                    nb->lHoverColor = nbl->lHoverColor;
                    nb->rHoverColor = nbl->rHoverColor;
                    nb->lColor = nbl->lColor;
                    nb->rColor = nbl->rColor;
                    nb->AddTextString(fnames[i]);
                    nb->SetTextJustification(0, justification);
                    nb->AddTextString(descriptions[i]);
                    nb->SetTextJustification(1, 2);
                    nb->SetCallback(callBackType, callBack);
                    nb->jsonVar = jsonVar;
                    nbl->buttons.push_back(nb);
                    size++;
                }
            }
            break;
        }
    }
}

void GameMenu::MakeButton(json_spirit::Object &obj, Button *b, int &i)
{
    JsonString jv;
    Button *nb;
    ScrollBar *ns;
    map<string, Color>::iterator cit;
    json_spirit::Array buttonArray;
    int jsonVar;
    int callBackType = 1;
    int tmp;
    int textJustification = 0;
    int func = 0;
    string s;
    int (*callBack)(int, string) = NULL;
    int childn = 0;
    int justification = b->justification;
    b->texture.texInfo = BlankTextureID;
    b->InitializeTexture();
    for ( ; i < obj.size(); i++){
        mostRecent = obj[i].name_;
        jv = GetJval(mostRecent);
        switch (jv){
            case jX:
                b->SetX(obj[i].value_.get_int());
                break;
            case jY:
                b->SetY(obj[i].value_.get_int());
                break;
            case jJUSTIFICATION:
                if (obj[i].value_.get_str() == "left"){
                    justification = 0;
                }else if (obj[i].value_.get_str() == "right"){
                    justification = 2;
                }else{ //centered
                    justification = 1;
                }
                break;
            case jTEXTJUSTIFICATION:
                if (obj[i].value_.get_str() == "left"){
                    textJustification = 0;
                }
                else if (obj[i].value_.get_str() == "right"){
                    textJustification = 2;
                }
                else{ //centered
                    textJustification = 1;
                }
                break;
            case jCALLBACK:
                jv = GetJval(obj[i].value_.get_str());
                switch (jv){
                    case jLOADGAME:
                        callBack = jCallBacks[jLOADGAME];
                    break;
                }
                break;
            case jWIDTH:
                b->SetWidth(obj[i].value_.get_int());
                break;
            case jHEIGHT:
                b->SetHeight(obj[i].value_.get_int());
                break;
            case jCHILDPAD:
                b->childPad = obj[i].value_.get_int();
                break;
            case jCHILDHEIGHT:
                b->childHeight = obj[i].value_.get_int();
                break;
            case jHOVERFUNC:
                if (obj[i].value_.get_str() == "TEXTUREPACK_DESCRIPTION"){
                    b->hoverFunc = 1;
                }
                break;
            case jTEXCOORDS:
                GetTexCoords(obj[i].value_.get_obj(), b->texture);
                break;
            case jLHOVERCOLOR:
                cit = uiColors.find(obj[i].value_.get_str());
                if (cit != uiColors.end()) b->lHoverColor = cit->second;
                break;
            case jRHOVERCOLOR:
                cit = uiColors.find(obj[i].value_.get_str());
                if (cit != uiColors.end()) b->rHoverColor = cit->second;
                break;
            case jHOVERCOLOR:
                cit = uiColors.find(obj[i].value_.get_str());
                if (cit != uiColors.end()) b->rHoverColor = b->lHoverColor = cit->second;
                break;
            case jCHILDBOXLCOLOR:
                cit = uiColors.find(obj[i].value_.get_str());
                if (cit != uiColors.end()) b->childBoxLColor = cit->second;
                break;
            case jCHILDBOXRCOLOR:
                cit = uiColors.find(obj[i].value_.get_str());
                if (cit != uiColors.end()) b->childBoxRColor = cit->second;
                break;
            case jCHILDBOXCOLOR:
                cit = uiColors.find(obj[i].value_.get_str());
                if (cit != uiColors.end()) b->childBoxLColor = b->childBoxRColor = cit->second;
                break;
            case jCOLOR:
                cit = uiColors.find(obj[i].value_.get_str());
                if (cit != uiColors.end()) b->lColor = b->rColor = cit->second;
                break;
            case jLCOLOR:
                cit = uiColors.find(obj[i].value_.get_str());
                if (cit != uiColors.end()) b->lColor = cit->second;
                break;
            case jRCOLOR:
                cit = uiColors.find(obj[i].value_.get_str());
                if (cit != uiColors.end()) b->rColor = cit->second;
                break;
            case jTEXTCOLOR:
                cit = uiColors.find(obj[i].value_.get_str());
                if (cit != uiColors.end()) b->textColor = cit->second;
                break;
            case jHOVERTEXTCOLOR:
                cit = uiColors.find(obj[i].value_.get_str());
                if (cit != uiColors.end()) b->hoverTextColor = cit->second;
                break;
            case jSLIDER:
                ns = new ScrollBar(10, 10, 0, 0, 10, 1, 0, 1, 0.1, 0, glm::vec4(1.0), glm::vec4(1.0));
                if (b->scrollBar != NULL){
                    delete b->scrollBar;
                }
                tmp = 0;
                MakeScrollBar(obj[i].value_.get_obj(), ns, tmp);
                b->scrollBar = ns;
                break;
            case jSOURCE:
                b->texture.texInfo = getTextureInfo(obj[i].value_.get_str());
                break;
            case jTEXT:
                b->AddTextString(obj[i].value_.get_str());
                b->texts.back().justification = textJustification;
                break;
            case jMAXHEIGHT:
                b->maxHeight = obj[i].value_.get_int();
                break;
            case jTEXTSIZE:
                b->textSize = obj[i].value_.get_int();
                break;
            case jFUNC:
                func = obj[i].value_.get_int();
                break;
            case jALWAYSOPEN:
                b->alwaysOpen = obj[i].value_.get_int() == 1;
                break;
            case jBUTTONS:
                buttonArray = obj[i].value_.get_array();
                for (int j = 0; j < buttonArray.size(); j++){
                    nb = new Button("", b->GetWidth()-b->childPad, b->childHeight, b->GetX()+b->childPad, b->GetY()-b->GetHeight()*0.5-b->childPad-childn*(b->childHeight), b->texture.texInfo.ID, func);
                    int dI = 0;
                    nb->lHoverColor = b->lHoverColor;
                    nb->rHoverColor = b->rHoverColor;
                    nb->lColor = b->lColor;
                    nb->rColor = b->rColor;
                    nb->SetTextJustification(0, justification);
                    MakeButton(buttonArray[j].get_obj(), nb, dI);
                    b->childButtons.push_back(nb);
                    childn++;
                }
                break;
            case jVARIABLE:
                jsonVar = GetJVval(obj[i].value_.get_str());
                if (jsonVar != jvNONE){
                    callBack = SetVariable;
                    callBackType = 1;
                }
                b->jsonVar = jsonVar;
                break;
            case jDISPLAYVARIABLE:
                b->SetDisplayVariable(GetJVval(obj[i].value_.get_str()));
                break;
            case jGENERATEBUTTONSFROM:
                s = obj[i].value_.get_str();
                vector <string> fnames;
                vector <string> descriptions;
                bool done = 0;
                if (s == "VARIABLE"){
                    BuildButtonFromVariable(b);
                    done = 1;
                }
                else if (s == "EDITORWORLDS"){
                    nb = new Button("", b->GetWidth() - b->childPad, b->childHeight, b->GetX() + b->childPad, b->GetY() - b->GetHeight()*0.5 - b->childPad - childn*(b->childHeight), b->texture.texInfo.ID, func);
                    int dI = 0;
                    nb->lHoverColor = b->lHoverColor;
                    nb->rHoverColor = b->rHoverColor;
                    nb->lColor = b->lColor;
                    nb->rColor = b->rColor;
                    nb->AddTextString("(Empty Planet)");
                    nb->SetTextJustification(0, justification);
                    nb->SetCallback(callBackType, callBack);
                    nb->jsonVar = jsonVar;
                    nb->textSize = nb->GetHeight() - 5;
                    b->childButtons.push_back(nb);
                    childn++;
                    s = "Worlds/";
                }
                if (!done){
                    b->generateFrom = 2;
                    b->generateDir = s;
                    b->childCallback = callBack;
                    fileManager.getDirectoryEntries(fnames, descriptions, s);
                    for (int i = 0; i < descriptions.size(); i++){
                        nb = new Button("", b->GetWidth()-b->childPad, b->childHeight, b->GetX()+b->childPad, b->GetY()-b->GetHeight()*0.5-b->childPad-childn*(b->childHeight), b->texture.texInfo.ID, func);
                        int dI = 0;
                        nb->lHoverColor = b->lHoverColor;
                        nb->rHoverColor =  b->rHoverColor;
                        nb->lColor = b->lColor;
                        nb->rColor = b->rColor;
                        nb->AddTextString(fnames[i]);
                        nb->SetTextJustification(0, justification);
                        nb->AddTextString(descriptions[i]);
                        nb->SetTextJustification(1, 2);
                        nb->SetCallback(callBackType, callBack);
                        nb->jsonVar = jsonVar;
                        nb->textSize = nb->GetHeight()-5;
                        b->childButtons.push_back(nb);
                        childn++;
                    }
                }
                break;
        }
    }
    if (b->scrollBar){
        int buttonsNotShown = b->childButtons.size() - (b->maxHeight - 2*b->childPad + b->GetHeight()) / b->childHeight + 1;

        if (buttonsNotShown < 0){
            delete b->scrollBar;
            b->scrollBar = NULL;
        }
        else{
            b->scrollBar->value = buttonsNotShown;
            b->scrollBar->maxValue = buttonsNotShown;
            scrollBars.push_back(b->scrollBar);
        }
    }
    if (b->textSize == -1) b->textSize = b->GetHeight()-5;
    if (b->childButtons.size() == 0){
        b->retVal = func;
        b->SetCallback(1, callBack);
    }
    b->texture.SetPosition(b->GetX(), b->GetY(), b->GetWidth(), b->GetHeight());
    b->UpdateColor();
}

void GameMenu::MakeText(json_spirit::Object &obj, Texture2D &tex, int &i){
    JsonString jv;
    string text = "";
    int justification = 0;
    int textSize = 10;
    int maxWidth = 0;
    Color color(glm::vec4(1.0));
    map<string, Color>::iterator cit;
    for ( ; i < obj.size(); i++){
        mostRecent = obj[i].name_;
        jv = GetJval(mostRecent);
        switch (jv){
            case jX:
                tex.SetPosition(obj[i].value_.get_int(), tex.ypos, tex.width, tex.height);
                break;
            case jY:
                tex.SetPosition(tex.xpos, obj[i].value_.get_int(), tex.width, tex.height);
                break;
            case jTEXT:
                text = obj[i].value_.get_str();
                break;
            case jTEXTSIZE:
                textSize = obj[i].value_.get_int();
                break;
            case jMAXWIDTH:
                maxWidth = obj[i].value_.get_int();
                break;
            case jJUSTIFICATION:
                if (obj[i].value_.get_str() == "left"){
                    justification = 0;
                }else if (obj[i].value_.get_str() == "right"){
                    justification = 2;
                }else{ //centered
                    justification = 1;
                }
                break;
            case jTEXTCOLOR:
                cit = uiColors.find(obj[i].value_.get_str());
                if (cit != uiColors.end()) color = cit->second;
                break;
        }
    }
    int yCentered = 1;
    if (maxWidth) yCentered = 0;
    SetText(tex, text.c_str(), tex.xpos, tex.ypos, textSize, 0, justification, maxWidth, yCentered);
    tex.maxWidth = maxWidth;
    tex.SetColor(color);
}

void GameMenu::MakeTexture(json_spirit::Object &obj, Texture2D *tex, int &i)
{
    JsonString jv;
    map<string, Color>::iterator cit;
    tex->texInfo = BlankTextureID;
    string str;
    Color lColor(glm::vec4(1.0)), rColor(glm::vec4(1.0));
    for ( ; i < obj.size(); i++){
        mostRecent = obj[i].name_;
        jv = GetJval(mostRecent);
        switch (jv){
            case jX:
                tex->SetPosition(obj[i].value_.get_int(), tex->ypos, tex->width, tex->height);
                break;
            case jY:
                tex->SetPosition(tex->xpos, obj[i].value_.get_int(), tex->width, tex->height);
                break;
            case jWIDTH:
                tex->width = obj[i].value_.get_int();
                break;
            case jHEIGHT:
                tex->height = obj[i].value_.get_int();
                break;
            case jSOURCE:
                str = obj[i].value_.get_str();
                if (str == "TEXTUREPACK"){
                    tex->texSource = str;
                }else{
                    tex->texInfo = getTextureInfo(str);
                }
                break;
            case jLCOLOR:
                cit = uiColors.find(obj[i].value_.get_str());
                if (cit != uiColors.end()) lColor = cit->second;
                break;
            case jTEXCOORDS:
                GetTexCoords(obj[i].value_.get_obj(), *tex);
                break;
            case jRCOLOR:
                cit = uiColors.find(obj[i].value_.get_str());
                if (cit != uiColors.end()) rColor = cit->second;
                break;
            case jCOLOR:
                cit = uiColors.find(obj[i].value_.get_str());
                if (cit != uiColors.end()) lColor = rColor = cit->second;
                break;
        }
    }
    tex->SetPosition(tex->xpos, tex->ypos, tex->width, tex->height);
    tex->SetHalfColor(lColor, 0);
    tex->SetHalfColor(rColor, 1);

}

void GameMenu::MakeTextField(json_spirit::Object &obj, TextField *texf, int &i) {
    JsonString jv;
    map<string, Color>::iterator cit;
    texf->backTexture.texInfo = BlankTextureID;
    texf->frontTexture.texInfo = BlankTextureID;
    glm::vec4 bColor(1.0), fColor(1.0);
    for ( ; i < obj.size(); i++){
        mostRecent = obj[i].name_;
        jv = GetJval(mostRecent);
        switch (jv){
            case jX:
                texf->SetX(obj[i].value_.get_int());
                break;
            case jY:
                texf->SetY(obj[i].value_.get_int());
                break;
            case jWIDTH:
                texf->SetWidth(obj[i].value_.get_int());
                break;
            case jHEIGHT:
                texf->SetHeight(obj[i].value_.get_int());
                break;
            case jSOURCE:
                texf->frontTexture.texInfo = getTextureInfo(obj[i].value_.get_str());
                break;        
            case jFRONTCOLOR:
                cit = uiColors.find(obj[i].value_.get_str());
                if (cit != uiColors.end()) texf->foreColor = cit->second;
                break;
            case jBACKCOLOR:
                cit = uiColors.find(obj[i].value_.get_str());
                if (cit != uiColors.end()) texf->backColor = cit->second;
                break;
            case jTEXTCOLOR:
                cit = uiColors.find(obj[i].value_.get_str());
                if (cit != uiColors.end()) texf->textColor = cit->second;
                break;
            case jBORDERSIZE:
                texf->borderWidth = obj[i].value_.get_int();
                break;
            case jTEXTSIZE:
                texf->fontSize = obj[i].value_.get_int();
                break;
            case jVARIABLE:
                texf->jsonVar = GetJVval(obj[i].value_.get_str());
                if (texf->jsonVar != jvNONE){
                    texf->SetCallback(1, SetVariable);
                }
                break;
        }
    }
    texf->InitializeTextures();
}

void GameMenu::MakeScrollBar(json_spirit::Object &obj, ScrollBar *ns, int &i)
{
    JsonString jv;
    map<string, Color>::iterator cit;
    int jsonVar;
    int justification = 0;

    glm::vec4 bColor(1.0), fColor(1.0);
    for ( ; i < obj.size(); i++){
        mostRecent = obj[i].name_;
        jv = GetJval(mostRecent);
        switch (jv){
            case jX:
                ns->SetX(obj[i].value_.get_int());
                break;
            case jY:
                ns->SetY(obj[i].value_.get_int());
                break;
            case jWIDTH:
                ns->SetWidth(obj[i].value_.get_int());
                break;
            case jHEIGHT:
                ns->SetHeight(obj[i].value_.get_int());
                break;
            case jFRONTCOLOR:
                cit = uiColors.find(obj[i].value_.get_str());
                if (cit != uiColors.end()) ns->frontColor = cit->second;
                break;
            case jBACKCOLOR:
                cit = uiColors.find(obj[i].value_.get_str());
                if (cit != uiColors.end()) ns->backColor = cit->second;
                break;
            case jSLIDESIZE:
                ns->slideWidth = obj[i].value_.get_int();
                break;
            case jISVERTICAL:
                ns->isVertical = (obj[i].value_.get_int() != 0);
                break;
            case jTEXT:
                ns->AddTextString(obj[i].value_.get_str());
                ns->texts.back().justification = justification;
                break;
            case jTEXTSIZE:
                ns->textSize = obj[i].value_.get_int();
                break;
            case jJUSTIFICATION:
                if (obj[i].value_.get_str() == "left"){
                    justification = 0;
                }else if (obj[i].value_.get_str() == "right"){
                    justification = 2;
                }else{ //centered
                    justification = 1;
                }
                break;
            case jVARIABLE:
                jsonVar = GetJVval(obj[i].value_.get_str());
                if (jsonVar != jvNONE){
                    ns->SetCallback(1, SetVariable);
                }
                ns->jsonVar = jsonVar;
                BuildScrollBarFromVariable(ns);
                break;
        }
    }
}

void GameMenu::MakeCheckBox(json_spirit::Object &obj, CheckBox *cb, int &i)
{
    int justification = 2;
    int jsonVar;
    JsonString jv;
    map<string, Color>::iterator cit;

    glm::vec4 bColor(1.0), fColor(1.0);
    for ( ; i < obj.size(); i++){
        mostRecent = obj[i].name_;
        jv = GetJval(mostRecent);
        switch (jv){
            case jX:
                cb->SetX(obj[i].value_.get_int());
                break;
            case jY:
                cb->SetY(obj[i].value_.get_int());
                break;
            case jWIDTH:
                cb->SetWidth(obj[i].value_.get_int());
                break;
            case jHEIGHT:
                cb->SetHeight(obj[i].value_.get_int());
                break;
            case jFRONTCOLOR:
                cit = uiColors.find(obj[i].value_.get_str());
                if (cit != uiColors.end()) cb->frontColor = cit->second;
                break;
            case jBACKCOLOR:
                cit = uiColors.find(obj[i].value_.get_str());
                if (cit != uiColors.end()) cb->backColor = cit->second;
                break;
            case jVARIABLE:
                jsonVar = GetJVval(obj[i].value_.get_str());
                if (jsonVar != jvNONE){
                    cb->SetCallback(1, SetVariable);
                }
                cb->jsonVar = jsonVar;
                break;
            case jPADDING:
                cb->pad = obj[i].value_.get_int();
                break;
            case jJUSTIFICATION:
                if (obj[i].value_.get_str() == "left"){
                    justification = 0;
                }else if (obj[i].value_.get_str() == "right"){
                    justification = 2;
                }else{ //centered
                    justification = 1;
                }
                break;
            case jTEXT:
                cb->AddTextString(obj[i].value_.get_str());
                cb->texts.back().justification = justification;
                break;
            case jTEXTSIZE:
                cb->textSize = obj[i].value_.get_int();
                break;
        }
    }
    if (cb->textSize == -1) cb->textSize = -5;
    if (cb->jsonVar){
        string rv = GetDisplayVarText(cb->jsonVar);
        if (rv == "1"){
            cb->active = 1;
        }else{
            cb->active = 0;
        }
    }
}

void GameMenu::GetTexCoords(json_spirit::Object &obj, Texture2D &tex)
{
    JsonString jv;
    float ustrt = 0.0f;
    float vstrt = 0.0f;
    float uwidth = 1.0f;
    float vwidth = 1.0f;
    for (int i = 0; i < obj.size() && i < 4; i++){
        mostRecent = obj[i].name_;
        jv = GetJval(mostRecent);
        switch (i){
        case 0:
            ustrt = obj[i].value_.get_int()/(float)tex.texInfo.width;
            break;
        case 1:
            vstrt = obj[i].value_.get_int()/(float)tex.texInfo.height;
            break;
        case 2:
            uwidth = obj[i].value_.get_int()/(float)tex.texInfo.width-ustrt;
            break;
        case 3:
            vwidth = obj[i].value_.get_int()/(float)tex.texInfo.height-vstrt;
            break;
        }
    }
    tex.SetUvs(ustrt, vstrt, uwidth, vwidth);
}

void InventoryMenu::Draw()
{


    float fade = 1.0f;
    if (fadeInAnimation.active){
        fadeInAnimation.anim += animSpeed;
        if (fadeInAnimation.anim > 1.0f) fadeInAnimation.anim = 1.0f;
        fade = fadeInAnimation.anim;
    }
    if (fadingOut && fadeOutAnimation.active){
        fadeOutAnimation.anim += animSpeed;
        if (fadeOutAnimation.anim > 1.0f) fadeOutAnimation.anim = 1.0f;
        fade = 1.0f - fadeOutAnimation.anim;
    }

    glDisable(GL_CULL_FACE);
    for (unsigned int i = 0; i < textures.size(); i++){
        textures[i].SetFade(fade);
        textures[i].Draw();
    }
    glEnable(GL_CULL_FACE);

    if (scrambleStep < 32){
        scrambleAnimation.anim += animSpeed*glSpeedFactor;
        if (scrambleAnimation.anim > 1.0f){
            scrambleAnimation.anim = 0.0f;
            scrambleStep = scrambleStep << 1;
        }
    }
    else{
        scrambleAnimation.anim = 0.0f;
    }

    Button *b;
    for (unsigned int i = 0; i < buttons.size(); i++){
        b = buttons[i];
        for (int j = 0; j < b->texts.size(); j++) b->texts[j].texture.SetFade(fade);
        if (b->displayVariable != jvNONE){
            b->SetDisplayVarText(GetDisplayVarText(b->displayVariable));
        }
        b->texture.SetFade(fade);
        b->Draw();
    }

    for (unsigned int i = 0; i < buttonLists.size(); i++){
        buttonLists[i]->fade = fade;
        for (unsigned int k = 0; k < buttonLists[i]->buttons.size(); k++){
            b = buttonLists[i]->buttons[k];
            if (b->displayVariable != jvNONE){
                b->SetDisplayVarText(GetDisplayVarText(b->displayVariable));
            }
        }
        buttonLists[i]->Draw(scrambleStep);

    }

    for (unsigned int i = 0; i < textFields.size(); i++){
        textFields[i]->Draw();
    }

    for (unsigned int i = 0; i < scrollBars.size(); i++){
        scrollBars[i]->Draw();
    }

    for (unsigned int i = 0; i < checkBoxes.size(); i++){
        checkBoxes[i]->Draw();
    }

    if (overlayActive != -1){
        overlays[overlayActive]->Draw();
    }

    //draw the 3d block
    static double dt = 0.0;
    dt += 0.01*glSpeedFactor;

    if (lastBlockSelected != 0){
        glm::quat quaternion = glm::quat(glm::vec3(0.2 + dt, 0.4 + dt, 0.6 + dt));
        glm::mat4 rotation = glm::toMat4(quaternion);


        glm::mat4 ProjectionMatrix = glm::perspective(graphicsOptions.fov, (float)graphicsOptions.screenWidth / graphicsOptions.screenHeight, 0.1f, 100.0f);
        glm::vec3 emptyVec(0, 0, 0);
        // Camera matrix
        glm::mat4 ViewMatrix = glm::lookAt(
            emptyVec,           // Camera is here
            glm::vec3(0.0, 0.0, -1.0), // and looks here : at the same position, plus "direction"
            glm::vec3(0.0, 1.0, 0.0)                  // Head is up (set to 0,-1,0 to look upside-down)
            );
        //FrustumViewMatrix = glm::lookAt(worldPosition, worldPosition+worldDirection, worldUp);
        glm::mat4 VPb = ProjectionMatrix * ViewMatrix;

        glEnable(GL_CULL_FACE);
        glClear(GL_DEPTH_BUFFER_BIT);
        glClearDepth(1.0);
        Draw3DCube(&(Blocks[lastBlockSelected]), 3, 0, -5, VPb, rotation);
    }
}

void InventoryMenu::MakeInventoryLists(ButtonList *bl, string filter)
{
    blockIDs.clear();
    for (int i = 0; i < bl->buttons.size(); i++){
        delete bl->buttons[i];
    }
    invList = bl;

    for (int i = 0; i < textFields.size(); i++){
        if (textFields[i]->label == invList->currFilter){
            filterTextField = textFields[i];
            break;
        }
    }

    invList->buttons.clear();
    Button *nb;
    string txt;
    bl->currFilter = "";

    bool use;
    string tmp1;

    for (size_t i = 0; i < player->inventory.size(); i++){
        use = 1;
        if (selectedCategory == 1){
            if (player->inventory[i]->type != ITEM_BLOCK) use = 0;
        }else if (selectedCategory == 2){
            if (player->inventory[i]->type != ITEM_WEAPON) use = 0;
        }else if (selectedCategory == 3){
            if (player->inventory[i]->type != ITEM_ARMOR) use = 0;
        }else if (selectedCategory == 4){
            if (player->inventory[i]->type != ITEM_CONSUMABLE) use = 0;
        }else if (selectedCategory == 5){
            if (player->inventory[i]->type != ITEM_MATERIAL) use = 0;
        }else if (selectedCategory == 6){
            if (player->inventory[i]->type != ITEM_MISC) use = 0;
        }
        if (filter.size()){
            tmp1 = player->inventory[i]->name;
            for (size_t s = 0; s < tmp1.size(); s++) {
                if (tmp1[s] >= 'A' && tmp1[s] <= 'Z') tmp1[s] += 'a'-'A';
            }
            if (tmp1.find(filter) == string::npos) use = 0;
        }
        if (!(ObjectList[player->inventory[i]->ID])){
            pError(("Inventory error blocktype " + to_string(player->inventory[i]->ID) + " does not exist!").c_str());
        }else{
            if (use){
            //    if (player->Inventory[i]->count > 1){
            //        txt = player->Inventory[i]->name + " x " + to_string(player->Inventory[i]->count);
        //        }else{
                    txt = player->inventory[i]->name;
            //    }
            //    cout << player->Inventory[i]->name << " " << player->Inventory[i]->ID << endl;
                blockIDs.push_back(player->inventory[i]->ID);
                nb = new Button(txt, bl->GetWidth(), bl->GetHeight(), bl->GetX(), bl->GetY(), BlankTextureID.ID, i+1);
                nb->lHoverColor = bl->lHoverColor;
                nb->rHoverColor = bl->rHoverColor;
                nb->lColor = bl->lColor;
                nb->rColor = bl->rColor;
                nb->textSize = nb->GetHeight()-5;
                bl->buttons.push_back(nb);
            }
        }
    }

    //set up the scrollbar to have the correct scroll values
    if (bl->scrollBar){
        int buttonsNotShown = bl->buttons.size() - (bl->maxHeight)/bl->GetHeight();
    
        if (buttonsNotShown < 0) buttonsNotShown = 0;

        bl->scrollBar->maxValue = buttonsNotShown;
        bl->scrollBar->increment = 0.01f;
        bl->scrollBar->value = buttonsNotShown;
        bl->scrollBar->minValue = 0;
    }
}

void InventoryMenu::UpdateInventoryList()
{
    blockIDs.clear();
    lastBlockSelected = 0;
    for (size_t i = 0; i < invList->buttons.size(); i++){ //DELETE ALL BUTTONS
        delete invList->buttons[i];
    }
    invList->buttons.clear();

    Button *nb;
    string txt;
    string filt;
    bool use;
    string tmp1;

    for (size_t i = 0; i < player->inventory.size(); i++){
        use = 1;
        if (selectedCategory == 1){
            if (player->inventory[i]->type != ITEM_BLOCK) use = 0;
        }else if (selectedCategory == 2){
            if (player->inventory[i]->type != ITEM_WEAPON) use = 0;
        }else if (selectedCategory == 3){
            if (player->inventory[i]->type != ITEM_ARMOR) use = 0;
        }else if (selectedCategory == 4){
            if (player->inventory[i]->type != ITEM_CONSUMABLE) use = 0;
        }else if (selectedCategory == 5){
            if (player->inventory[i]->type != ITEM_MATERIAL) use = 0;
        }else if (selectedCategory == 6){
            if (player->inventory[i]->type != ITEM_MISC) use = 0;
        }
        if (invList->currFilter.size()){
            tmp1 = player->inventory[i]->name;
            for (size_t s = 0; s < tmp1.size(); s++) {
                if (tmp1[s] >= 'A' && tmp1[s] <= 'Z') tmp1[s] += 'a'-'A';
            }
            filt = invList->currFilter;
            for (size_t s = 0; s < filt.size(); s++) {
                if (filt[s] >= 'A' && filt[s] <= 'Z') filt[s] += 'a'-'A';
            }
            if (tmp1.find(filt) == string::npos) use = 0;
        }
        if (!(ObjectList[player->inventory[i]->ID])){
            pError(("Inventory error blocktype " + to_string(player->inventory[i]->ID) + " does not exist!").c_str());
        }else{
            if (use){
        
                txt = player->inventory[i]->name;
                blockIDs.push_back(player->inventory[i]->ID);
                nb = new Button(txt, invList->GetWidth(), invList->GetHeight(), invList->GetX(), invList->GetY(), BlankTextureID.ID, i+1);
                nb->lHoverColor = invList->lHoverColor;
                nb->rHoverColor = invList->rHoverColor;
                nb->lColor = invList->lColor;
                nb->textSize = nb->GetHeight()-5;
                nb->rColor = invList->rColor;
                if (player->inventory[i] == player->leftEquippedItem) {
                    nb->AddTextString("L");
                    nb->texts.back().SetPosMod(400, 0);
                }
            //    nb->texts[1].Set
                if (player->inventory[i] == player->rightEquippedItem) {
                    nb->AddTextString("R");
                    nb->texts.back().SetPosMod(420, 0);
                }
                invList->buttons.push_back(nb);
            }
        }
    }

    //set up the scrollbar to have the correct scroll values
    if (invList->scrollBar){
        int buttonsNotShown = invList->buttons.size() - (invList->maxHeight)/invList->GetHeight();
    
        if (buttonsNotShown < 0) buttonsNotShown = 0;

        invList->scrollBar->maxValue = buttonsNotShown;
        invList->scrollBar->increment = 0.01f;
        if (invList->scrollBar->value > buttonsNotShown){
            invList->scrollBar->value = buttonsNotShown;
        }
        invList->scrollBar->minValue = 0;
    }
}

int InventoryMenu::Control(InventoryArgs &args)
{
    if (overlayActive != -1){
        FlushControlStates();

        return overlays[overlayActive]->Control();
    }

    SDL_PumpEvents();

    SDL_Event evnt;    
    int xpos, ypos;
    SDL_GetMouseState(&xpos, &ypos);

    int mx = (int)(xpos / (float)graphicsOptions.screenWidth * (float)screenWidth2d);
    int my = (int)(screenHeight2d - (ypos / (float)graphicsOptions.screenHeight * (float)screenHeight2d));
    bool updateListBoxes = 1;
    Button *b;
    TextField *tf;

    if (filterTextField){
        if (filterTextField->text != invList->currFilter){
            invList->currFilter = filterTextField->text;
            UpdateInventoryList();
        }
    }
    
    int ymod = 0;
    int bh = 0;
    int rv = 0;
    SDL_StartTextInput();

    hoverButton = NULL;
    for (size_t i = 0; i < buttonLists.size(); i++){
        ymod = 0;
        if (buttonLists[i]->scrollBar){
            if (buttonLists[i]->scrollBar->isGrabbed){
                buttonLists[i]->scrollBar->Control(mx, my);
            }
            ymod = -(buttonLists[i]->scrollBar->maxValue - buttonLists[i]->scrollBar->value)*buttonLists[i]->GetHeight();
        }
        for (size_t j = 0; j < buttonLists[i]->buttons.size(); j++){
            b = buttonLists[i]->buttons[j];
            b->update();
            if (b->IsMouseOver(mx, my + ymod)){
                hoverButton = b;
                lastBlockSelected = blockIDs[j];
                b->SetHover(1);
            }else{
                b->SetHover(0);
            }
            bh = b->GetChildBlockHeight();
            if (bh){
                for (size_t j = 0; j < b->childButtons.size(); j++){
                    if (b->childButtons[j]->IsMouseOver(mx, my + ymod)){
                        hoverButton = b->childButtons[j];
                        b->childButtons[j]->SetHover(1);
                    }else{
                        b->childButtons[j]->SetHover(0);
                    }
                }
            }
            ymod += bh + b->GetHeight() + buttonLists[i]->padding;
        }
    }

    //check hover
    ymod = 0;
    bh = 0;
    for (size_t i = 0; i < buttons.size(); i++){
        b = buttons[i];
        b->update();
        if (b->IsMouseOver(mx, my + ymod)){
            hoverButton = b;
            b->SetHover(1);
        }else{
            b->SetHover(0);
        }
        bh = b->GetChildBlockHeight();
        if (bh){
            for (size_t j = 0; j < buttons[i]->childButtons.size(); j++){
                if (b->childButtons[j]->IsMouseOver(mx, my + ymod)){
                    hoverButton = b->childButtons[j];
                    b->childButtons[j]->SetHover(1);
                }else{
                    b->childButtons[j]->SetHover(0);
                }
            }
        }
        ymod += bh;
    }

    if (fadingOut){
        if (fadeOutAnimation.anim == 1.0f){
            return 20;
        }else{
            return 0;
        }
    }

    while(SDL_PollEvent(&evnt))
    {
        switch(evnt.type)
        {
        case SDL_QUIT:
            exit(19934);
            break;
        case SDL_MOUSEWHEEL:
            invList->scrollBar->ChangeValue(invList->scrollBar->value + evnt.wheel.y);
            break;
        case SDL_MOUSEBUTTONDOWN:

            for (size_t i = 0; i < buttons.size(); i++) buttons[i]->SetActive(0);
            for (size_t i = 0; i < buttonLists.size(); i++){
                if (buttonLists[i]->scrollBar){
                    buttonLists[i]->scrollBar->MouseDown(mx, my);
                }
                for (size_t j = 0; j < buttonLists[i]->buttons.size(); j++){
                    buttonLists[i]->buttons[j]->SetActive(0);
                }
            }

            if (hoverButton){
                if (hoverButton->retVal){
                    rv = hoverButton->retVal;
                    args.selected = rv;
                    if (evnt.button.button == SDL_BUTTON_LEFT){
                        args.arm = 0;
                    }else if (evnt.button.button == SDL_BUTTON_RIGHT){
                        args.arm = 1;
                    }
                }
                hoverButton->OnClick();
            }

            for (size_t i = 0; i < textFields.size(); i++){
                if (textFields[i]->IsMouseOver(mx, my)){
                    textFields[i]->Focus();
                }else{
                    textFields[i]->UnFocus();
                }
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if (evnt.button.button = SDL_BUTTON_LEFT){
                for (size_t i = 0; i < buttonLists.size(); i++){
                    if (buttonLists[i]->scrollBar){
                        buttonLists[i]->scrollBar->MouseUp();
                    }
                }
            }
            break;
        case SDL_KEYDOWN:
            GameManager::inputManager->pushEvent(evnt);
            if (evnt.key.keysym.sym == SDLK_F12){ //reload the controls
                ReInitializeControls();
                InitializeInventory(player);
            }
            else if (evnt.key.keysym.sym == SDLK_TAB || evnt.key.keysym.sym == SDLK_ESCAPE){
                return -1;
            }
            tf = NULL;
            for (size_t i = 0; i < textFields.size(); i++){
                if (textFields[i]->IsFocused()){
                    tf = textFields[i];
                    break;
                }
            }
            if (tf != NULL){
                if (evnt.key.keysym.sym == SDLK_BACKSPACE){
                    tf->RemoveChar();
                }
                else if (evnt.key.keysym.sym == SDLK_TAB || evnt.key.keysym.sym == SDLK_RETURN){
                    tf->UnFocus();
                }
            }
            break;
        case SDL_TEXTINPUT:
        //    printf("%s",evnt.text.text);
            tf = NULL;
            for (size_t i = 0; i < textFields.size(); i++){
                if (textFields[i]->IsFocused()){
                    tf = textFields[i];
                    break;
                }
            }
            
            if (tf) tf->AddStr(evnt.text.text);
            
            break;
        case SDL_KEYUP:
            GameManager::inputManager->pushEvent(evnt);
            break;
        }
    }
    return rv;
}

void GameMenu::LoadJsonFile(const char *filePath)
{
    ifstream is( filePath );
    if (is.fail()){
        perror(filePath);
        pError(((string)"Could not open JSON file " + filePath).c_str());
    }
    mostRecent = "NO TOKEN SET";
    json_spirit::Value value;
    json_spirit::Object ob0;
    try{
        json_spirit::read_or_throw(is, value);
    }catch (json_spirit::Error_position e){
        pError(("JSON format error in " + string(filePath) + ". Line " + to_string(e.line_) + " Column " + to_string(e.column_) + ". " + e.reason_).c_str());
    }
    try{
        ob0 = value.get_obj();
    }catch (exception e){
        pError(((string)"JSON object read exception in " + filePath + ". All values in this file must be encapsulated in a main object. ").c_str());
        is.close();
        return;
    }
            
    try{
        string animstr;
        for (size_t i = 0; i < ob0.size(); i++){
            mostRecent = ob0[i].name_;
            JsonString jval = GetJval(mostRecent);
            switch (jval){
                case jANIMATION:
                    animstr = ob0[i].value_.get_str();
                    if (animstr == "scramble"){
                        scrambleAnimation.active = 1;
                    }else if (animstr == "fadeIn"){
                        fadeInAnimation.active = 1;
                    }else if (animstr == "fadeOut"){
                        fadeOutAnimation.active = 1;
                    }
                    break;
                case jANIMATIONSPEED:
                    animSpeed = ob0[i].value_.get_int()/1000.0f;
                    break;
                default:
                    ProcessJsonObject(ob0[i].value_.get_obj(), mostRecent);
                    break;
            }
        //    int jv = GetJval(name);
        }
    }catch (exception e){
        pError(((string)"JSON read exception in " + filePath + ". Offending token = " + mostRecent + ". You most likely gave a variable an improper value.").c_str());
        is.close();
        return;
    }

    is.close();
}
