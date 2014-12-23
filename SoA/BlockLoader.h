#pragma once

#include "BlockData.h"

class BlockPack;
class IOManager;

class BlockLoader
{
public:
    /// Loads blocks from a .yml file
    /// @param filePath: The .yml file path
    /// @param pack: Depository for all loaded blocks
    /// @return true on success, false on failure
    static bool loadBlocks(const nString& filePath, BlockPack* pack);

    /// Saves blocks to a .yml file
    /// @param filePath: The .yml file path
    /// @param pack: Source of block data
    /// @return true on success, false on failure
    static bool saveBlocks(const nString& filePath, BlockPack* pack);
private:

    /// Does some needed postprocessing on a block after load
    /// @param block: the block to process
    static void postProcessBlockLoad(Block* block, IOManager* ioManager);

    /// Sets up the water blocks. This is temporary
    /// @param blocks: Output list for blocks
    static void SetWaterBlocks(std::vector<Block>& blocks);
};

