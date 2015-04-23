#include "stdafx.h"
#include "MainMenuAPI.h"

#include <Vorb/io/IOManager.h>
#include <Vorb/ui/KeyStrings.h>
#include "App.h"
#include "GameManager.h"
#include "InputMapper.h"
#include "MainMenuScreen.h"
#include "TerrainPatch.h"

using namespace Awesomium;

const ui32 ITEM_GEN_ID = 0;

const int BORDERLESS_ID = 1;
const int FULLSCREEN_ID = 2;
const int PLANET_QUALITY_ID = 3;

void MainMenuAPI::init(WebView* webView, vui::CustomJSMethodHandler<MainMenuAPI>* methodHandler,
                       vui::IGameScreen* ownerScreen) {

    // Helper macro for adding functions
    #define ADDFUNC(a) addFunction(""#a"", &MainMenuAPI::##a)

    // Set up the interface object so we can talk to the JS
    m_methodHandler = methodHandler;
    m_webView = webView;

    // Set up our screen handle so we can talk to the game
    setOwnerScreen(ownerScreen);

    // Add functions here
    ADDFUNC(getCameraPosition);
    ADDFUNC(getSaveFiles);
    ADDFUNC(getControls);
    
    ADDFUNC(setCameraFocalLength);
    ADDFUNC(setCameraPosition);
    ADDFUNC(setCameraTarget);
    ADDFUNC(print);
    ADDFUNC(loadSaveGame);
    ADDFUNC(newSaveGame);
    ADDFUNC(optionUpdate);
    ADDFUNC(quit);

    // Add controls
    addExistingObject("ListItemGenerator", ITEM_GEN_ID);
}

void MainMenuAPI::setOwnerScreen(vui::IGameScreen* ownerScreen) {
    _ownerScreen = static_cast<MainMenuScreen*>(ownerScreen);
}

Awesomium::JSValue MainMenuAPI::initMainMenu() {
    JSObject obj = getObject(ITEM_GEN_ID);
    JSArray controls;

    { // Add options sublist
        JSArray subItems;
        JSArray graphicsOptionsArray, controlsArray;

        // Graphics options
        graphicsOptionsArray.Push(WSLit("click"));
        graphicsOptionsArray.Push(WSLit("Video Options"));
        JSArray linkData;
        linkData.Push(WSLit("video_options.html"));
        linkData.Push(WSLit(""));
        graphicsOptionsArray.Push(WSLit("video_options.html"));
        graphicsOptionsArray.Push(WSLit(""));
        graphicsOptionsArray.Push(WSLit("Make the game pretty or ugly."));
        graphicsOptionsArray.Push(JSValue(0));
        graphicsOptionsArray.Push(WSLit(""));
        subItems.Push(graphicsOptionsArray);
        // Controls
        controlsArray.Push(WSLit("click"));
        controlsArray.Push(WSLit("Controls"));
        controlsArray.Push(WSLit("#"));
        controlsArray.Push(WSLit(""));
        controlsArray.Push(WSLit("See which buttons do what."));
        controlsArray.Push(JSValue(0));
        controlsArray.Push(WSLit(""));
        subItems.Push(controlsArray);
        
        JSArray args;
        args.Push(WSLit("subList"));
        args.Push(WSLit("Options"));
        args.Push(subItems);
        args.Push(WSLit(""));
        args.Push(WSLit("Customize the game."));
        controls.Push(args);
    }

    { // Add exit button
        JSArray args;
        args.Push(WSLit("click"));
        args.Push(WSLit("Exit"));
        args.Push(WSLit("#"));
        args.Push(WSLit(""));
        args.Push(WSLit("Just ten more minutes..."));
        args.Push(JSValue(4));
        args.Push(WSLit("App.quit"));
        controls.Push(args);
    }

    return JSValue(controls);
}

