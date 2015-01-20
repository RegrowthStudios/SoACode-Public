#pragma once

const i16 WSO_DONT_CARE_ID = 0xffff;
#define WSO_MAX_SIZE 8
#define WSO_NAME_MAX_LENGTH 128

// Stores All Combinations Of A WSO
class WSOData {
public:
    // The Number Of Blocks Inside This WSO
    const i32& getBlockCount() const {
        return size.x * size.y * size.z;
    }

    // The Name Of This WSO
    cString name;

    // The Index Of This WSO In The Atlas
    i32 index;

    // Necessary IDs For The WSO To Exist
    i16* wsoIDs;

    // The Size Of The WSO
    i32v3 size;

    // The Model File (If It Wants One)
    cString modelFile;
};