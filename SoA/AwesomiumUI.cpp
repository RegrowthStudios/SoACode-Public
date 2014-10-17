#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "AwesomiumUI.h"

#include <cstdlib>

#include "BlockData.h"
#include "Errors.h"

#include "TerrainGenerator.h"

// TODO: Remove This
using namespace std;

GLushort uiboxDrawIndices[6] = {0,1,2,2,3,0};
GLfloat uiboxUVs[8] = {0, 0, 0, 1, 1, 1, 1, 0};

UIUpdateData uiUpdateData;

AwesomiumUI *currentUserInterface;

int getWebKeyFromSDLKey(SDL_Scancode key);

AwesomiumUI::AwesomiumUI()
{
    mouseX = 0;
    mouseY = 0;
    isInitialized = 0;
    uiUpdateData.code = 0;
    uiUpdateData.str = "";
}

AwesomiumUI::~AwesomiumUI(void)
{
}

inline void addSlider(JSArray &slidersArray, int id1, const char *s, float min, float max, float step, int minMax = 0){
    slidersArray.Push(JSValue(id1));
    slidersArray.Push(WSLit(s));
    slidersArray.Push(JSValue(min));
    slidersArray.Push(JSValue(max));
    slidersArray.Push(JSValue(step));
    slidersArray.Push(JSValue(minMax));
}

inline void addDropBox(JSArray &dropBoxArray, int id1, const char *s, int min, int max){
    dropBoxArray.Push(JSValue(id1));
    dropBoxArray.Push(WSLit(s));
    dropBoxArray.Push(JSValue(min));
    dropBoxArray.Push(JSValue(max));
}

