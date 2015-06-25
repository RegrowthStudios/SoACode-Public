#include "stdafx.h"
#include "BlockLoader.h"

#include <Vorb/io/IOManager.h>
#include <Vorb/io/Keg.h>

#include "BlockPack.h"
#include "Chunk.h"
#include "Errors.h"
#include "GameManager.h"
#include "VoxelBits.h"

#define BLOCK_MAPPING_PATH "BlockMapping.ini"
#define BLOCK_DATA_PATH "BlockData.yml"
#define BINARY_CACHE_PATH "BlockCache.bin"

bool BlockLoader::loadBlocks(const vio::IOManager& iom, BlockPack* pack) {
    // Load existing mapping if there is one
    tryLoadMapping(iom, BLOCK_MAPPING_PATH, pack);

    // Check for binary cache
    vio::Path binPath;
    bool useCache = false;
    if (iom.resolvePath(BINARY_CACHE_PATH, binPath)) {
        vio::Path dataPath;
        if (!iom.resolvePath(BLOCK_DATA_PATH, dataPath)) return false;
        if (binPath.getLastModTime() >= dataPath.getLastModTime()) {
            useCache = true;
        }
    }
   
    // Clear CA physics cache
    CaPhysicsType::clearTypes();

    GameBlockPostProcess bpp(&iom, &CaPhysicsType::typesCache);
    pack->onBlockAddition += bpp.del;
    if (useCache) {
        if (!BlockLoader::loadBinary(iom, BINARY_CACHE_PATH, pack)) {
            printf("Failed to load binary cache %s\n", BINARY_CACHE_PATH);
            if (!BlockLoader::load(iom, BLOCK_DATA_PATH, pack)) {
                pack->onBlockAddition -= bpp.del;
                return false;
            }
        }
    } else {
        if (!BlockLoader::load(iom, BLOCK_DATA_PATH, pack)) {
            pack->onBlockAddition -= bpp.del;
            return false;
        }
    }
    pack->onBlockAddition -= bpp.del;

    saveMapping(iom, BLOCK_MAPPING_PATH, pack);
    if (!useCache) {
   //     saveBinary(iom, BINARY_CACHE_PATH, pack);
    }

    return true;
}

// Conditional keg write
#define COND_WRITE_KEG(key, var) if (b.##var != d.##var) { writer.push(keg::WriterParam::KEY) << nString(key); writer.push(keg::WriterParam::VALUE) << b.##var; } 

bool BlockLoader::saveBlocks(const nString& filePath, BlockPack* pack) {
    // Open the portal to Hell
    std::ofstream file(filePath);
    if (file.fail()) return false;

    BlockPack& blocks = *pack;

    std::map<nString, const Block*> sortMap;
    const std::vector<Block>& blockList = blocks.getBlockList();
    for (int i = 0; i < blockList.size(); i++) {
        const Block& b = blockList[i];
        if (b.active) {
            sortMap[b.sID] = &b;
        }
    }
    // Default block
    Block d;
    // Emit data
    keg::YAMLWriter writer;
    writer.push(keg::WriterParam::BEGIN_MAP);
    for (auto& it : sortMap) {
        const Block& b = *it.second;

        // Write the block name first
        writer.push(keg::WriterParam::KEY) << b.sID;
        // Write the block data now
        writer.push(keg::WriterParam::VALUE);
        writer.push(keg::WriterParam::BEGIN_MAP);

        COND_WRITE_KEG("allowsLight", allowLight);
        COND_WRITE_KEG("collide", collide);
        COND_WRITE_KEG("crushable", isCrushable);
        COND_WRITE_KEG("explosionPower", explosivePower);
        COND_WRITE_KEG("explosionPowerLoss", explosionPowerLoss);
        COND_WRITE_KEG("explosionRays", explosionRays);
        COND_WRITE_KEG("explosionResistance", explosionResistance);
        COND_WRITE_KEG("flammability", flammability);
        COND_WRITE_KEG("floatingAction", floatingAction);
        COND_WRITE_KEG("lightColorFilter", colorFilter);
        if (b.meshType != d.meshType) {
            writer.push(keg::WriterParam::KEY) << nString("meshType");
            switch (b.meshType) {
                case MeshType::NONE:
                    writer.push(keg::WriterParam::VALUE) << nString("none");
                    break;
                case MeshType::BLOCK:
                    writer.push(keg::WriterParam::VALUE) << nString("cube");
                    break;
                case MeshType::LEAVES:
                    writer.push(keg::WriterParam::VALUE) << nString("leaves");
                    break;
                case MeshType::FLORA:
                    writer.push(keg::WriterParam::VALUE) << nString("triangle");
                    break;
                case MeshType::CROSSFLORA:
                    writer.push(keg::WriterParam::VALUE) << nString("cross");
                    break;
                case MeshType::LIQUID:
                    writer.push(keg::WriterParam::VALUE) << nString("liquid");
                    break;
                case MeshType::FLAT:
                    writer.push(keg::WriterParam::VALUE) << nString("flat");
                    break;
            }
        }
        COND_WRITE_KEG("moveMod", moveMod);
        COND_WRITE_KEG("name", sID); // TEMPORARY
        switch (b.occlude) {
            case BlockOcclusion::NONE:
                writer.push(keg::WriterParam::KEY) << nString("occlusion");
                writer.push(keg::WriterParam::VALUE) << nString("none");
                break;
            case BlockOcclusion::SELF:
                writer.push(keg::WriterParam::KEY) << nString("occlusion");
                writer.push(keg::WriterParam::VALUE) << nString("self");
                break;
            case BlockOcclusion::SELF_ONLY:
                writer.push(keg::WriterParam::KEY) << nString("occlusion");
                writer.push(keg::WriterParam::VALUE) << nString("selfOnly");
                break;
        }
        COND_WRITE_KEG("sinkVal", sinkVal);
        COND_WRITE_KEG("spawnerVal", spawnerVal);
        COND_WRITE_KEG("supportive", isSupportive);
        COND_WRITE_KEG("waterBreak", waterBreak);

        //keg::write((ui8*)b, writer, keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(Block));
        writer.push(keg::WriterParam::END_MAP);
        
    }
    writer.push(keg::WriterParam::END_MAP);

    file << writer.c_str();
    file.flush();
    file.close();
    return true;
}

