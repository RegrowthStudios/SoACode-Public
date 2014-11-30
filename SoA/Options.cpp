#include "stdafx.h"
#include "Options.h"

#include <SDL/SDL.h>

#include "FileSystem.h"
#include "GameManager.h"
#include "InputManager.h"

KEG_TYPE_INIT_BEGIN_DEF_VAR(GraphicsOptions)
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("enableParticles", Keg::Value::basic(Keg::BasicType::BOOL, offsetof(GraphicsOptions, enableParticles)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("fov", Keg::Value::basic(Keg::BasicType::F32, offsetof(GraphicsOptions, fov)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("gamma", Keg::Value::basic(Keg::BasicType::F32, offsetof(GraphicsOptions, gamma)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("voxelRenderDistance", Keg::Value::basic(Keg::BasicType::I32, offsetof(GraphicsOptions, voxelRenderDistance)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("terrainQuality", Keg::Value::basic(Keg::BasicType::I32, offsetof(GraphicsOptions, lodDetail)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("texturePack", Keg::Value::basic(Keg::BasicType::STRING, offsetof(GraphicsOptions, texturePackString)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("maxFps", Keg::Value::basic(Keg::BasicType::F32, offsetof(GraphicsOptions, maxFPS)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("motionBlur", Keg::Value::basic(Keg::BasicType::F32, offsetof(GraphicsOptions, motionBlur)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("msaa", Keg::Value::basic(Keg::BasicType::I32, offsetof(GraphicsOptions, msaa)));
KEG_TYPE_INIT_END

KEG_TYPE_INIT_BEGIN_DEF_VAR(GameOptions)
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("mouseSensitivity", Keg::Value::basic(Keg::BasicType::F32, offsetof(GameOptions, mouseSensitivity)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("invertMouse", Keg::Value::basic(Keg::BasicType::BOOL, offsetof(GameOptions, invertMouse)));
KEG_TYPE_INIT_END

KEG_TYPE_INIT_BEGIN_DEF_VAR(SoundOptions)
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("musicVolume", Keg::Value::basic(Keg::BasicType::F32, offsetof(SoundOptions, musicVolume)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("effectVolume", Keg::Value::basic(Keg::BasicType::F32, offsetof(SoundOptions, effectVolume)));
KEG_TYPE_INIT_END

std::vector<ui32v2> SCREEN_RESOLUTIONS;

GraphicsOptions graphicsOptions;
SoundOptions soundOptions;
GameOptions gameOptions;
MenuOptions menuOptions;

bool loadOptions(const cString filePath) {
    IOManager ioManager; // TODO: Pass in a real boy
    const cString data = ioManager.readFileToString(filePath);

    YAML::Node node = YAML::Load(data);
    if (node.IsNull() || !node.IsMap()) {
        delete[] data;
        perror(filePath);
        return false;
    }

    // Manually parse yml file
    Keg::Value v = Keg::Value::custom("Axis", 0);
    for (auto& kvp : node) {
        nString structName = kvp.first.as<nString>();
        if (structName == "GraphicsOptions") {
            Keg::parse((ui8*)&graphicsOptions, kvp.second, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(GraphicsOptions));
        } else if (structName == "GameOptions") {
            Keg::parse((ui8*)&gameOptions, kvp.second, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(GameOptions));
        } else if (structName == "SoundOptions") {
            Keg::parse((ui8*)&soundOptions, kvp.second, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(SoundOptions));
        }
    }

    delete[] data;
    return true;
}