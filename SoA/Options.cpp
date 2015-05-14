#include "stdafx.h"
#include "Options.h"

#include <SDL/SDL.h>
#include <Vorb/io/IOManager.h>

#include "FileSystem.h"
#include "GameManager.h"
#include "InputMapper.h"

KEG_TYPE_DEF(Options, Options, kt) {
    kt.addValue("enableParticles", keg::Value::basic(offsetof(Options, enableParticles), keg::BasicType::BOOL));
    kt.addValue("fov", keg::Value::basic(offsetof(Options, fov), keg::BasicType::F32));
    kt.addValue("gamma", keg::Value::basic(offsetof(Options, gamma), keg::BasicType::F32));
    kt.addValue("voxelRenderDistance", keg::Value::basic(offsetof(Options, voxelRenderDistance), keg::BasicType::I32));
    kt.addValue("terrainQuality", keg::Value::basic(offsetof(Options, lodDetail), keg::BasicType::I32));
    kt.addValue("texturePack", keg::Value::basic(offsetof(Options, currTexturePack), keg::BasicType::STRING));
    kt.addValue("maxFps", keg::Value::basic(offsetof(Options, maxFPS), keg::BasicType::F32));
    kt.addValue("motionBlur", keg::Value::basic(offsetof(Options, motionBlur), keg::BasicType::I32));
    kt.addValue("msaa", keg::Value::basic(offsetof(Options, msaa), keg::BasicType::I32));
    // Sound Options
    kt.addValue("musicVolume", keg::Value::basic(offsetof(Options, musicVolume), keg::BasicType::F32));
    kt.addValue("effectVolume", keg::Value::basic(offsetof(Options, effectVolume), keg::BasicType::F32));
    // Game Options
    kt.addValue("mouseSensitivity", keg::Value::basic(offsetof(Options, mouseSensitivity), keg::BasicType::F32));
    kt.addValue("invertMouse", keg::Value::basic(offsetof(Options, invertMouse), keg::BasicType::BOOL));
}

std::vector<ui32v2> SCREEN_RESOLUTIONS;

Options graphicsOptions;

bool loadOptions(const cString filePath) {
    vio::IOManager ioManager; // TODO: Pass in a real boy
    const cString data = ioManager.readFileToString(filePath);

    keg::ReadContext context;
    context.env = keg::getGlobalEnvironment();
    context.reader.init(data);
    keg::Node node = context.reader.getFirst();
    keg::parse((ui8*)&graphicsOptions, node, context, &KEG_GLOBAL_TYPE(Options));

    context.reader.dispose();
    delete[] data;
    return true;
}