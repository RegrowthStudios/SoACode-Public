///
/// SoaState.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 10 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// The main game state for SoA
///

#pragma once

#ifndef SoAState_h__
#define SoAState_h__

#include "SpaceSystem.h"
#include "GameSystem.h"
#include "Camera.h"
#include "MainMenuSystemViewer.h"

#include "ModPathResolver.h"
#include "BlockPack.h"
#include "BlockTexturePack.h"
#include "BlockMaterialLoader.h"

#include <Vorb/io/IOManager.h>
#include <Vorb/ecs/Entity.h>
#include <Vorb/VorbPreDecl.inl>

class ChunkMeshManager;
class DebugRenderer;
class MeshManager;
class PlanetLoader;
class SoaOptions;
DECL_VIO(class IOManager);

struct SoaState {
    SoaState() {}

    SpaceSystem* spaceSystem = nullptr;
    GameSystem* gameSystem = nullptr;

    vecs::EntityID startingPlanet = 0;
    vecs::EntityID playerEntity = 0;

    DebugRenderer* debugRenderer = nullptr;
    MeshManager* meshManager = nullptr;
    ChunkMeshManager* chunkMeshManager = nullptr;
    MainMenuSystemViewer* systemViewer = nullptr;

    vio::IOManager* systemIoManager = nullptr;
    PlanetLoader* planetLoader = nullptr;

    SoaOptions* options = nullptr; // Lives in App

    BlockPack blocks;
    BlockMaterialLoader blockTextureLoader;
    BlockTexturePack* blockTextures = nullptr;

    // TODO(Ben): This is temporary?
    CinematicCamera spaceCamera; ///< The camera that looks at the planet from space
    CinematicCamera localCamera; ///< Camera for voxels and far terrain

    vio::IOManager saveFileIom;
    ModPathResolver texturePathResolver;
    bool isNewGame = true;
    f64v3 startSpacePos = f64v3(0.0f);
    int startFace = 0;
    f64 time = 0.0;
    bool isInputEnabled = true;
    float timeStep = 0.016f;
private:
    VORB_NON_COPYABLE(SoaState);
};

#endif // SoAState_h__
