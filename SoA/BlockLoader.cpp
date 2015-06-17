#include "stdafx.h"
#include "BlockLoader.h"

#include <Vorb/io/IOManager.h>
#include <Vorb/io/Keg.h>

#include "BlockPack.h"
#include "Chunk.h"
#include "Errors.h"
#include "GameManager.h"

#define BLOCK_MAPPING_PATH "BlockMapping.ini"
#define BLOCK_DATA_PATH "BlockData.yml"

bool BlockLoader::loadBlocks(const vio::IOManager& iom, BlockPack* pack) {
    // Load existing mapping if there is one
    tryLoadMapping(iom, BLOCK_MAPPING_PATH, pack);

    // Clear CA physics cache
    CaPhysicsType::clearTypes();

    GameBlockPostProcess bpp(&iom, &CaPhysicsType::typesCache);
    pack->onBlockAddition += bpp.del;
    if (!BlockLoader::load(&iom, BLOCK_DATA_PATH, pack)) {
        pack->onBlockAddition -= bpp.del;
        return false;
    }
    pack->onBlockAddition -= bpp.del;

    saveMapping(iom, BLOCK_MAPPING_PATH, pack);

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

bool BlockLoader::load(const vio::IOManager* iom, const cString filePath, BlockPack* pack) {
    // Read file
    nString data;
    iom->readFileToString(filePath, data);
    if (data.empty()) return false;

    // Convert to YAML
    keg::ReadContext context;
    context.env = keg::getGlobalEnvironment();
    context.reader.init(data.c_str());
    keg::Node node = context.reader.getFirst();
    if (keg::getType(node) != keg::NodeType::MAP) {
        context.reader.dispose();
        return false;
    }

    // Load all block nodes
    std::vector<Block> loadedBlocks;
    auto f = makeFunctor<Sender, const nString&, keg::Node>([&] (Sender, const nString& name, keg::Node value) {
        // Add a block
        loadedBlocks.emplace_back();
        Block& b = loadedBlocks.back();

        // Set name to key
        b.name = name;
        
        // Load data
        keg::parse((ui8*)&b, value, context, &KEG_GLOBAL_TYPE(Block));
    });
    context.reader.forAllInMap(node, f);
    delete f;
    context.reader.dispose();

    // Add blocks to pack
    for (auto& b : loadedBlocks) {
        pack->append(b);
    }

    return true;
}


GameBlockPostProcess::GameBlockPostProcess(const vio::IOManager* iom, CaPhysicsTypeDict* caCache) :
    m_iom(iom),
    m_caCache(caCache) {
    del = makeDelegate(*this, &GameBlockPostProcess::invoke);
}

void GameBlockPostProcess::invoke(Sender s, ui16 id) {
    Block& block = ((BlockPack*)s)->operator[](id);
    block.active = true;

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
            // TODO(Ben): Why new?
            // TODO(Ben): This isn't getting stored...
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

bool BlockLoader::tryLoadMapping(const vio::IOManager& iom, const cString filePath, BlockPack* pack) {
    vio::Path path;
    if (!iom.resolvePath(filePath, path)) return false;

    std::ifstream file(path.getCString());
    if (file.fail()) return false;

    // TODO(Ben): Handle comments
    nString token;
    BlockID id;
    while (std::getline(file, token, ':')) {
        // Read the id
        file >> id;
        pack->reserveID(token, id);
        // Get the newline
        char nl;
        file.get(nl);
    }
    return true;
}

bool BlockLoader::saveMapping(const vio::IOManager& iom, const cString filePath, BlockPack* pack) {    
    vio::FileStream fs = iom.openFile(filePath, vio::FileOpenFlags::WRITE_ONLY_CREATE);
    if (!fs.isOpened()) pError("Failed to open block mapping file for save");
    for (auto& b : pack->getBlockMap()) {
        fs.write("%s: %d\n", b.first.c_str(), b.second);
    }
    return true;
}