Awesomium::JSValue MainMenuAPI::initVideoOptionsMenu() {
    JSObject obj = getObject(ITEM_GEN_ID);

    { // Add quality slider
        JSArray args;
        args.Push(WSLit("Planet Quality")); // name
        args.Push(JSValue(1)); // min
        args.Push(JSValue(5)); // max
        args.Push(JSValue(1)); // initialVal
        args.Push(JSValue(1)); // intervalRes
        args.Push(WSLit("")); // category
        args.Push(WSLit("Adjust planet terrain quality")); // description
        args.Push(JSValue(PLANET_QUALITY_ID)); // ID
        args.Push(WSLit("App.optionUpdate")); // updateCallback
        args.Push(JSValue(true)); // updateInRealTime
        obj.Invoke(WSLit("generateSlider"), args);
    }

    { // Add borderless toggle
        JSArray args;
        args.Push(WSLit("Borderless Window")); // name
        args.Push(JSValue(_ownerScreen->_app->getWindow().isBorderless())); // initialVal
        args.Push(WSLit("")); // category
        args.Push(WSLit("Toggle the window border")); // description
        args.Push(JSValue(BORDERLESS_ID)); // ID
        args.Push(WSLit("App.optionUpdate")); // updateCallback
        args.Push(JSValue(true)); // updateInRealTime
        obj.Invoke(WSLit("generateToggle"), args);
    }

    { // Add fullscreen toggle
        JSArray args;
        args.Push(WSLit("Fullscreen")); // name
        args.Push(JSValue(_ownerScreen->_app->getWindow().isFullscreen())); // initialVal
        args.Push(WSLit("")); // category
        args.Push(WSLit("Toggle fullscreen mode")); // description
        args.Push(JSValue(FULLSCREEN_ID)); // ID
        args.Push(WSLit("App.optionUpdate")); // updateCallback
        args.Push(JSValue(true)); // updateInRealTime
        obj.Invoke(WSLit("generateToggle"), args);
    }

    { // Add test slider
        JSArray args;
        JSArray vals;
        args.Push(WSLit("Test")); // name
        vals.Push(WSLit("A"));
        vals.Push(WSLit("B"));
        vals.Push(WSLit("C"));
        vals.Push(WSLit("Z"));
        args.Push(vals); // vals
        args.Push(WSLit("A")); // initialVal
        args.Push(WSLit("")); // category
        args.Push(WSLit("roflcopter")); // description
        args.Push(JSValue(69)); // ID
        args.Push(WSLit("App.optionUpdate")); // updateCallback
        args.Push(JSValue(true)); // updateInRealTime
        obj.Invoke(WSLit("generateDiscreteSlider"), args);
    }

    { // Add test combo box
        JSArray args;
        JSArray vals;
        args.Push(WSLit("Test Combo")); // name
        vals.Push(WSLit("A"));
        vals.Push(WSLit("B"));
        vals.Push(WSLit("C"));
        vals.Push(WSLit("Z"));
        args.Push(vals); // vals
        args.Push(WSLit("A")); // initialVal
        args.Push(WSLit("")); // category
        args.Push(WSLit("roflcopter")); // description
        args.Push(JSValue(70)); // ID
        args.Push(WSLit("App.optionUpdate")); // updateCallback
        args.Push(JSValue(true)); // updateInRealTime
        obj.Invoke(WSLit("generateComboBox"), args);
    }
    return JSValue();
}

Awesomium::JSValue MainMenuAPI::initControlsMenu() {
    JSObject obj = getObject(ITEM_GEN_ID);

    InputMapper* inputMapper = _ownerScreen->m_inputMapper;
    const InputMapper::InputMap &inputMap = inputMapper->getInputLookup();
    char buf[256];
    // Add buttons for each input
    for (auto& it : inputMap) {
        JSArray args;
        InputMapper::Input& in = inputMapper->get(it.second);
        sprintf(buf, "%20s | %10s", it.first.c_str(), VirtualKeyStrings[in.key]);
        args.Push(WSLit(buf));
        args.Push(WSLit("#"));
        args.Push(WSLit(""));
        args.Push(WSLit(""));
        args.Push(JSValue(4));
        args.Push(WSLit(""));
        obj.Invoke(WSLit("generateClickable"), args);
    }

    return JSValue();
}

Awesomium::JSArray generateClickable(const cString name, const JSArray& linkData,
                                     const cString category, const cString description,
                                     int id, const cString updateCallback) {
    JSArray a;
    a.Push(WSLit(name));
    a.Push(linkData);
    a.Push(WSLit(category));
    a.Push(WSLit(description));
    a.Push(JSValue(id));
    a.Push(WSLit(updateCallback));
    return a;
}

Awesomium::JSArray generateText(const cString name, const cString text,
                                const cString category, const cString description) {
    JSArray a;
    a.Push(WSLit(name));
    a.Push(WSLit(text));
    a.Push(WSLit(category));
    a.Push(WSLit(description));
    return a;
}

Awesomium::JSArray generateToggle(const cString name, bool isToggled,
                                  const cString category, const cString description,
                                  int id, const cString updateCallback,
                                  bool updateInRealTime) {
    JSArray a;
    a.Push(WSLit(name));
    isToggled ? a.Push(WSLit("checked")) : a.Push(WSLit(""));
    a.Push(WSLit(category));
    a.Push(WSLit(description));
    a.Push(JSValue(id));
    a.Push(WSLit(updateCallback));
    a.Push(JSValue(updateInRealTime));
    return a;
}

Awesomium::JSArray generateSlider(const cString name, Awesomium::JSValue min,
                                  Awesomium::JSValue max, Awesomium::JSValue initialVal,
                                  Awesomium::JSValue intervalRes,
                                  const cString category, const cString description,
                                  int id, const cString updateCallback,
                                  bool updateInRealTime) {
    JSArray a;
    a.Push(WSLit(name));
    a.Push(min);
    a.Push(max);
    a.Push(initialVal);
    a.Push(intervalRes);
    a.Push(WSLit(category));
    a.Push(WSLit(description));
    a.Push(JSValue(id));
    a.Push(WSLit(updateCallback));
    a.Push(JSValue(updateInRealTime));
    return a;
}

