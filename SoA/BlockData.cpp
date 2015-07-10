#include "stdafx.h"
#include "BlockData.h"

#include "BlockPack.h"
#include "Errors.h"
#include "GameManager.h"
#include "SoaOptions.h"
#include "Rendering.h"
#include "ZipFile.h"

KEG_ENUM_DEF(BlockOcclusion, BlockOcclusion, e) {
    e.addValue("none", BlockOcclusion::NONE);
    e.addValue("self", BlockOcclusion::SELF);
    e.addValue("selfOnly", BlockOcclusion::SELF_ONLY);
    e.addValue("all", BlockOcclusion::ALL);
}

KEG_TYPE_DEF_SAME_NAME(Block, kt) {
    kt.addValue("ID", keg::Value::basic(offsetof(Block, temp), keg::BasicType::I32));
    kt.addValue("name", keg::Value::basic(offsetof(Block, name), keg::BasicType::STRING));
    kt.addValue("burnTransformID", keg::Value::basic(offsetof(Block, burnTransformID), keg::BasicType::STRING));
    kt.addValue("waveEffect", keg::Value::basic(offsetof(Block, waveEffect), keg::BasicType::I16));
    kt.addValue("lightColor", keg::Value::basic(offsetof(Block, lightColor), keg::BasicType::UI8_V3));
    kt.addValue("caPhysics", keg::Value::basic(offsetof(Block, caFilePath), keg::BasicType::STRING));
    kt.addValue("waterMeshLevel", keg::Value::basic(offsetof(Block, waterMeshLevel), keg::BasicType::I16));
    kt.addValue("floatingAction", keg::Value::basic(offsetof(Block, floatingAction), keg::BasicType::I16));
    kt.addValue("occlusion", keg::Value::custom(offsetof(Block, occlude), "BlockOcclusion", true));
    kt.addValue("spawnID", keg::Value::basic(offsetof(Block, spawnerID), keg::BasicType::STRING));
    kt.addValue("sinkID", keg::Value::basic(offsetof(Block, sinkID), keg::BasicType::STRING));
    kt.addValue("explosionRays", keg::Value::basic(offsetof(Block, explosionRays), keg::BasicType::UI16));
    kt.addValue("meshType", keg::Value::custom(offsetof(Block, meshType), "MeshType", true));
    kt.addValue("moveMod", keg::Value::basic(offsetof(Block, moveMod), keg::BasicType::F32));
    kt.addValue("explosionResistance", keg::Value::basic(offsetof(Block, explosionResistance), keg::BasicType::F32));
    kt.addValue("explosionPower", keg::Value::basic(offsetof(Block, explosivePower), keg::BasicType::F32));
    kt.addValue("flammability", keg::Value::basic(offsetof(Block, flammability), keg::BasicType::F32));
    kt.addValue("explosionPowerLoss", keg::Value::basic(offsetof(Block, explosionPowerLoss), keg::BasicType::F32));
    kt.addValue("lightColorFilter", keg::Value::basic(offsetof(Block, colorFilter), keg::BasicType::F32_V3));
    kt.addValue("emitter", keg::Value::basic(offsetof(Block, emitterName), keg::BasicType::STRING));
    kt.addValue("movesPowder", keg::Value::basic(offsetof(Block, powderMove), keg::BasicType::BOOL));
    kt.addValue("collide", keg::Value::basic(offsetof(Block, collide), keg::BasicType::BOOL));
    kt.addValue("waterBreak", keg::Value::basic(offsetof(Block, waterBreak), keg::BasicType::BOOL));
    kt.addValue("scatterSunRays", keg::Value::basic(offsetof(Block, blockLight), keg::BasicType::BOOL));
    kt.addValue("useable", keg::Value::basic(offsetof(Block, useable), keg::BasicType::BOOL));
    kt.addValue("allowsLight", keg::Value::basic(offsetof(Block, allowLight), keg::BasicType::BOOL));
    kt.addValue("crushable", keg::Value::basic(offsetof(Block, isCrushable), keg::BasicType::BOOL));
    kt.addValue("supportive", keg::Value::basic(offsetof(Block, isSupportive), keg::BasicType::BOOL));
}

// TODO(Ben): LOL
Block::Block() : emitterName(""), 
emitterOnBreakName(""), 
emitter(nullptr),
emitterOnBreak(nullptr),
emitterRandom(nullptr),
emitterRandomName(""),
lightColor(0, 0, 0) {
    allowLight = false;
    ID = 0;
    name = particleTexName = "";
    memset(textures, 0, sizeof(textures));
    particleTex = 0;
    collide = true;
    occlude = BlockOcclusion::ALL;
    meshType = MeshType::BLOCK;
    waveEffect = 0;
    explosionResistance = 1.0;
    active = 0;
    useable = 1;
    blockLight = true;
    waterMeshLevel = 0;
    waterBreak = false;
    isCrushable = false;
    floatingAction = 1;
    flammability = 0.0f;
    isSupportive = true;
    explosivePower = 0.0;
    explosionPowerLoss = 0.0;
    explosionRays = 0;
    powderMove = true;
    moveMod = 1.0f;
    spawnerID = "";
    sinkID = "";
    colorFilter = f32v3(1.0f);
}
