///
/// VoxelBits.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 6 Nov 2014
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Contains utils for accessing voxel bits
/// TODO: Use C++ bit accessors
///

#pragma once

#ifndef VoxelBits_h__
#define VoxelBits_h__

//For lamp colors. Used to extract color values from the 16 bit color code
#define LAMP_RED_MASK 0x7C00
#define LAMP_GREEN_MASK 0x3E0
#define LAMP_BLUE_MASK 0x1f
#define LAMP_RED_SHIFT 10
#define LAMP_GREEN_SHIFT 5
#define FLORA_HEIGHT_MASK 0x1f
#define FLORA_YPOS_MASK 0x3E0
#define FLORA_YPOS_SHIFT 5
//no blue shift

namespace VoxelBits {

    inline ui16 getFloraHeight(ui16 b) {
        return b & FLORA_HEIGHT_MASK;
    }

    inline ui16 getFloraPosition(ui16 b) {
        return (b & FLORA_YPOS_MASK) >> FLORA_YPOS_SHIFT;
    }

    inline ui16 getLampRed(ui16 b) {
        return b & LAMP_RED_MASK;
    }

    inline ui16 getLampGreen(ui16 b) {
        return b & LAMP_GREEN_MASK;
    }

    inline ui16 getLampBlue(ui16 b) {
        return b & LAMP_BLUE_MASK;
    }

    inline void setFloraHeight(ui16& b, ui16 floraHeight) {
        b = (b & (~FLORA_HEIGHT_MASK)) | floraHeight;
    }

    inline void setFloraPosition(ui16& b, ui16 yPos) {
        b = (b & (~FLORA_YPOS_MASK)) | (yPos << FLORA_YPOS_SHIFT);
    }

    inline ui16 getLampRedFromHex(ui16 color) { 
        return (color & LAMP_RED_MASK) >> LAMP_RED_SHIFT; 
    }

    inline ui16 getLampGreenFromHex(ui16 color) { 
        return (color & LAMP_GREEN_MASK) >> LAMP_GREEN_SHIFT;
    }

    inline ui16 getLampBlueFromHex(ui16 color) { 
        return color & LAMP_BLUE_MASK; 
    }

}

#endif // VoxelBits_h__