void generateTreeControls(CustomJSMethodHandler &methodHandler)
{
    JSArray d, s;
    JSArray args;
    addSlider(s, TREE_INI_IDCORE, "Core Block", 0, 4096, 1);
    addSlider(s, TREE_INI_IDBARK, "Bark Block", 0, 4096, 1);
    addSlider(s, TREE_INI_IDROOT, "Root Block", 0, 4096, 1);
    addSlider(s, TREE_INI_IDLEAVES, "Leaves Block", 0, 4096, 1);
    addSlider(s, TREE_INI_IDSPECIALBLOCK, "Special Block", 0, 4096, 1);

    addSlider(s, TREE_INI_TRUNKHEIGHT, "Trunk Height", 0, 128, 1, 1);
    addSlider(s, TREE_INI_TRUNKHEIGHTBASE, "Base Height", 0, 128, 1, 1);
    addSlider(s, TREE_INI_TRUNKCOREWIDTH, "Trunk Core Width", 0, 20, 1);
    addSlider(s, TREE_INI_TRUNKWIDTHBASE, "Base Trunk Width", 0, 30, 1, 1);
    addSlider(s, TREE_INI_TRUNKWIDTHMID, "Middle Trunk Width", 0, 30, 1, 1);
    addSlider(s, TREE_INI_TRUNKWIDTHTOP, "Top Trunk Width", 0, 30, 1);

    //instead, drag sliders with an invisible back thats positions represent the trunk widths,
    //draw lines between the slider hands to visually illustrate the effect of moving them.
    //Allow arbitraty numbers of points to be added to the trees
    addSlider(s, TREE_INI_TRUNKSLOPESTART, "Trunk Slope Start", 0, 100, 1);
    addSlider(s, TREE_INI_TRUNKSLOPEEND, "Trunk Slope End", 0, 100, 1);
    addSlider(s, TREE_INI_TRUNKCHANGEDIRCHANCE, "Trunk Change Dir Chance", 0, 1, 0.01f);
    addSlider(s, TREE_INI_BRANCHSTART, "Branch Start", 0, 1.0, 0.01f);

    addDropBox(d, TREE_INI_BRANCHDIRECTIONBOTTOM, "Bottom Branch Direction", 0, 3);
    d.Push(WSLit("Up"));
    d.Push(WSLit("Sideways"));
    d.Push(WSLit("Down"));
    d.Push(WSLit("In Trunk Dir"));

    addDropBox(d, TREE_INI_BRANCHDIRECTIONTOP, "Top Branch Direction", 0, 3);
    d.Push(WSLit("Up"));
    d.Push(WSLit("Sideways"));
    d.Push(WSLit("Down"));
    d.Push(WSLit("In Trunk Dir"));

    addSlider(s, TREE_INI_BRANCHCHANCEBOTTOM, "Bottom Branch Chance", 0, 1, 0.01f, 1);
    addSlider(s, TREE_INI_BRANCHCHANCETOP, "Top Branch Chance", 0, 1, 0.01f, 1);
    addSlider(s, TREE_INI_BRANCHCHANCECAPMOD, "Add Branch Chance At Top", 0, 1.0, 0.01f);
    addSlider(s, TREE_INI_BRANCHLENGTHBOTTOM, "Bottom Branch Length", 0, 100, 1, 1);
    addSlider(s, TREE_INI_BRANCHLENGTHTOP, "Top Branch Length", 0, 100, 1, 1);
    addSlider(s, TREE_INI_BRANCHWIDTHBOTTOM, "Bottom Branch Width", 0, 30, 1, 1);
    addSlider(s, TREE_INI_BRANCHWIDTHTOP, "Top Branch Width", 0, 30, 1, 1);
    addSlider(s, TREE_INI_HASTHICKCAPBRANCHES, "Has Thick Cap Branches", 0, 1, 1);

    addDropBox(d, TREE_INI_LEAFCAPSHAPE, "Leaf Cap Shape", 0, 4);
    d.Push(WSLit("None"));
    d.Push(WSLit("Basic"));
    d.Push(WSLit("Round"));
    d.Push(WSLit("Pine"));
    d.Push(WSLit("Mushroom Cap"));

    addDropBox(d, TREE_INI_BRANCHLEAFSHAPE, "Branch Leaf Shape", 0, 2);
    d.Push(WSLit("None"));
    d.Push(WSLit("Basic"));
    d.Push(WSLit("Round"));

    addSlider(s, TREE_INI_LEAFCAPSIZE, "Leaf Cap Size", 0, 30, 1);
    addSlider(s, TREE_INI_BRANCHLEAFSIZEMOD, "Branch Leaf Size Modifier", 0, 30, 1);
    addSlider(s, TREE_INI_BRANCHLEAFYMOD, "Branch Leaf Y Offset", -5, 5, 1);
    addSlider(s, TREE_INI_DROOPYLEAVESACTIVE, "Has Droopy Leaves", 0, 1, 1);
    addSlider(s, TREE_INI_DROOPYLEAVESSLOPE, "Droopy Leaves Slope", 0, 100, 1);
    addSlider(s, TREE_INI_DROOPYLEAVESDSLOPE, "Droopy Leaves D Slope", 0, 100, 1);
    addSlider(s, TREE_INI_DROOPYLEAVESLENGTH, "Droopy Leaves Length", 0, 100, 1);
    addSlider(s, TREE_INI_MUSHROOMCAPINVERTED, "Has Inverted Mushroom Cap", 0, 1, 1);
    addSlider(s, TREE_INI_MUSHROOMCAPSTRETCHMOD, "Mushroom Cap Stretch", -1, 6, 1);
    addSlider(s, TREE_INI_MUSHROOMCAPCURLLENGTH, "Mushroom Cap Curl Length", 0, 30, 1);
    addSlider(s, TREE_INI_MUSHROOMCAPTHICKNESS, "Mushroom Cap Thickness", 0, 30, 1);
    addSlider(s, TREE_INI_ROOTDEPTH, "Root Depth", 0, 10, 0.01f);

    args.Push(WSLit("Trees"));
    args.Push(d);
    args.Push(s);
    methodHandler.myObject->Invoke(WSLit("GenerateControls"), args);

}

