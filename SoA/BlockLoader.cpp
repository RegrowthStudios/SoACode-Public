#include "stdafx.h"
#include "BlockLoader.h"

#include <boost/algorithm/string/replace.hpp>

#include "BlockData.h"
#include "Chunk.h"
#include "Errors.h"
#include "GameManager.h"
#include "IOManager.h"
#include "Keg.h"
#include "TexturePackLoader.h"

bool BlockLoader::loadBlocks(const nString& filePath) {
    IOManager ioManager; // TODO: Pass in a real boy
    const cString data = ioManager.readFileToString(filePath.c_str());

    YAML::Node node = YAML::Load(data);
    if (node.IsNull() || !node.IsMap()) {
        delete[] data;
        return false;
    }

    Block b;
    Blocks.resize(numBlocks);
    Keg::Value v = Keg::Value::custom("Block", 0);
    for (auto& kvp : node) {
        b.name = kvp.first.as<nString>();
        Keg::parse((ui8*)&b, kvp.second, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(Block));

        // Bit of post-processing on the block
        postProcessBlockLoad(&b);

        Blocks[b.ID] = b;
    }
    delete[] data;

    // Set up the water blocks
    if (!(SetWaterBlocks(LOWWATER))) return 0;

    return true;
}

bool BlockLoader::saveBlocks(const nString& filePath) {
    // Open the portal to Hell
    std::ofstream file(filePath);
    if (file.fail()) return false;

    // Emit data
    YAML::Emitter e;
    e << YAML::BeginMap;
    for (size_t i = 0; i < Blocks.size(); i++) {
        if (Blocks[i].active) {
            // TODO: Water is a special case. We have 100 water block IDs, but we only want to write water once.
            if (i >= LOWWATER && i != LOWWATER) continue;

            // Write the block name first
            e << YAML::Key << Blocks[i].name;
            // Write the block data now
            e << YAML::Value;
            e << YAML::BeginMap;
            Keg::write((ui8*)&(Blocks[i]), e, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(Block));
            e << YAML::EndMap;
        }
    }
    e << YAML::EndMap;


    file << e.c_str();
    file.flush();
    file.close();
    return true;
}


void BlockLoader::postProcessBlockLoad(Block* block) {
    block->active = true;
    GameManager::texturePackLoader->registerBlockTexture(block->topTexName);
    GameManager::texturePackLoader->registerBlockTexture(block->leftTexName);
    GameManager::texturePackLoader->registerBlockTexture(block->rightTexName);
    GameManager::texturePackLoader->registerBlockTexture(block->backTexName);
    GameManager::texturePackLoader->registerBlockTexture(block->frontTexName);
    GameManager::texturePackLoader->registerBlockTexture(block->bottomTexName);

    // Pack light color
    block->lightColorPacked = ((ui16)block->lightColor.r << LAMP_RED_SHIFT) |
        ((ui16)block->lightColor.g << LAMP_GREEN_SHIFT) |
        (ui16)block->lightColor.b;
}


i32 BlockLoader::SetWaterBlocks(int startID) {
    float weight = 0;
    Blocks[startID].name = "Water (1)"; //now build rest of water blocks
    for (int i = startID + 1; i <= startID + 99; i++) {
        if (Blocks[i].active) {
            char buffer[1024];
            sprintf(buffer, "ERROR: Block ID %d reserved for Water is already used by %s", i, Blocks[i].name);
            showMessage(buffer);
            return 0;
        }
        Blocks[i] = Blocks[startID];
        Blocks[i].name = "Water (" + std::to_string(i - startID + 1) + ")";
        Blocks[i].waterMeshLevel = i - startID + 1;
    }
    for (int i = startID + 100; i < startID + 150; i++) {
        if (Blocks[i].active) {
            char buffer[1024];
            sprintf(buffer, "ERROR: Block ID %d reserved for Pressurized Water is already used by %s", i, Blocks[i].name);
            showMessage(buffer);
            return 0;
        }
        Blocks[i] = Blocks[startID];
        Blocks[i].name = "Water Pressurized (" + std::to_string(i - (startID + 99)) + ")";
        Blocks[i].waterMeshLevel = i - startID + 1;
    }
}