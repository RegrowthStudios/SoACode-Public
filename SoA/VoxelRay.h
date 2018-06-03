#pragma once

// Traverses The Endless Space Of Local Voxels
class VoxelRay {
public:
    // Create A Ray At The Starting Local Position With A Normalized Direction
    VoxelRay(f64v3 start, f64v3 direction);

    // Traverse To The Next Voxel And Return The Local Coordinates (Grid Offset)
    i32v3 getNextVoxelPosition();

    // Access The Origin Values
    const f64v3& getStartPosition() const {
        return _startPos;
    }
    const f64v3& getDirection() const {
        return _direction;
    }

    // The Total Distance The Ray Has Traversed
    const f64& getDistanceTraversed() const {
        return _currentDist;
    }
private:
    // Initialization Values
    f64v3 _startPos;
    f64v3 _direction;

    // The Current Traversal Information
    f64 _currentDist;
    f64v3 _currentPos;

    // The Offset From The Chunk Grid Origin
    i32v3 _currentVoxelPos;
};