#pragma once
#include <JSON/json_spirit_value.h>
#include <JSON/json_spirit_reader.h>
#include <JSON/json_spirit_error_position.h>

#include "GameControls.h"
#include "global.h"
#include "Player.h"

bool InitializeMenus();

static enum JsonString { jNONE, jCOLORDEF, jTYPE, jCOLOR, jLCOLOR, jRCOLOR, jX, jY, jWIDTH, jHEIGHT, jPADDING,
                    jHOVERCOLOR, jLHOVERCOLOR, jRHOVERCOLOR, jFUNC, jTEXT, jTEXTFIELD, jCHILDPAD, jCHILDHEIGHT, jBUTTONS, 
                    jBUTTONLIST, jBUTTON, jTEXTURE, jR, jG, jB, jA, jCOMMENT, jSOURCE, jCHILDBOXLCOLOR, jCHILDBOXRCOLOR, 
                    jCHILDBOXCOLOR, jTEXTCOLOR, jFRONTCOLOR, jBACKCOLOR, jHOVERTEXTCOLOR, jANIMATION, jANIMATIONSPEED, jJUSTIFICATION,
                    jTEXTSIZE, jGENERATEBUTTONSFROM, jSLIDER, jCALLBACK, jLOADGAME, jBORDERSIZE, jVARIABLE, jDISPLAYVARIABLE, jFILTER,
                    jSLIDESIZE, jISVERTICAL, jMAXHEIGHT, jTEXCOORDS, jCAPSVARIABLE, jCHECKBOX, jTEXTJUSTIFICATION, jALWAYSOPEN,
                    jMAXWIDTH, jHOVERFUNC };

void RegisterJCallback(JsonString j, int (*func)(int, string));

struct MenuAnimation
{
    MenuAnimation(){
        anim = 0.0f;
        active = 0;
    }
    bool active;
    float anim;
};

class GameMenu
{
public:
    GameMenu(string jsonfilePath = "");
    ~GameMenu()
    {
        for (size_t i = 0; i < overlays.size(); i++){
            delete overlays[i];
        }
        FreeControls();
    }
    void SetOverlayText(string txt);
    void Open();
    void InitializeControls();
    void ReInitializeControls();
    void FreeControls();
    void FlushControlStates();
    virtual int Control();
    virtual void Draw();
    void SetOverlayActive(int oa){
        overlayActive = oa;
    }
    void LoadJsonFile(const char *filePath);
    void ProcessJsonObject(json_spirit::Object &obj, string label);
    void MakeButtonList(json_spirit::Object &obj, ButtonList *nbl, int &i);
    void MakeButton(json_spirit::Object &obj, Button *b, int &i);
    void MakeText(json_spirit::Object &obj, Texture2D &tex, int &i);
    void MakeTexture(json_spirit::Object &obj, Texture2D *tex, int &i);
    void MakeTextField(json_spirit::Object &obj, TextField *texf, int &i);
    void MakeScrollBar(json_spirit::Object &obj, ScrollBar *ns, int &i);
    void MakeCheckBox(json_spirit::Object &obj, CheckBox *cb, int &i);
    void GetTexCoords(json_spirit::Object &obj, Texture2D &tex); 

    vector <GameMenu *> overlays;
    int overlayActive;
    Player *player;
protected:
    bool fadingOut;
    MenuAnimation scrambleAnimation;
    int scrambleStep;
    MenuAnimation fadeInAnimation;
    MenuAnimation fadeOutAnimation;
    float animSpeed;
    Button *hoverButton;
    string jsonFilePath;
    vector <CheckBox*> checkBoxes; 
    vector <ButtonList*> buttonLists;
    vector <Button*> buttons;
    vector <Texture2D> textures;
    vector <ScrollBar*> scrollBars;
    vector <TextField*> textFields;
};

struct InventoryArgs{
    InventoryArgs(): selected(-1), arm(0){}
    int selected;
    bool arm;
};

class InventoryMenu : public GameMenu
{
public:
    InventoryMenu(string jsonFilePath = "") : selectedCategory(0), GameMenu(jsonFilePath)
    {
        invList = NULL;
        filterTextField = NULL;
    }

    void InitializeInventory(Player *playr)
    {
        lastBlockSelected = 0;
        player = playr;
        for (size_t i = 0; i < buttonLists.size(); i++){
            if (buttonLists[i]->displayInventory){
                invList = buttonLists[i];
                if (invList->currFilter.size()){
                    for (size_t i = 0; i < textFields.size(); i++){
                        if (textFields[i]->label == invList->currFilter){
                            filterTextField = textFields[i];
                            break;
                        }
                    }
                }
                break;
            }
        }
        if (invList){
            MakeInventoryLists(invList, "");
        }
    }

    int Control(InventoryArgs &args);
    void Draw();
    void MakeInventoryLists(ButtonList *bl, string filter);
    void UpdateInventoryList();
    int lastBlockSelected;
    
    vector <int> blockIDs;
    ButtonList *invList;
    TextField *filterTextField;
    int selectedCategory;
};


//menus
extern GameMenu *newGameMenu;
extern GameMenu *loadGameMenu;
extern GameMenu *creditsMenu;
extern GameMenu *mainMenu;
extern GameMenu *pauseMenu;
extern GameMenu *videoOptionsMenu, *audioOptionsMenu, *gameOptionsMenu;
extern GameMenu *controlsMenu;
extern GameMenu *newMarkerMenu, *markerMenu, *deleteMarkerMenu;
extern InventoryMenu *inventoryMenu;
extern GameMenu *texturePackMenu;
extern GameMenu *worldEditorSelectMenu;
