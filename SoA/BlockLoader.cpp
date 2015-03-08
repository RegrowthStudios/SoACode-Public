#include "stdafx.h"
#include "BlockLoader.h"

#include <Vorb/io/IOManager.h>
#include <Vorb/io/Keg.h>

#include "BlockPack.h"
#include "Chunk.h"
#include "Errors.h"
#include "GameManager.h"
#include "TexturePackLoader.h"


bool BlockLoader::loadBlocks(const nString& filePath, BlockPack* pack) {
    vio::IOManager iom; // TODO: Pass in a real boy

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
    keg::YAMLWriter writer;
    writer.push(keg::WriterParam::BEGIN_MAP);
    for (size_t i = 0; i < blocks.size(); i++) {
        if (blocks[i].active) {
            // TODO: Water is a special case. We have 100 water block IDs, but we only want to write water once.
            if (i >= LOWWATER && i != LOWWATER) continue;

            // Write the block name first
            writer.push(keg::WriterParam::KEY) << blocks[i].name;
            // Write the block data now
            writer.push(keg::WriterParam::VALUE);
            writer.push(keg::WriterParam::BEGIN_MAP);
            keg::write((ui8*)&(blocks[i]), writer, keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(Block));
            writer.push(keg::WriterParam::END_MAP);
        }
    }
    writer.push(keg::WriterParam::END_MAP);

    file << writer.c_str();
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

bool BlockLoader::load(const vio::IOManager* iom, const cString filePath, BlockPack* pack) {
    // Read file
    const cString data = iom->readFileToString(filePath);
    if (!data) return false;

    // Convert to YAML
    keg::YAMLReader reader;
    reader.init(data);
    keg::Node node = reader.getFirst();
    if (keg::getType(node) != keg::NodeType::MAP) {
        delete[] data;
        reader.dispose();
        return false;
    }

    // Load all block nodes
    std::vector<Block> loadedBlocks;
    auto f = createDelegate<const nString&, keg::Node>([&] (Sender, const nString& name, keg::Node value) {
        // Add a block
        loadedBlocks.emplace_back();
        Block& b = loadedBlocks.back();

        // Set name to key
        b.name = name;
        
        // Load data
        keg::parse((ui8*)&b, value, reader, keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(Block));
    });
    reader.forAllInMap(node, f);
    delete f;
    reader.dispose();
    delete[] data;

    // Add blocks to pack
    pack->append(loadedBlocks.data(), loadedBlocks.size());

    return true;
}


GameBlockPostProcess::GameBlockPostProcess(const vio::IOManager* iom, TexturePackLoader* tpl, CaPhysicsTypeDict* caCache) :
    m_iom(iom),
    m_texPackLoader(tpl),
    m_caCache(caCache) {
    // Empty
}

void GameBlockPostProcess::invoke(Sender s, ui16 id) {
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

