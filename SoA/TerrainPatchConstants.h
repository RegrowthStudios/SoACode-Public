///
/// TerrainPatchConstants.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 17 Feb 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Constants for terrain patch stuff
///

#pragma once

#ifndef TerrainPatchConstants_h__
#define TerrainPatchConstants_h__

const int PATCH_NORMALMAP_PIXELS_PER_QUAD = 4; ///< Pixels of normalMap per quad
const int PATCH_WIDTH = 33; ///< Width of patches in vertices
const int PADDED_PATCH_WIDTH = PATCH_WIDTH + 2; ///< Width of patches in vertices
const int PATCH_SIZE = PATCH_WIDTH * PATCH_WIDTH; ///< Size of patches in vertices
const int PATCH_NORMALMAP_WIDTH = (PATCH_WIDTH - 1) * PATCH_NORMALMAP_PIXELS_PER_QUAD + 2; ///< Width of normalmap in pixels, + 2 for padding
const int PATCH_HEIGHTMAP_WIDTH = PATCH_NORMALMAP_WIDTH + 2; ///< Width of heightmap in pixels, + 2 for padding
const int TEXELS_PER_PATCH = PATCH_NORMALMAP_WIDTH - 2; ///< The number of texels contained in a patch.

const int NUM_SKIRTS = 4;
const int INDICES_PER_QUAD = 6; ///< Indices used to render a quad with glDrawElements
const int PATCH_INDICES = (PATCH_WIDTH - 1) * (PATCH_WIDTH - 1 + NUM_SKIRTS) * INDICES_PER_QUAD; ///< Total indices in a patch
const int PATCH_INDICES_NO_SKIRTS = (PATCH_WIDTH - 1) * (PATCH_WIDTH - 1) * INDICES_PER_QUAD; ///< Total indices in a patch with no skirts

#endif // TerrainPatchConstants_h__
