#include "stdafx.h"
#include "BlockLoader.h"

#include <boost/algorithm/string/replace.hpp>

#include "BlockPack.h"
#include "CAEngine.h"
#include "Chunk.h"
#include "Errors.h"
#include "GameManager.h"
#include "IOManager.h"
#include "Keg.h"
#include "TexturePackLoader.h"

bool BlockLoader::loadBlocks(const nString& filePath, BlockPack* pack) {
    IOManager ioManager; // TODO: Pass in a real boy
    const cString data = ioManager.readFileToString(filePath.c_str());

    YAML::Node node = YAML::Load(data);
    if (node.IsNull() || !node.IsMap()) {
        delete[] data;
        return false;
    }

    // Clear CA physics cache
    CaPhysicsType::clearTypes();

    std::vector<Block> loadedBlocks;

    Keg::Value v = Keg::Value::custom("Block", 0);
    for (auto& kvp : node) {
        Block b;
        b.name = kvp.first.as<nString>();
        Keg::parse((ui8*)&b, kvp.second, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(Block));

        // Bit of post-processing on the block
        postProcessBlockLoad(&b, &ioManager);

        loadedBlocks.push_back(b);
    }
    delete[] data;

    // Set up the water blocks
    SetWaterBlocks(loadedBlocks);

    pack->append(loadedBlocks.data(), loadedBlocks.size());
    
    LOWWATER = (*pack)["Water (1)"].ID; // TODO: Please kill me now... I can't take this kind of nonsense anymore


    return true;
}

bool BlockLoader::saveBlocks(const nString& filePath, BlockPack* pack) {
    // Open the portal to Hell
    std::ofstream file(filePath);
    if (file.fail()) return false;

    BlockPack& blocks = *pack;

    // Emit data
    YAML::Emitter e;
    e << YAML::BeginMap;
    for (size_t i = 0; i < blocks.size(); i++) {
        if (blocks[i].active) {
            // TODO: Water is a special case. We have 100 water block IDs, but we only want to write water once.
            if (i >= LOWWATER && i != LOWWATER) continue;

            // Write the block name first
            e << YAML::Key << blocks[i].name;
            // Write the block data now
            e << YAML::Value;
            e << YAML::BeginMap;
            Keg::write((ui8*)&(blocks[i]), e, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(Block));
            e << YAML::EndMap;
        }
    }
    e << YAML::EndMap;


    file << e.c_str();
    file.flush();
    file.close();
    return true;
}


void BlockLoader::postProcessBlockLoad(Block* block, IOManager* ioManager) {
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
    // Ca Physics
    if (block->caFilePath.length()) {
        // Check if this physics type was already loaded
        auto it = CaPhysicsType::typesCache.find(block->caFilePath);
        if (it == CaPhysicsType::typesCache.end()) {
            CaPhysicsType* newType = new CaPhysicsType();
            // Load in the data
            if (newType->loadFromYml(block->caFilePath, ioManager)) {
                block->caIndex = newType->getCaIndex();
                block->caAlg = newType->getCaAlg();
            } else {
                delete newType;
            }
        } else {
            block->caIndex = it->second->getCaIndex();
            block->caAlg = it->second->getCaAlg();
        }
    }
}


void BlockLoader::SetWaterBlocks(std::vector<Block>& blocks) {
    for (i32 i = 0; i < 100; i++) {
        blocks.push_back(Block());
        blocks.back().name = "Water (" + std::to_string(i + 1) + ")";
        blocks.back().waterMeshLevel = i + 1;
        blocks.back().meshType = MeshType::LIQUID;
    }
    for (i32 i = 100; i < 150; i++) {
        blocks.push_back(Block());
        blocks.back().name = "Water Pressurized (" + std::to_string(i + 1) + ")";
        blocks.back().waterMeshLevel = i + 1;
        blocks.back().meshType = MeshType::LIQUID; // TODO: Should this be here?
    }
}