#include "stdafx.h"
#include "Options.h"

#include <SDL/SDL.h>
#include <Vorb/io/IOManager.h>

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
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("motionBlur", Keg::Value::basic(Keg::BasicType::I32, offsetof(GraphicsOptions, motionBlur)));
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
    vio::IOManager ioManager; // TODO: Pass in a real boy
    const cString data = ioManager.readFileToString(filePath);

    keg::YAMLReader reader;
    reader.init(data);
    keg::Node node = reader.getFirst();
    if (keg::getType(node) != keg::NodeType::MAP) {
        reader.dispose();
        delete[] data;
        perror(filePath);
        return false;
    }

    // Manually parse yml file
    auto f = createDelegate<const nString&, keg::Node>([&] (Sender, const nString& structName, keg::Node value) {
        if (structName == "GraphicsOptions") {
            Keg::parse((ui8*)&graphicsOptions, value, reader, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(GraphicsOptions));
        } else if (structName == "GameOptions") {
            Keg::parse((ui8*)&gameOptions, value, reader, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(GameOptions));
        } else if (structName == "SoundOptions") {
            Keg::parse((ui8*)&soundOptions, value, reader, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(SoundOptions));
        }
    });
    reader.forAllInMap(node, f);
    delete f;

    reader.dispose();
    delete[] data;
    return true;
}