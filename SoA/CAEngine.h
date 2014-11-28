#pragma once
#include "CellularAutomataTask.h"
#include "Constants.h"
#include "Keg.h"
#include "LiquidData.h"

class ChunkManager;
class Chunk;
class IOManager;

/// Resolution of CA updates in frames
#define CA_TICK_RES 4
/// Should be half of CA_TICK_RES
#define HALF_CA_TICK_RES 2

class CaPhysicsData {
public:
    ui32 liquidLevels = 0;
    ui32 updateRate = 0;
    CA_FLAG caFlag;
};
KEG_TYPE_DECL(CaPhysicsData);

class CaPhysicsType {
public:

    /// Updates the type.
    /// @return true if it this physics type should simulate
    bool update();

    bool loadFromYml(const nString& filePath, IOManager* ioManager);

    // Getters
    const int& getCaIndex() const { return _caIndex; }
    const ui32& getUpdateRate() const { return _data.updateRate; }
    const CA_FLAG& getCaFlag() const { return _data.caFlag; }
    const bool& getIsEven() const { return _isEven; }

    static const int& getNumCaTypes() { return typesCache.size(); }

    static void clearTypes();

    static std::map<nString, CaPhysicsType*> typesCache;
private:
    static std::vector<CaPhysicsType*> typesArray;

    CaPhysicsData _data;
    int _caIndex;
    bool _isEven = false; 

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