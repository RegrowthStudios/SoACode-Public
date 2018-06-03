//
// ClientState.h
// Seed of Andromeda
//
// Created by Benjamin Arnold on 11 Aug 2015
// Copyright 2014 Regrowth Studios
// All Rights Reserved
//
// Summary:
// State for the client only.
//

#pragma once

#ifndef ClientState_h__
#define ClientState_h__

#include "BlockTextureLoader.h"
#include "BlockTexturePack.h"
#include "Camera.h"
#include "MainMenuSystemViewer.h"
#include "ModPathResolver.h"
#include "VoxelEditor.h"

class DebugRenderer;

class ChunkMeshManager;

struct ClientState {
    ChunkMeshManager* chunkMeshManager = nullptr;
    // TODO(Ben): Commonstate
    DebugRenderer* debugRenderer = nullptr;
    MainMenuSystemViewer* systemViewer = nullptr;

    BlockTextureLoader blockTextureLoader;
    BlockTexturePack* blockTextures = nullptr;
    ModPathResolver texturePathResolver;
    VoxelEditor voxelEditor; // TODO(Ben): Should maybe be server side?

    vecs::EntityID startingPlanet = 0;
    vecs::EntityID playerEntity = 0;

    // TODO(Ben): This is temporary!
    CinematicCamera spaceCamera; ///< The camera that looks at the planet from space

    bool isNewGame = true;
    f64v3 startSpacePos = f64v3(0.0f); ///< Starting position of player entity
};

#endif // ClientState_h__
