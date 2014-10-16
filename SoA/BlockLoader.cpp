#include "stdafx.h"
#include "BlockLoader.h"

#include <boost/algorithm/string/replace.hpp>
#include "BlockData.h"

void BlockLoader::loadBlocks(nString filePath) {
   
}

void BlockLoader::saveBlocks(nString filePath) {
    std::ofstream file(filePath);
    for (size_t i = 0; i < Blocks.size(); i++) {
        if (Blocks[i].active) {
            if (i >= LOWWATER) {
                if (i == LOWWATER) {
                    file << "Water:\n";
                } else {
                    continue;
                }
            } else {
                file << Blocks[i].name << ":\n";
            }

            nString data = "  " + Keg::write(&Blocks[i], "Block", nullptr);
            // This is hacky until cristian changes write
            boost::replace_all(data, "\n", "\n  ");

            file << data << std::endl;
        }
    }
    file.close();
}