#include "stdafx.h"
#include "Options.h"

#include <SDL/SDL.h>
#include <Vorb/io/IOManager.h>

#include "FileSystem.h"
#include "GameManager.h"
#include "InputManager.h"

KEG_TYPE_DEF(GraphicsOptions, GraphicsOptions, kt) {
    kt.addValue("enableParticles", keg::Value::basic(offsetof(GraphicsOptions, enableParticles), keg::BasicType::BOOL));
    kt.addValue("fov", keg::Value::basic(offsetof(GraphicsOptions, fov), keg::BasicType::F32));
    kt.addValue("gamma", keg::Value::basic(offsetof(GraphicsOptions, gamma), keg::BasicType::F32));
    kt.addValue("voxelRenderDistance", keg::Value::basic(offsetof(GraphicsOptions, voxelRenderDistance), keg::BasicType::I32));
    kt.addValue("terrainQuality", keg::Value::basic(offsetof(GraphicsOptions, lodDetail), keg::BasicType::I32));
    kt.addValue("texturePack", keg::Value::basic(offsetof(GraphicsOptions, texturePackString), keg::BasicType::STRING));
    kt.addValue("maxFps", keg::Value::basic(offsetof(GraphicsOptions, maxFPS), keg::BasicType::F32));
    kt.addValue("motionBlur", keg::Value::basic(offsetof(GraphicsOptions, motionBlur), keg::BasicType::I32));
    kt.addValue("msaa", keg::Value::basic(offsetof(GraphicsOptions, msaa), keg::BasicType::I32));
}

KEG_TYPE_DEF(GameOptions, GameOptions, kt) {
    kt.addValue("mouseSensitivity", keg::Value::basic(offsetof(GameOptions, mouseSensitivity), keg::BasicType::F32));
    kt.addValue("invertMouse", keg::Value::basic(offsetof(GameOptions, invertMouse), keg::BasicType::BOOL));
}

KEG_TYPE_DEF(SoundOptions, SoundOptions, kt) {
    kt.addValue("musicVolume", keg::Value::basic(offsetof(SoundOptions, musicVolume), keg::BasicType::F32));
    kt.addValue("effectVolume", keg::Value::basic(offsetof(SoundOptions, effectVolume), keg::BasicType::F32));
}

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
    auto f = makeFunctor<Sender, const nString&, keg::Node>([&] (Sender, const nString& structName, keg::Node value) {
        if (structName == "GraphicsOptions") {
            keg::parse((ui8*)&graphicsOptions, value, reader, keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(GraphicsOptions));
        } else if (structName == "GameOptions") {
            keg::parse((ui8*)&gameOptions, value, reader, keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(GameOptions));
        } else if (structName == "SoundOptions") {
            keg::parse((ui8*)&soundOptions, value, reader, keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(SoundOptions));
        }
    });
    reader.forAllInMap(node, f);
    delete f;

    reader.dispose();
    delete[] data;
    return true;
}