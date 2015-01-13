#pragma once
#include <Vorb/io/Keg.h>
#include <Vorb/VorbPreDecl.inl>

#include "CellularAutomataTask.h"
#include "Constants.h"
#include "LiquidData.h"

DECL_VIO(class, IOManager)

class ChunkManager;
class Chunk;

/// Resolution of CA updates in frames
#define CA_TICK_RES 4
/// Should be half of CA_TICK_RES
#define HALF_CA_TICK_RES 2

class CaPhysicsData {
public:
    ui32 liquidLevels = 0; ///< Number of block IDs reserved for liquid
    ui32 updateRate = 0; ///< Update speed of the CA
    CA_ALGORITHM alg; ///< CA algorithm to use
};
KEG_TYPE_DECL(CaPhysicsData);

typedef std::map<nString, CaPhysicsType*> CaPhysicsTypeDict;
typedef std::vector<CaPhysicsType*> CaPhysicsTypeList;

class CaPhysicsType {
public:

    /// Updates the type.
    /// @return true if it this physics type should simulate
    bool update();

    /// Loads the data from a yml file
    /// @param filePath: path of the yml file
    /// @param ioManager: IOManager that will read the file
    /// @return true on success
    bool loadFromYml(const nString& filePath, const vio::IOManager* ioManager);

    // Getters
    const int& getCaIndex() const { return _caIndex; }
    const ui32& getUpdateRate() const { return _data.updateRate; }
    const CA_ALGORITHM& getCaAlg() const { return _data.alg; }
    const bool& getIsEven() const { return _isEven; }

    // Static functions
    /// Gets the number of CA types currently cached
    static int getNumCaTypes() { return typesArray.size(); }
    /// Clears all the cached types
    static void clearTypes();

    static CaPhysicsTypeList typesArray; ///< Stores cached types
    static CaPhysicsTypeDict typesCache; ///< Stores cached types mapped to filepath
private:

    CaPhysicsData _data; ///< The algorithm specific data
    int _caIndex; ///< index into typesArray
    bool _isEven = false;  ///< Used for optimization, only update odd or even chunks at any time

    ui32 _ticks = 0; ///< Counts the ticks
};

class CAEngine {
public:
    CAEngine();
    void setChunk(Chunk* chunk) { _chunk = chunk; }
    void updateSpawnerBlocks(bool powders);
    void updateLiquidBlocks(int caIndex);
    void updatePowderBlocks(int caIndex);
private:
    
    void liquidPhysics(i32 startBlockIndex, i32 b);
    void powderPhysics(i32 c);

    i32 _dirIndex = 0;
    i32 _lowIndex;
    i32 _range;
    i32 _highIndex;
    std::vector<ui16> _usedUpdateFlagList;
    bool _blockUpdateFlagList[CHUNK_SIZE];
    Chunk* _chunk;
    Chunk* _lockedChunk;
};