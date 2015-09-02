#include "stdafx.h"
#include "VoxelRay.h"

#include <float.h>

#include <Vorb/utils.h>

inline f64 fastFloorf(f64 x) {
    return FastConversion<f64, f64>::floor(x);
}
inline f64 fastCeilf(f64 x) {
    return FastConversion<f64, f64>::ceiling(x);
}

VoxelRay::VoxelRay(f64v3 start, f64v3 direction) {
    _startPos = start;
    _direction = direction;
    _currentPos = _startPos;
    _currentVoxelPos = i32v3(fastFloor(_startPos.x), fastFloor(_startPos.y), fastFloor(_startPos.z));
    _currentDist = 0.0f;
}

i32v3 VoxelRay::getNextVoxelPosition() {
    //Find All Distances To Next Voxel In Each Direction
    f64v3 next;
    f64v3 r;

    // X-Distance
    if (_direction.x > 0) {
        if (_currentPos.x == (i32)_currentPos.x) next.x = _currentPos.x + 1;
        else next.x = fastCeilf(_currentPos.x);
        r.x = (next.x - _currentPos.x) / _direction.x;
    } else if (_direction.x < 0) {
        if (_currentPos.x == (i32)_currentPos.x) next.x = _currentPos.x - 1;
        else next.x = fastFloorf(_currentPos.x);
        r.x = (next.x - _currentPos.x) / _direction.x;
    } else {
        r.x = FLT_MAX;
    }

    // Y-Distance
    if (_direction.y > 0) {
        if (_currentPos.y == (i32)_currentPos.y) next.y = _currentPos.y + 1;
        else next.y = fastCeilf(_currentPos.y);
        r.y = (next.y - _currentPos.y) / _direction.y;
    } else if (_direction.y < 0) {
        if (_currentPos.y == (i32)_currentPos.y) next.y = _currentPos.y - 1;
        else next.y = fastFloorf(_currentPos.y);
        r.y = (next.y - _currentPos.y) / _direction.y;
    } else {
        r.y = FLT_MAX;
    }

    // Z-Distance
    if (_direction.z > 0) {
        if (_currentPos.z == (i32)_currentPos.z) next.z = _currentPos.z + 1;
        else next.z = fastCeilf(_currentPos.z);
        r.z = (next.z - _currentPos.z) / _direction.z;
    } else if (_direction.z < 0) {
        if (_currentPos.z == (i32)_currentPos.z) next.z = _currentPos.z - 1;
        else next.z = fastFloorf(_currentPos.z);
        r.z = (next.z - _currentPos.z) / _direction.z;
    } else {
        r.z = FLT_MAX;
    }

    // Get Minimum Movement To The Next Voxel
    f64 rat;
    if (r.x < r.y && r.x < r.z) {
        // Move In The X-Direction
        rat = r.x;
        _currentPos += _direction * rat;
        if (_direction.x > 0) _currentVoxelPos.x++;
        else if (_direction.x < 0) _currentVoxelPos.x--;
    } else if (r.y < r.z) {
        // Move In The Y-Direction
        rat = r.y;
        _currentPos += _direction * rat;
        if (_direction.y > 0) _currentVoxelPos.y++;
        else if (_direction.y < 0) _currentVoxelPos.y--;
    } else {
        // Move In The Z-Direction
        rat = r.z;
        _currentPos += _direction * rat;
        if (_direction.z > 0) _currentVoxelPos.z++;
        else if (_direction.z < 0) _currentVoxelPos.z--;
    }

    // Add The Distance The Ray Has Traversed
    _currentDist += rat;

    return _currentVoxelPos;
}
