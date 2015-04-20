#include "stdafx.h"
#include "MainMenuAPI.h"

#include <Vorb/io/IOManager.h>
#include "App.h"
#include "MainMenuScreen.h"
#include "GameManager.h"

using namespace Awesomium;

const ui32 ITEM_GEN_ID = 0;

const int BORDERLESS_ID = 1;
const int FULLSCREEN_ID = 2;

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
    ADDFUNC(getPlanetRadius);
    ADDFUNC(getSaveFiles);
    
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
    initVideoOptionsControls();
    initIndexControls();  
}

void MainMenuAPI::setOwnerScreen(vui::IGameScreen* ownerScreen) {
    _ownerScreen = static_cast<MainMenuScreen*>(ownerScreen);
}

void MainMenuAPI::initIndexControls() {
    JSObject obj = getObject(ITEM_GEN_ID);

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
        args.Push(WSLit("Options"));
        args.Push(subItems);
        args.Push(WSLit(""));
        args.Push(WSLit("Customize the game."));
        obj.Invoke(WSLit("generateSubList"), args);
    }

    { // Add exit button
        JSArray args;
        args.Push(WSLit("Exit"));
        args.Push(WSLit("#"));
        args.Push(WSLit(""));
        args.Push(WSLit("Just ten more minutes..."));
        args.Push(JSValue(4));
        args.Push(WSLit("App.quit"));
        obj.Invoke(WSLit("generateClickable"), args);
    }
}

void MainMenuAPI::initVideoOptionsControls() {
    JSObject obj = getObject(ITEM_GEN_ID);

    { // Add quality slider
        JSArray args;
        args.Push(WSLit("Planet Quality")); // name
        args.Push(JSValue(0)); // min
        args.Push(JSValue(10)); // max
        args.Push(JSValue(5)); // initialVal
        args.Push(JSValue(1)); // intervalRes
        args.Push(WSLit("")); // category
        args.Push(WSLit("Adjust planet terrain quality")); // description
        args.Push(JSValue(6)); // ID
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
}

JSValue MainMenuAPI::getCameraPosition(const JSArray& args) {
    JSArray rv;
    const f64v3& pos = _ownerScreen->getCamera().getPosition();
    rv.Push(::JSValue(pos.x));
    rv.Push(::JSValue(pos.y));
    rv.Push(::JSValue(pos.z));
    return ::JSValue(rv);
}

JSValue MainMenuAPI::getPlanetRadius(const JSArray& args) {
  //  return ::JSValue(GameManager::planet->getRadius());
    return JSValue();
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
    }
}

void MainMenuAPI::quit(const JSArray& args) {
    _ownerScreen->onQuit(0, 0);
}
