///
/// TerrainPatchConstants.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 17 Feb 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Constants for terrain patch stuff
///

#pragma once

#ifndef TerrainPatchConstants_h__
#define TerrainPatchConstants_h__

const int PATCH_NORMALMAP_PIXELS_PER_QUAD = 4; ///< Pixels of normalMap per quad
const int PATCH_WIDTH = 33; ///< Width of patches in vertices
const int PATCH_SIZE = PATCH_WIDTH * PATCH_WIDTH; ///< Size of patches in vertices
const int PATCH_NORMALMAP_WIDTH = (PATCH_WIDTH - 1) * PATCH_NORMALMAP_PIXELS_PER_QUAD + 2; ///< Width of normalmap in pixels, + 2 for padding
const int PATCH_HEIGHTMAP_WIDTH = PATCH_NORMALMAP_WIDTH + 2; ///< Width of heightmap in pixels, + 2 for padding

const int PATCH_MAX_LOD = 25; ///< Absolute maximum depth of subdivision // TODO(Ben): This should differ per planet?

const int INDICES_PER_QUAD = 6; ///< Indices used to render a quad with glDrawElements
const int PATCH_INDICES = (PATCH_WIDTH - 1) * (PATCH_WIDTH + 3) * INDICES_PER_QUAD; ///< Total indices in a patch

#endif // TerrainPatchConstants_h__
