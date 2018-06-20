#pragma once
#include "Vorb/types.h"

// Parses A File Into A Block Of Data
/*
 * Format:
 * type MiscCharacters{data}
 * Example:
 * c - The ASCII A - {0x41}
 */
i32 ByteBlit(const cString file, void* dst, i32 maxSize);