#include "stdafx.h"
#include "MainMenuAPI.h"

#include <Vorb/io/IOManager.h>
#include "MainMenuScreen.h"
#include "GameManager.h"

void MainMenuAPI::init(Awesomium::JSObject* interfaceObject, vui::IGameScreen* ownerScreen) {

    // Helper macro for adding functions
    #define ADDFUNC(a) addFunction(""#a"", &MainMenuAPI::##a)

    // Set up the interface object so we can talk to the JS
    _interfaceObject = interfaceObject;

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
}

void MainMenuAPI::setOwnerScreen(vui::IGameScreen* ownerScreen) {
    _ownerScreen = static_cast<MainMenuScreen*>(ownerScreen);
}

Awesomium::JSValue MainMenuAPI::getCameraPosition(const Awesomium::JSArray& args) {
    Awesomium::JSArray rv;
    const f64v3& pos = _ownerScreen->getCamera().getPosition();
    rv.Push(Awesomium::JSValue(pos.x));
    rv.Push(Awesomium::JSValue(pos.y));
    rv.Push(Awesomium::JSValue(pos.z));
    return Awesomium::JSValue(rv);
}

Awesomium::JSValue MainMenuAPI::getPlanetRadius(const Awesomium::JSArray& args) {
  //  return Awesomium::JSValue(GameManager::planet->getRadius());
    return Awesomium::JSValue();
}

Awesomium::JSValue MainMenuAPI::getSaveFiles(const Awesomium::JSArray& args) {

    // Read the contents of the Saves directory
    std::vector<vpath> paths;
    _ownerScreen->getIOManager().getDirectoryEntries("Saves", paths);

    // For getting localtime
    tm timeinfo;

    char timeString[128];

    Awesomium::JSArray entries;
    for (int i = 0; i < paths.size(); i++) {
        if (paths[i].isDirectory()) {
            // Add the filename
            nString fileName = paths[i].getLeaf();
            entries.Push(Awesomium::WSLit(fileName.c_str()));
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
            entries.Push(Awesomium::WSLit(timeString));
        }
    }
    return Awesomium::JSValue(entries);
}

void MainMenuAPI::setCameraFocalLength(const Awesomium::JSArray& args) {
    _ownerScreen->getCamera().setFocalLength((float)args[0].ToDouble());
}

void MainMenuAPI::setCameraPosition(const Awesomium::JSArray& args) {
    _ownerScreen->getCamera().setPosition(f64v3(args[0].ToDouble(), args[1].ToDouble(), args[2].ToDouble()));
}

void MainMenuAPI::setCameraTarget(const Awesomium::JSArray& args) {
    f64v3 targetPos(args[0].ToDouble(), args[1].ToDouble(), args[2].ToDouble());
    float time = args[3].ToDouble();
    float focalLength = args[4].ToDouble();
    f32v3 targetDir(args[5].ToDouble(), args[6].ToDouble(), args[7].ToDouble());
    f32v3 targetRight(args[8].ToDouble(), args[9].ToDouble(), args[10].ToDouble());
    _ownerScreen->getCamera().setTarget(targetPos, glm::normalize(targetDir), glm::normalize(targetRight), focalLength);
}

void MainMenuAPI::loadSaveGame(const Awesomium::JSArray& args) {
    if (!args.size()) return;

    nString fileName = "Saves/" + Awesomium::ToString(args[0].ToString());
    _ownerScreen->loadGame(fileName);
}

void MainMenuAPI::newSaveGame(const Awesomium::JSArray& args) {
    if (!args.size()) return;

    nString fileName = "Saves/" + Awesomium::ToString(args[0].ToString());
    _ownerScreen->newGame(fileName);
}