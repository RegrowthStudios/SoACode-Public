#include "stdafx.h"
#include "MainMenuAPI.h"

#include "MainMenuScreen.h"
#include "GameManager.h"
#include "Planet.h"

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

void MainMenuAPI::init(Awesomium::JSObject* interfaceObject, IGameScreen* ownerScreen) {

    // Set up the interface object so we can talk to the JS
    _interfaceObject = interfaceObject;

    // Set up our screen handle so we can talk to the game
    setOwnerScreen(ownerScreen);

    // Add functions here
    addFunction("getCameraPosition", &MainMenuAPI::getCameraPosition);
    addFunction("getPlanetRadius", &MainMenuAPI::getPlanetRadius);
    addFunction("getSaveFiles", &MainMenuAPI::getSaveFiles);

    addFunction("setCameraFocalLength", &MainMenuAPI::setCameraFocalLength);
    addFunction("setCameraPosition", &MainMenuAPI::setCameraPosition);
    addFunction("setCameraTarget", &MainMenuAPI::setCameraTarget);
    addFunction("print", &MainMenuAPI::print);

}

void MainMenuAPI::setOwnerScreen(IGameScreen* ownerScreen) {
    _ownerScreen = static_cast<MainMenuScreen*>(ownerScreen);
}

Awesomium::JSValue MainMenuAPI::getCameraPosition(const Awesomium::JSArray& args) {
    Awesomium::JSArray rv;
    const f64v3& pos = _ownerScreen->getCamera().position();
    rv.Push(Awesomium::JSValue(pos.x));
    rv.Push(Awesomium::JSValue(pos.y));
    rv.Push(Awesomium::JSValue(pos.z));
    return Awesomium::JSValue(rv);
}

Awesomium::JSValue MainMenuAPI::getPlanetRadius(const Awesomium::JSArray& args) {
    return Awesomium::JSValue(GameManager::planet->radius);
}

Awesomium::JSValue MainMenuAPI::getSaveFiles(const Awesomium::JSArray& args) {
    std::cout << "Getting Save Files\n" << std::endl;
    // Read the contents of the Saves directory
    std::vector<boost::filesystem::path> paths;
    _ownerScreen->getIOManager().getDirectoryEntries("Saves", paths);

    Awesomium::JSArray entries;
    for (int i = 0; i < paths.size(); i++) {
        if (boost::filesystem::is_directory(paths[i])) {
            // Add the filename
            const char* fileName = paths[i].filename().string().c_str();
            entries.Push(Awesomium::WSLit(fileName));
            // Add the access time
            const char* writeTime = boost::lexical_cast<nString>(boost::filesystem::last_write_time(paths[i])).c_str();
            entries.Push(Awesomium::WSLit(writeTime));
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
    f64v3 targetDir(args[5].ToDouble(), args[6].ToDouble(), args[7].ToDouble());
    f64v3 targetRight(args[8].ToDouble(), args[9].ToDouble(), args[10].ToDouble());
    _ownerScreen->getCamera().zoomTo(targetPos, time, glm::normalize(targetDir), glm::normalize(targetRight), glm::dvec3(0.0), GameManager::planet->radius, focalLength);
}

void MainMenuAPI::print(const Awesomium::JSArray& args) {
    if (args.size()) {
        std::cout << args[0].ToString() << std::endl;
    }
}