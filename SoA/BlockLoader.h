#pragma once

class BlockLoader
{
public:
    /// Loads blocks from a .yml file
    /// @param filePath: The .yml file path
    /// @return true on success, false on failure
    static bool loadBlocks(const nString& filePath);

    /// Saves blocks to a .yml file
    /// @param filePath: The .yml file path
    /// @return true on success, false on failure
    static bool saveBlocks(const nString& filePath);
};

