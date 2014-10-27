#include "stdafx.h"
#include "BlockLoader.h"

#include <boost/algorithm/string/replace.hpp>

#include "BlockData.h"
#include "IOManager.h"
#include "Keg.h"

bool BlockLoader::loadBlocks(const nString& filePath) {
    IOManager ioManager; // TODO: Pass in a real boy
    const cString data = ioManager.readFileToString(filePath.c_str());

    // TODO(Cristian): Implement this

    YAML::Node node = YAML::Load(data);
    if (node.IsNull() || !node.IsMap()) {
        delete[] data;
        return false;
    }

    Block b;
    Blocks.resize(numBlocks);
    Keg::Value v = Keg::Value::custom("Block", 0);
    for (auto& kvp : node) {
        nString name = kvp.first.as<nString>();
        Keg::parse((ui8*)&b, kvp.second, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(Block));

        // TODO: Ben's magic gumbo recipe
        Blocks[b.ID] = b;
    }
    delete[] data;

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