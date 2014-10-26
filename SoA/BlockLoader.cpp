#include "stdafx.h"
#include "BlockLoader.h"
#include "IOManager.h"

#include <yaml-cpp/yaml.h>

#include <boost/algorithm/string/replace.hpp>
#include "BlockData.h"

bool BlockLoader::loadBlocks(const nString& filePath) {
    IOManager ioManager;
    nString data;
    ioManager.readFileToString(filePath.c_str(), data);

    // TODO(Cristian): Implement this

    return false;
}

bool BlockLoader::saveBlocks(const nString& filePath) {

    // Exit since its not implemented
    return true;

    std::ofstream file(filePath);
    if (file.fail()) {
        return false;
    }

    // TODO(Cristian): Implement this

    for (size_t i = 0; i < Blocks.size(); i++) {
        if (Blocks[i].active) {
            // Water is a special case. We have 100 water block IDs, but we only want to write water once.
            if (i >= LOWWATER) {
                if (i == LOWWATER) {
                   // Write a single water block here with Water as the name
                }
                continue;
            }

            // Write non-water blocks here
         
        }
    }
    file.close();
    return true;
}