Awesomium::JSArray generateDiscrete(const cString name, Awesomium::JSArray vals,
                                    Awesomium::JSValue initialVal,
                                    const cString category, const cString description,
                                    int id, const cString updateCallback,
                                    bool updateInRealTime) {
    JSArray a;
    a.Push(WSLit(name));
    a.Push(vals);
    a.Push(initialVal);
    a.Push(WSLit(category));
    a.Push(WSLit(description));
    a.Push(JSValue(id));
    a.Push(WSLit(updateCallback));
    a.Push(JSValue(updateInRealTime));
    return a;
}

Awesomium::JSArray generateCombo(const cString name, Awesomium::JSArray vals,
                                 Awesomium::JSValue initialVal,
                                 const cString category, const cString description,
                                 int id, const cString updateCallback,
                                 bool updateInRealTime) {
    JSArray a;
    a.Push(WSLit(name));
    a.Push(vals);
    a.Push(initialVal);
    a.Push(WSLit(category));
    a.Push(WSLit(description));
    a.Push(JSValue(id));
    a.Push(WSLit(updateCallback));
    a.Push(JSValue(updateInRealTime));
    return a;
}

JSValue MainMenuAPI::getCameraPosition(const JSArray& args) {
    JSArray rv;
    const f64v3& pos = _ownerScreen->getCamera().getPosition();
    rv.Push(JSValue(pos.x));
    rv.Push(JSValue(pos.y));
    rv.Push(JSValue(pos.z));
    return JSValue(rv);
}

JSValue MainMenuAPI::getSaveFiles(const JSArray& args) {

    // Read the contents of the Saves directory
    std::vector<vpath> paths;
    _ownerScreen->getIOManager().getDirectoryEntries("Saves", paths);

    // For getting localtime
    tm timeinfo;

    char timeString[128];

    JSArray entries;
    for (int i = 0; i < paths.size(); i++) {
        if (paths[i].isDirectory()) {
            // Add the filename
            nString fileName = paths[i].getLeaf();
            entries.Push(::WSLit(fileName.c_str()));
            // Add the access time
            time_t writeTime = paths[i].getLastModTime();
            // Get the time info
            localtime_s(&timeinfo, &writeTime);
            // Create the string
            sprintf(timeString, "%02d.%02d.%04d  %02d:%02d", 
                    timeinfo.tm_mday,
                    timeinfo.tm_mon,
                    timeinfo.tm_year + 1900,
                    timeinfo.tm_hour,
                    timeinfo.tm_min);
            entries.Push(WSLit(timeString));
        }
    }
    return JSValue(entries);
}

Awesomium::JSValue MainMenuAPI::getControls(const Awesomium::JSArray& args) {
    switch (m_currentPage) {
        case 0:
            return initMainMenu();
            break;
    }
    return JSValue();
}

void MainMenuAPI::setCameraFocalLength(const JSArray& args) {
    _ownerScreen->getCamera().setFocalLength((float)args[0].ToDouble());
}

void MainMenuAPI::setCameraPosition(const JSArray& args) {
    _ownerScreen->getCamera().setPosition(f64v3(args[0].ToDouble(), args[1].ToDouble(), args[2].ToDouble()));
}

void MainMenuAPI::setCameraTarget(const JSArray& args) {
    f64v3 targetPos(args[0].ToDouble(), args[1].ToDouble(), args[2].ToDouble());
    float time = args[3].ToDouble();
    float focalLength = args[4].ToDouble();
    f32v3 targetDir(args[5].ToDouble(), args[6].ToDouble(), args[7].ToDouble());
    f32v3 targetRight(args[8].ToDouble(), args[9].ToDouble(), args[10].ToDouble());
    _ownerScreen->getCamera().setTarget(targetPos, glm::normalize(targetDir), glm::normalize(targetRight), focalLength);
}

void MainMenuAPI::loadSaveGame(const JSArray& args) {
    if (!args.size()) return;

    nString fileName = "Saves/" + ::ToString(args[0].ToString());
    _ownerScreen->loadGame(fileName);
}

void MainMenuAPI::newSaveGame(const JSArray& args) {
    if (!args.size()) return;

    nString fileName = "Saves/" + ::ToString(args[0].ToString());
    _ownerScreen->newGame(fileName);
}

void MainMenuAPI::optionUpdate(const JSArray& args) {
    std::cout << args[0].ToString() << std::endl;
    std::cout << args[1].ToString() << std::endl;
    int id = args[0].ToInteger();

    vui::GameWindow* gameWindow = const_cast<vui::GameWindow*>(&_ownerScreen->_app->getWindow());

    switch (id) {
        case BORDERLESS_ID: 
            gameWindow->setBorderless(Awesomium::ToString(args[1].ToString()) == "on");
            break;
        case FULLSCREEN_ID:
            gameWindow->setFullscreen(Awesomium::ToString(args[1].ToString()) == "on");
            break;
        case PLANET_QUALITY_ID:
            TerrainPatch::setQuality(args[1].ToInteger());
            break;
    }
}

void MainMenuAPI::quit(const JSArray& args) {
    _ownerScreen->onQuit(0, 0);
}
