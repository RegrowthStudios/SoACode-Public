#include "stdafx.h"
#include "BlockLoader.h"

#include <boost/algorithm/string/replace.hpp>

#include "BlockData.h"
#include "IOManager.h"
#include "Keg.h"

bool BlockLoader::loadBlocks(const nString& filePath) {
    IOManager ioManager;
    nString data;
    ioManager.readFileToString(filePath.c_str(), data);

    // TODO(Cristian): Implement this

    return false;
}

bool BlockLoader::saveBlocks(const nString& filePath) {
    // Open the portal to Hell
    std::ofstream file(filePath);
    if (file.fail()) return false;

    // TODO(Cristian): Implement this

    // Emit data
    //YAML::Emitter e;
    //e << YAML::BeginSeq;
    //for (size_t i = 0; i < Blocks.size(); i++) {
    //    if (Blocks[i].active) {
    //        // Water is a special case. We have 100 water block IDs, but we only want to write water once.
    //        if (i >= LOWWATER && i != LOWWATER) continue;

    //        // Encapsulation hack
    //        e << YAML::BeginMap;

    //        // Write the block name first
    //        e << YAML::BeginMap;
    //        e << YAML::Key << "name" << YAML::Value << Blocks[i].name;
    //        e << YAML::EndMap;

    //        // Write the block data now
    //        Keg::write((ui8*)&Blocks[i], e, nullptr, &KEG_GLOBAL_TYPE(Block));
    //        e << YAML::EndMap;
    //    }
    //}
    //e << YAML::EndSeq;


    //file << e.c_str();
    //file.flush();
    //file.close();
    return true;
}