bool BlockLoader::load(const vio::IOManager& iom, const cString filePath, BlockPack* pack) {
    // Read file
    nString data;
    iom.readFileToString(filePath, data);
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
    auto f = makeFunctor<Sender, const nString&, keg::Node>([&] (Sender, const nString& key, keg::Node value) {
        // Add a block
        loadedBlocks.emplace_back();
        Block& b = loadedBlocks.back();

        // Set sID to key
        b.sID = key;
        
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

bool BlockLoader::saveMapping(const vio::IOManager& iom, const cString filePath, BlockPack* pack) {    
    vio::FileStream fs = iom.openFile(filePath, vio::FileOpenFlags::WRITE_ONLY_CREATE);
    if (!fs.isOpened()) pError("Failed to open block mapping file for save");

    for (auto& b : pack->getBlockMap()) {
        fs.write("%s: %d\n", b.first.c_str(), b.second);
    }
    return true;
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

bool BlockLoader::saveBinary(const vio::IOManager& iom, const cString filePath, BlockPack* pack) {
    vio::FileStream fs = iom.openFile(filePath, vio::FileOpenFlags::WRITE_ONLY_CREATE | vio::FileOpenFlags::BINARY);
    if (!fs.isOpened()) return false;

    ui32 size = pack->getBlockMap().size();
    ui32 blockSize = sizeof(Block);
    // Write sizes
    fs.write(1, sizeof(ui32), &size);
    fs.write(1, sizeof(ui32), &blockSize);

    // TODO(Ben): This isn't complete.
    const std::vector<Block>& blockList = pack->getBlockList();
    for (auto& b : pack->getBlockMap()) {
        const Block& block = blockList[b.second];
        fs.write("%s", block.name.c_str()); fs.write(1, 1, "\0");
        for (int i = 0; i < 6; i++) {
            fs.write("%s\0", block.texturePaths[i].c_str()); fs.write(1, 1, "\0");
        }
        fs.write(1, sizeof(bool), &block.powderMove);
        fs.write(1, sizeof(bool), &block.collide);
        fs.write(1, sizeof(bool), &block.waterBreak);
        fs.write(1, sizeof(bool), &block.blockLight);
        fs.write(1, sizeof(bool), &block.useable);
        fs.write(1, sizeof(bool), &block.allowLight);
        fs.write(1, sizeof(bool), &block.isCrushable);
        fs.write(1, sizeof(bool), &block.isSupportive);
    }
    return true;
}

void readStr(vio::FileStream& fs, char* buf) {
    int i = 0;
    do {
        fs.read(1, sizeof(char), buf + i);
    } while (buf[i++] != 0);
}

bool BlockLoader::loadBinary(const vio::IOManager& iom, const cString filePath, BlockPack* pack) {
    vio::FileStream fs = iom.openFile(filePath, vio::FileOpenFlags::READ_ONLY_EXISTING | vio::FileOpenFlags::BINARY);
    if (!fs.isOpened()) return false;

    ui32 size;
    ui32 blockSize;
    // Read sizes
    fs.read(1, sizeof(ui32), &size);
    fs.read(1, sizeof(ui32), &blockSize);
    // Make sure block size didn't change. DEBUG MODE CHANGES THE SIZE!!!
    //if (blockSize != sizeof(Block)) return false;

    char buf[512];

    // TODO(Ben): This isn't complete.
    for (ui32 i = 0; i < size; i++) {
        Block b;
        readStr(fs, buf);
        b.name = buf;
        for (int i = 0; i < 6; i++) {
            readStr(fs, buf);
            b.texturePaths[i] = buf;
        }
        fs.read(1, sizeof(bool), &b.powderMove);
        fs.read(1, sizeof(bool), &b.collide);
        fs.read(1, sizeof(bool), &b.waterBreak);
        fs.read(1, sizeof(bool), &b.blockLight);
        fs.read(1, sizeof(bool), &b.useable);
        fs.read(1, sizeof(bool), &b.allowLight);
        fs.read(1, sizeof(bool), &b.isCrushable);
        fs.read(1, sizeof(bool), &b.isSupportive);
        pack->append(b);
    }
    return true;
}