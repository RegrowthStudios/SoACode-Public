#include "stdafx.h"
#include "BlockLoader.h"

#include <boost/algorithm/string/replace.hpp>

#include "BlockPack.h"
#include "Chunk.h"
#include "Errors.h"
#include "GameManager.h"
#include "IOManager.h"
#include "Keg.h"
#include "TexturePackLoader.h"

bool BlockLoader::loadBlocks(const nString& filePath, BlockPack* pack) {
    IOManager iom; // TODO: Pass in a real boy

    // Clear CA physics cache
    CaPhysicsType::clearTypes();

    GameBlockPostProcess bpp(&iom, GameManager::texturePackLoader, &CaPhysicsType::typesCache);
    pack->onBlockAddition += &bpp;
    if (!BlockLoader::load(&iom, filePath.c_str(), pack)) {
        pack->onBlockAddition -= &bpp;
        return false;
    }
    pack->onBlockAddition -= &bpp;

    // Set up the water blocks
    std::vector<Block> waterBlocks;
    SetWaterBlocks(waterBlocks);
    pack->append(waterBlocks.data(), waterBlocks.size());
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

bool BlockLoader::load(const IOManager* iom, const cString filePath, BlockPack* pack) {
    // Read file
    const cString data = iom->readFileToString(filePath);
    if (!data) return false;

    // Convert to YAML
    YAML::Node node = YAML::Load(data);
    if (node.IsNull() || !node.IsMap()) {
        delete[] data;
        return false;
    }

    // Load all block nodes
    std::vector<Block> loadedBlocks;
    for (auto& kvp : node) {
        // Add a block
        loadedBlocks.emplace_back();
        Block& b = loadedBlocks.back();

        // Set name to key
        b.name = kvp.first.as<nString>();
        
        // Load data
        Keg::parse((ui8*)&b, kvp.second, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(Block));
    }
    delete[] data;

    // Add blocks to pack
    pack->append(loadedBlocks.data(), loadedBlocks.size());

    return true;
}


GameBlockPostProcess::GameBlockPostProcess(const IOManager* iom, TexturePackLoader* tpl, CaPhysicsTypeDict* caCache) :
    m_iom(iom),
    m_texPackLoader(tpl),
    m_caCache(caCache) {
    // Empty
}

void GameBlockPostProcess::invoke(void* s, ui16 id) {
    Block& block = ((BlockPack*)s)->operator[](id);
    block.active = true;

    // Block textures
    m_texPackLoader->registerBlockTexture(block.topTexName);
    m_texPackLoader->registerBlockTexture(block.leftTexName);
    m_texPackLoader->registerBlockTexture(block.rightTexName);
    m_texPackLoader->registerBlockTexture(block.backTexName);
    m_texPackLoader->registerBlockTexture(block.frontTexName);
    m_texPackLoader->registerBlockTexture(block.bottomTexName);

    // Pack light color
    block.lightColorPacked =
        ((ui16)block.lightColor.r << LAMP_RED_SHIFT) |
        ((ui16)block.lightColor.g << LAMP_GREEN_SHIFT) |
        (ui16)block.lightColor.b;

    // Ca Physics
    if (block.caFilePath.length()) {
        // Check if this physics type was already loaded
        auto it = m_caCache->find(block.caFilePath);
        if (it == m_caCache->end()) {
            CaPhysicsType* newType = new CaPhysicsType();
            // Load in the data
            if (newType->loadFromYml(block.caFilePath, m_iom)) {
                block.caIndex = newType->getCaIndex();
                block.caAlg = newType->getCaAlg();
            } else {
                delete newType;
            }
        } else {
            block.caIndex = it->second->getCaIndex();
            block.caAlg = it->second->getCaAlg();
        }
    }

}

