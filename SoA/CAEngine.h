#pragma once
#include "CellularAutomataTask.h"
#include "Constants.h"
#include "LiquidData.h"

class ChunkManager;
class Chunk;

/// Resolution of CA updates in frames
#define CA_TICK_RES 4

class CaPhysicsType {
public:
    CaPhysicsType(ui32 ticksUntilUpdate, CA_FLAG caFlag) :
        _ticksUntilUpdate(ticksUntilUpdate),
        _caFlag(caFlag) {
        _caIndex = numCaTypes++;
    }

    /// Updates the type.
    /// @return true if it this physics type should simulate
    bool update() {
        _ticks++;
        if (_ticks == _halfTicksUntilUpdate) {
            _ticks = 0;
            _isEven = !_isEven;
            return true;
        }
        return false;
    }

    // Getters
    const int& getCaIndex() const { return _caIndex; }
    const ui32& getTicksUntilUpdate() const { return _ticksUntilUpdate; }
    const CA_FLAG& getCaFlag() const { return _caFlag; }
    const bool& getIsEven() const { return _isEven; }

    static const int& getNumCaTypes() { return numCaTypes; }

private:
    static int numCaTypes;

    int _caIndex;
    ui32 _ticksUntilUpdate; ///< In units of CA_TICK_RES
    ui32 _halfTicksUntilUpdate; ///< half of _ticksUntilUpdate
    CA_FLAG _caFlag; ///< Determines which algorithm to use
    bool _isEven = false; 

    ui32 _ticks = 0; ///< Counts the ticks
};

class CAEngine {
public:
    CAEngine();
    void setChunk(Chunk* chunk) { _chunk = chunk; }
    void updateSpawnerBlocks(bool powders);
    void updateLiquidBlocks();
    void updatePowderBlocks();
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