//If success, returns 1, else returns 0
bool AwesomiumUI::Initialize(const char *InputDir, int width, int height, int screenWidth, int screenHeight, int drawWidth, int drawHeight)
{
    int tmp;
    Width = width;
    Height = height;
    ScreenWidth = screenWidth;
    ScreenHeight = screenHeight;

    if (drawWidth == -1 || drawHeight == -1){
        DrawWidth = width;
        DrawHeight = height;
    }else{
        DrawWidth = drawWidth;
        DrawHeight = drawHeight;
    }
    
    UItexture1.Initialize(GuiTextureID, 0, 0, screenWidth/32, screenWidth/32, glm::vec4(1.0f), 1.0f/32.0f, 29.0f/32.0f, 1.0f/32.0f, 1.0f/32.0f);
    ArrowTextureLeft.Initialize(GuiTextureID, screenWidth/6 - screenWidth/32, screenHeight/2 - screenWidth/32,  screenWidth/16, screenWidth/16, glm::vec4(1.0f), 24.0f/32.0f, 28.0f/32.0f, -4.0f/32.0f, 4.0f/32.0f);
    ArrowTextureRight.Initialize(GuiTextureID, screenWidth - screenWidth/6 - screenWidth/32, screenHeight/2 - screenWidth/32, screenWidth/16, screenWidth/16, glm::vec4(1.0f), 20.0f/32.0f, 28.0f/32.0f, 4.0f/32.0f, 4.0f/32.0f);

    SetColor(1.0f,1.0f,1.0f,1.0f);
    SetDrawPosition(0, 0);

    webCore = WebCore::instance();
    if (webCore == 0){
        webCore = WebCore::Initialize(WebConfig());
    }

    webSession = webCore->CreateWebSession(WSLit("WebSession"), WebPreferences());
    webView = webCore->CreateWebView(Width, Height, webSession, Awesomium::kWebViewType_Offscreen);

    if (!webView){
        showMessage("Awesomium WebView could not be created!\n");
        exit(44);
    }

    if (!WriteDataPak(WSLit("UI_Resources.pak"), WSLit(InputDir), WSLit(""), nfiles)){
        std::cerr << "UI Initialization Error: Failed to write UI_Resources.pak\n";
        cin >> tmp;
        return 0;
    }
    
    data_source = new DataPakSource(WSLit("UI_Resources.pak"));

    webSession->AddDataSource(WSLit("UI"), data_source);

    // Load a certain URL into our WebView instance
    WebURL url(WSLit("asset://UI/Index.html"));

    if (!url.IsValid()){
         std::cerr << "UI Initialization Error: URL was unable to be parsed.";
         cin >> tmp;
         return 0;
    }

    webView->LoadURL(url);

    // Wait for our WebView to finish loading
    while (webView->IsLoading())
      webCore->Update();

    // Sleep a bit and update once more to give scripts and plugins
    // on the page a chance to finish loading.
    Sleep(30);
    webCore->Update();
    webView->SetTransparent(1);
    webView->set_js_method_handler(&methodHandler);
    glGenTextures(1, &textureID);

    glBindTexture(GL_TEXTURE_2D, textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);    

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 

    glBindTexture(GL_TEXTURE_2D, 0);

    Error error = webView->last_error();

    if (error) {
        pError("Awesomium error " + to_string(error));
    }

    BitmapSurface* surface = (BitmapSurface*)webView->surface();
    if (!surface){
        showMessage("webView->surface() returned null!");
        exit(131521);
    }
    //surface->SaveToJPEG(WSLit("./UI/storedui.jpg")); //debug

    my_value = webView->ExecuteJavascriptWithResult(WSLit("AppInterface"), WSLit(""));
    if (my_value.IsObject()){

        methodHandler.myObject = &my_value.ToObject();
        methodHandler.myObject->SetCustomMethod(WSLit("Quit"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("New"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("Save"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("SaveAs"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("ChangeVariable"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("GenerateNewSeed"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("OpenFileDialog"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("ChangeName"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("RequestChangeState"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("Regenerate"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("RequestBlockList"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("RequestBlock"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("ZoomToBlock"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("ChangeBlockID"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("ChangeTexture"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("ResizeAltColors"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("CheckBoxInput"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("SetTemperature"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("SetRainfall"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("ReloadTextures"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("RequestNoise"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("Cancel"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("Back"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("AddNoise"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("ChangeNoisePosition"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("DeleteNoise"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("Help"), false);
        methodHandler.myObject->SetCustomMethod(WSLit("SetType"), false);

        JSArray dropBoxArray, slidersArray, namesArray;
        JSArray args;

        generateTreeControls(methodHandler);

        args.Clear();
        //********************* BLOCK VARIABLES BEGIN ******************************************
        dropBoxArray.Clear();
        slidersArray.Clear();
        for (auto i = blockVariableMap.begin(); i != blockVariableMap.end(); i++){
            if (i->second.editorAccessible){
                if (i->second.controlType == 1){ //drop box
                    dropBoxArray.Push(JSValue(i->second.byteOffset));
                    dropBoxArray.Push(WSLit(i->first.c_str()));
                    dropBoxArray.Push(JSValue(i->second.min));
                    dropBoxArray.Push(JSValue(i->second.max));
                    for (size_t j = 0; j < i->second.listNames.size(); j++){
                        dropBoxArray.Push(WSLit(i->second.listNames[j].c_str()));
                    }
                }else if (i->second.controlType == 0){ //slider
                    slidersArray.Push(JSValue(i->second.byteOffset));
                    slidersArray.Push(WSLit(i->first.c_str()));
                    slidersArray.Push(JSValue(i->second.min));
                    slidersArray.Push(JSValue(i->second.max));
                    slidersArray.Push(JSValue(i->second.step));
                    slidersArray.Push(JSValue(-1)); //no double sliders needed
                }
            }
        }
        args.Push(WSLit("Blocks"));
        args.Push(dropBoxArray);
        args.Push(slidersArray);
        methodHandler.myObject->Invoke(WSLit("GenerateControls"), args);
        //********************* BLOCK VARIABLES END ******************************************
        args.Clear();
        //********************* BIOME VARIABLES BEGIN ******************************************
        dropBoxArray.Clear();
        slidersArray.Clear();
        for (auto i = fileManager.biomeVariableMap.begin(); i != fileManager.biomeVariableMap.end(); i++){
            if (i->second.editorAccessible){
                if (i->second.controlType == 1){ //drop box
                    dropBoxArray.Push(JSValue(i->second.byteOffset));
                    dropBoxArray.Push(WSLit(i->first.c_str()));
                    dropBoxArray.Push(JSValue(i->second.min));
                    dropBoxArray.Push(JSValue(i->second.max));
                    for (size_t j = 0; j < i->second.listNames.size(); j++){
                        dropBoxArray.Push(WSLit(i->second.listNames[j].c_str()));
                    }
                }else if (i->second.controlType == 0){ //slider
                    slidersArray.Push(JSValue(i->second.byteOffset));
                    slidersArray.Push(WSLit(i->first.c_str()));
                    slidersArray.Push(JSValue(i->second.min));
                    slidersArray.Push(JSValue(i->second.max));
                    slidersArray.Push(JSValue(i->second.step));
                    slidersArray.Push(JSValue(i->second.byteOffset2));
                }
            }
        }
        args.Push(WSLit("Biomes"));
        args.Push(dropBoxArray);
        args.Push(slidersArray);
        methodHandler.myObject->Invoke(WSLit("GenerateControls"), args);
        //********************* BIOME VARIABLES END ******************************************
        args.Clear();
        //********************* NOISE VARIABLES BEGIN ******************************************
        dropBoxArray.Clear();
        slidersArray.Clear();
        for (auto i = fileManager.noiseVariableMap.begin(); i != fileManager.noiseVariableMap.end(); i++){
            if (i->second.controlType == 1){ //drop box
                dropBoxArray.Push(JSValue(i->second.byteOffset));
                dropBoxArray.Push(WSLit(i->first.c_str()));
                dropBoxArray.Push(JSValue(i->second.min));
                dropBoxArray.Push(JSValue(i->second.max));
                for (size_t j = 0; j < i->second.listNames.size(); j++){
                    dropBoxArray.Push(WSLit(i->second.listNames[j].c_str()));
                }
            }else if (i->second.controlType == 0){ //slider
                slidersArray.Push(JSValue(i->second.byteOffset));
                slidersArray.Push(WSLit(i->first.c_str()));
                slidersArray.Push(JSValue(i->second.min));
                slidersArray.Push(JSValue(i->second.max));
                slidersArray.Push(JSValue(i->second.step));
                slidersArray.Push(JSValue(i->second.byteOffset2));
            }    
        }
        args.Push(WSLit("Noise-Mod"));
        args.Push(dropBoxArray);
        args.Push(slidersArray);
        methodHandler.myObject->Invoke(WSLit("GenerateControls"), args);
        //********************* NOISE VARIABLES END ******************************************
        args.Clear();
        //********************* TERRAIN TYPES BEGIN ******************************************
        namesArray.Clear();
        for (int i = 0; i < NumTerrainFunctions; i++){
            namesArray.Push(JSValue(i));
            namesArray.Push(WSLit(TerrainFunctionNames[i].c_str()));
        }
        args.Push(namesArray);
        methodHandler.myObject->Invoke(WSLit("SetNoiseTypes"), args);
        //********************* TERRAIN TYPES END ********************************************
        args.Clear();
        //********************* CLIMATE TYPES BEGIN ******************************************
        dropBoxArray.Clear();
        slidersArray.Clear();

        slidersArray.Push(JSValue(0));
        slidersArray.Push(WSLit("Base Temperature"));
        slidersArray.Push(JSValue(-100));
        slidersArray.Push(JSValue(200));
        slidersArray.Push(JSValue(1));
        slidersArray.Push(JSValue(-1));

        slidersArray.Push(JSValue(1));
        slidersArray.Push(WSLit("Base Rainfall"));
        slidersArray.Push(JSValue(-100));
        slidersArray.Push(JSValue(200));
        slidersArray.Push(JSValue(1));
        slidersArray.Push(JSValue(-1));

        args.Push(WSLit("Climate"));
        args.Push(dropBoxArray);
        args.Push(slidersArray);
        methodHandler.myObject->Invoke(WSLit("GenerateControls"), args);
        //********************* CLIMATE TYPES END ******************************************

        
    }else{
        showMessage("Could not initialize web view object! It is most likely a problem in AppInterface.js. Did you break something???\n");
        return 0;
    }
    isInitialized = 1;
    return 1;
}

int AwesomiumUI::update()
{
    webCore->Update();

    BitmapSurface* surface = (BitmapSurface*)webView->surface();

    if(surface && surface->is_dirty())
    {
        // renders the surface buffer to the opengl texture!
        glBindTexture( GL_TEXTURE_2D, textureID );
        glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, surface->width(), surface->height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, surface->buffer());
        surface->set_is_dirty(0);
    }
    if (!surface){
        fflush(stdout);
        showMessage("User Interface Error: webView->surface() returned null! Most likely due to erroneous code in the javascript or HTML5.\n");
    }
    return 1;
}

void AwesomiumUI::Draw()
{
    glBindBuffer(GL_ARRAY_BUFFER, texture2Dshader.Text2DVertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPos), vertexPos, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, texture2Dshader.Text2DUVBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uiboxUVs), uiboxUVs, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, texture2Dshader.Text2DElementBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uiboxDrawIndices), uiboxDrawIndices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, texture2Dshader.Text2DColorBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexColor), vertexColor, GL_STATIC_DRAW);

    // Bind shader
    texture2Dshader.Bind((GLfloat)ScreenWidth, (GLfloat)ScreenHeight);
    glUniform1f(texture2Dshader.Text2DUseRoundMaskID, 0.0f);
    
    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set our "myTextureSampler" sampler to user Texture Unit 0
    glUniform1i(texture2Dshader.Text2DUniformID, 0);

    glBindBuffer(GL_ARRAY_BUFFER, texture2Dshader.Text2DVertexBufferID);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );

    // 2nd attribute buffer : UVs
    glBindBuffer(GL_ARRAY_BUFFER, texture2Dshader.Text2DUVBufferID);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );

    // 3rd attribute buffer : Colors
    glBindBuffer(GL_ARRAY_BUFFER, texture2Dshader.Text2DColorBufferID);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0 );

    // Draw call
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texture2Dshader.Text2DElementBufferID);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    texture2Dshader.UnBind();
}

void AwesomiumUI::SetColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    for (int i = 0; i < 16; i+=4){
        vertexColor[i] = r;
        vertexColor[i+1] = g;
        vertexColor[i+2]= b;
        vertexColor[i+3] = a;
    }
}

void AwesomiumUI::SetDrawPosition(int x, int y)
{
    posX = x;
    posY = y;
    vertexPos[0] = (GLfloat)posX;
    vertexPos[1] = (GLfloat)posY + DrawHeight;
    vertexPos[2] = (GLfloat)posX;
    vertexPos[3] = (GLfloat)posY;
    vertexPos[4] = (GLfloat)posX + DrawWidth;
    vertexPos[5] = (GLfloat)posY;
    vertexPos[6] = (GLfloat)posX + DrawWidth;
    vertexPos[7] = (GLfloat)posY + DrawHeight;
}

void AwesomiumUI::SetDrawDimensions(int w, int h)
{
    DrawWidth = w;
    DrawHeight = h;
    vertexPos[0] = (GLfloat)posX;
    vertexPos[1] = (GLfloat)posY + DrawHeight;
    vertexPos[2] = (GLfloat)posX;
    vertexPos[3] = (GLfloat)posY;
    vertexPos[4] = (GLfloat)posX + DrawWidth;
    vertexPos[5] = (GLfloat)posY;
    vertexPos[6] = (GLfloat)posX + DrawWidth;
    vertexPos[7] = (GLfloat)posY + DrawHeight;
}

void AwesomiumUI::InjectMouseMove(int x, int y)
{
    mouseX = x;
    mouseY = y;
    webView->Focus();
    webView->InjectMouseMove(x - posX, y + ((posY + DrawHeight) - ScreenHeight));
}

//0 = left, 1 = middle, 2 = right
void AwesomiumUI::InjectMouseDown(int mouseButton)
{
    webView->Focus();
    webView->InjectMouseDown((MouseButton)mouseButton);
}

void AwesomiumUI::InjectMouseUp(int mouseButton)
{
    webView->Focus();
    webView->InjectMouseUp((MouseButton)mouseButton);
}

void AwesomiumUI::injectMouseWheel(int yMov)
{
    webView->Focus();
    webView->InjectMouseWheel(yMov, 0);
}

void AwesomiumUI::InjectKeyboardEvent(const SDL_Event& event)
{
    if (!(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP))
        return;

    webView->Focus();
    WebKeyboardEvent keyEvent;

    char* buf = new char[20];
    keyEvent.virtual_key_code = getWebKeyFromSDLKey(event.key.keysym.scancode);
    Awesomium::GetKeyIdentifierFromVirtualKeyCode(keyEvent.virtual_key_code,
    &buf);
    strcpy(keyEvent.key_identifier, buf);

    delete[] buf;
 
    keyEvent.modifiers = 0;
 
    if (event.key.keysym.mod & KMOD_LALT || event.key.keysym.mod & KMOD_RALT)
    keyEvent.modifiers |= Awesomium::WebKeyboardEvent::kModAltKey;
    if (event.key.keysym.mod & KMOD_LCTRL || event.key.keysym.mod & KMOD_RCTRL)
    keyEvent.modifiers |= Awesomium::WebKeyboardEvent::kModControlKey;
    if (event.key.keysym.mod & KMOD_LSHIFT || event.key.keysym.mod & KMOD_RSHIFT)
    keyEvent.modifiers |= Awesomium::WebKeyboardEvent::kModShiftKey;
    if (event.key.keysym.mod & KMOD_NUM)
    keyEvent.modifiers |= Awesomium::WebKeyboardEvent::kModIsKeypad;
 
    keyEvent.native_key_code = event.key.keysym.scancode;

    if (event.type == SDL_KEYUP) {
        keyEvent.type = Awesomium::WebKeyboardEvent::kTypeKeyUp;
        webView->InjectKeyboardEvent(keyEvent);
    } else if (event.type == SDL_TEXTINPUT) {
        unsigned int chr;
        //if ((event.key.keysym.unicode & 0xFF80) == 0)
    //    chr = event.key.keysym.unicode & 0x7F;
    //    else
    //    chr = event.key.keysym.unicode;
        chr = (int)event.text.text;
        keyEvent.text[0] = chr;
        keyEvent.unmodified_text[0] = chr;

        keyEvent.type = Awesomium::WebKeyboardEvent::kTypeKeyDown;
        webView->InjectKeyboardEvent(keyEvent);
 
        if (chr) {
            keyEvent.type = Awesomium::WebKeyboardEvent::kTypeChar;
            keyEvent.virtual_key_code = chr;
            keyEvent.native_key_code = chr;
            webView->InjectKeyboardEvent(keyEvent);
        }
    }
}

void AwesomiumUI::Destroy()
{
    if (isInitialized){
        delete data_source;
        webSession->Release();
        webView->Destroy();
        isInitialized = 0;
    }
}

void AwesomiumUI::changeState(int state)
{
    JSArray args;
    args.Push(JSValue(state));
    methodHandler.myObject->Invoke(WSLit("ChangeState"), args);
}

void CustomJSMethodHandler::OnMethodCall(WebView *caller, unsigned int remote_object_id, const WebString &method_name, const JSArray &args)
{
//    cout << "METHOD CALL\n" << endl;
    if(remote_object_id == myObject->remote_id()){

        cout << "CALLING: " << method_name << endl;
        //universal calls
        if (method_name == WSLit("Quit")) {
            uiUpdateData.code = -1;
            return;
        }else if (method_name == WSLit("ReloadTextures")) {
            uiUpdateData.code = -2;
            return;
        } 
        
        //non-universal
        switch (EditorState){
            case E_MAIN:
                if (method_name == WSLit("RequestChangeState")){
                    uiUpdateData.code = 8;
                    uiUpdateData.state = args[0].ToInteger();
                }
                break;
            case E_TREE_EDITOR:
                if (method_name == WSLit("New")) {
                    uiUpdateData.code = 1;
                }else if (method_name == WSLit("ChangeVariable")){
                    uiUpdateData.code = 2;
                    uiUpdateData.id = args[0].ToInteger();
                    uiUpdateData.str = ToString(args[1].ToString());
                }else if (method_name == WSLit("GenerateNewSeed")){
                    uiUpdateData.code = 3;
                }else if (method_name == WSLit("OpenFileDialog")){
                    uiUpdateData.code = 4;
                }else if (method_name == WSLit("Save")){
                    uiUpdateData.code = 5;
                }else if (method_name == WSLit("SaveAs")){
                    uiUpdateData.code = 6;
                }else if (method_name == WSLit("ChangeName")){
                    uiUpdateData.code = 7;
                    VariableUpdateData *vd = new VariableUpdateData;
                    vd->val = ToString(args[0].ToString());
                    uiUpdateData.variableUpdateData.push_back(vd);
                }else if (method_name == WSLit("RequestChangeState")){
                    uiUpdateData.code = 8;
                    uiUpdateData.state = args[0].ToInteger();
                }else if (method_name == WSLit("Regenerate")){
                    uiUpdateData.code = 9;
                }else if (method_name == WSLit("CheckBoxInput")){
                    uiUpdateData.code = 10;
                    uiUpdateData.args = args[0].ToArray();
                }
                break;
            case E_BLOCK_EDITOR:
                if (method_name == WSLit("RequestBlockList")){
                    if (ToString(args[0].ToString()) == "Open"){
                        uiUpdateData.code = 2;
                    }else if (ToString(args[0].ToString()) == "New"){
                        uiUpdateData.code = 6;
                    }
                }else if (method_name == WSLit("RequestBlock")){
                    uiUpdateData.code = 3;
                    uiUpdateData.state = args[0].ToInteger();
                    uiUpdateData.id = args[1].ToInteger();
                    uiUpdateData.mode = args[2].ToInteger();
                }else if (method_name == WSLit("ZoomToBlock")){
                    uiUpdateData.code = 4;
                }else if (method_name == WSLit("ChangeVariable")){
                    uiUpdateData.code = 5;
                    if (ToString(args[0].ToString()) == "BlockAltColor"){
                        VariableUpdateData *vd = new VariableUpdateData;
                        vd->type = 1;
                        vd->offset = args[1].ToInteger();
                        vd->val = ToString(args[2].ToString()) + "," + ToString(args[3].ToString()) + "," + ToString(args[4].ToString());
                        uiUpdateData.variableUpdateData.push_back(vd);
                    }else{
                        for (int i = 0; i < args.size(); i += 2){
                            VariableUpdateData *vd = new VariableUpdateData;
                            vd->offset = args[i].ToInteger();
                            vd->type = 0;
                            vd->val = ToString(args[i+1].ToString());
                            uiUpdateData.variableUpdateData.push_back(vd);
                        }
                    }
                }else if (method_name == WSLit("ChangeBlockID")){
                    uiUpdateData.code = 7;
                }else if (method_name == WSLit("RequestChangeState")){
                    uiUpdateData.code = 8;
                    uiUpdateData.state = args[0].ToInteger();
                }else if (method_name == WSLit("Regenerate")){
                    uiUpdateData.code = 9;
                }else if (method_name == WSLit("ChangeName")){
                    uiUpdateData.code = 10;
                    VariableUpdateData *vd = new VariableUpdateData;
                    vd->val = ToString(args[0].ToString());
                    uiUpdateData.variableUpdateData.push_back(vd);
                }else if (method_name == WSLit("ChangeTexture")){
                    uiUpdateData.code = 11;
                    uiUpdateData.state = args[0].ToInteger();
                }else if (method_name == WSLit("ResizeAltColors")){
                    uiUpdateData.code = 12;
                    uiUpdateData.mode = args[0].ToInteger();
                }else if (method_name == WSLit("Save")){
                    uiUpdateData.code = 13;
                }else if (method_name == WSLit("SaveAs")){
                    uiUpdateData.code = 13;
                }
                break;
            case E_BIOME_EDITOR:
                if (method_name == WSLit("Quit")) {
                    uiUpdateData.code = -1;
                }else if (method_name == WSLit("RequestChangeState")){
                    uiUpdateData.code = 8;
                    uiUpdateData.state = args[0].ToInteger();
                }else if (method_name == WSLit("ChangeVariable")){
                    uiUpdateData.code = 2;
                    for (int i = 0; i < args.size(); i += 2){
                        VariableUpdateData *vd = new VariableUpdateData;
                        vd->offset = args[i].ToInteger();
                        vd->val = ToString(args[i+1].ToString());
                        uiUpdateData.variableUpdateData.push_back(vd);
                    }
                }else if (method_name == WSLit("OpenFileDialog")){
                    uiUpdateData.code = 1;
                }else if (method_name == WSLit("SetTemperature")){
                    uiUpdateData.code = 3;
                    uiUpdateData.id = args[0].ToInteger();
                }else if (method_name == WSLit("SetRainfall")){
                    uiUpdateData.code = 4;
                    uiUpdateData.id = args[0].ToInteger();
                }else if (method_name == WSLit("RequestNoise")){
                    uiUpdateData.code = 5;
                    uiUpdateData.id = args[0].ToInteger();
                }else if (method_name == WSLit("Cancel")){
                    uiUpdateData.code = 6;
                }else if (method_name == WSLit("Save")){
                    uiUpdateData.code = 7;
                }else if (method_name == WSLit("SaveAs")){
                    uiUpdateData.code = 9;
                }else if (method_name == WSLit("Back")){
                    uiUpdateData.code = 10;
                }else if (method_name == WSLit("ChangeName")){
                    uiUpdateData.code = 11;
                    VariableUpdateData *vd = new VariableUpdateData;
                    vd->val = ToString(args[0].ToString());
                    uiUpdateData.variableUpdateData.push_back(vd);
                }else if (method_name == WSLit("AddNoise")){
                    uiUpdateData.code = 12;
                }else if (method_name == WSLit("ChangeNoisePosition")){
                    uiUpdateData.code = 13;
                    uiUpdateData.id = args[0].ToInteger();
                    uiUpdateData.mode = args[1].ToInteger();
                }else if (method_name == WSLit("DeleteNoise")){
                    uiUpdateData.code = 14;
                    uiUpdateData.id = args[0].ToInteger();
                }else if (method_name == WSLit("Help")){
                    uiUpdateData.code = 15;
                    uiUpdateData.id = args[0].ToInteger();
                }else if (method_name == WSLit("SetType")){
                    uiUpdateData.code = 16;
                    uiUpdateData.id = args[0].ToInteger();
                }
                break;
            case E_CLIMATE_EDITOR:
                if (method_name == WSLit("Quit")) {
                    uiUpdateData.code = -1;
                }
                else if (method_name == WSLit("RequestChangeState")){
                    uiUpdateData.code = 8;
                    uiUpdateData.state = args[0].ToInteger();
                }
                else if (method_name == WSLit("ChangeVariable")){
                    uiUpdateData.code = 2;
                    for (int i = 0; i < args.size(); i += 2){
                        VariableUpdateData *vd = new VariableUpdateData;
                        vd->type = args[i].ToInteger();
                        vd->val = ToString(args[i + 1].ToString());
                        uiUpdateData.variableUpdateData.push_back(vd);
                    }
                }
                break;
            case E_TERRAIN_EDITOR:
                if (method_name == WSLit("Quit")) {
                    uiUpdateData.code = -1;
                }
                else if (method_name == WSLit("RequestChangeState")){
                    uiUpdateData.code = 8;
                    uiUpdateData.state = args[0].ToInteger();
                }
                else if (method_name == WSLit("ChangeVariable")){
                    uiUpdateData.code = 2;
                    for (int i = 0; i < args.size(); i += 2){
                        VariableUpdateData *vd = new VariableUpdateData;
                        vd->offset = args[i].ToInteger();
                        vd->val = ToString(args[i + 1].ToString());
                        uiUpdateData.variableUpdateData.push_back(vd);
                    }
                }
                else if (method_name == WSLit("RequestNoise")){
                    uiUpdateData.code = 3;
                    uiUpdateData.id = args[0].ToInteger();
                }
                else if (method_name == WSLit("ChangeNoisePosition")){

                }
                else if (method_name == WSLit("DeleteNoise")){

                }
                else if (method_name == WSLit("Back")){
                    uiUpdateData.code = -3;
                }
                break;
        }
    }
}

JSValue CustomJSMethodHandler::OnMethodCallWithReturnValue(WebView *caller, unsigned int remote_object_id, const WebString &method_name, const JSArray &args)
{
    JSValue js;
    return js;
}

/// Helper Macro
#define mapKey(a, b) case SDLK_##a: return Awesomium::KeyCodes::AK_##b;
 
/// Get an Awesomium KeyCode from an SDLKey Code
int getWebKeyFromSDLKey2(SDL_Scancode key) {
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
