cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

cmake_policy(SET CMP0054 NEW)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  if (APPLE)
    message("\nRun the build script:  build.sh\n")
  endif (APPLE)
  message(FATAL_ERROR "You don't want to configure in the source folder!")
endif()


#project("SoA")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

enable_testing()

option(TARGET_CXX_17 OFF)

if(TARGET_CXX_17)
    set(CMAKE_CXX_STANDARD 17)
else()
    set(CMAKE_CXX_STANDARD 14)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    option(USING_GDB "Are we using gdb to debug?" On)
    option(EXTRA_DEBUG "Should we add extra debug symbols?" On)
    # We could move OPTIMISE_ON_DEBUG outside and support MSVC no doubt.
    option(OPTIMISE_ON_DEBUG "Should we optimise a debug target?" On)

    set(warnings "-Wall -Wextra -Wno-unknown-pragmas -Wno-strict-aliasing")
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        # This warning only exists for GCC.
        set(warnings "${warnings} -Wno-class-memaccess")
    endif()
    ADD_DEFINITIONS(
#        -std=c++11
#        -std=c++0x
        # Other flags
        ${warnings}
    )
    
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    if (${USING_GDB})
        if (${EXTRA_DEBUG})
            set(CMAKE_CXX_FLAGS_DEBUG "-ggdb3")
        else()
            set(CMAKE_CXX_FLAGS_DEBUG "-ggdb")
        endif()
    else()
        if (${EXTRA_DEBUG})
            set(CMAKE_CXX_FLAGS_DEBUG "-g3")
        else()
            set(CMAKE_CXX_FLAGS_DEBUG "-g")
        endif()
    endif ()
    if (${OPTIMISE_ON_DEBUG})
        # This is supported by both Clang and GCC as long as we are up-to-date.
        # Should be fine going back as far as Ubuntu 17.10, and deffo fine on Arch.
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og")
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(warnings "/W4 /WX /EHsc")
endif()

IF(APPLE)
   INCLUDE_DIRECTORIES ( /System/Library/Frameworks )
   FIND_LIBRARY(COCOA_LIBRARY Cocoa)
   FIND_LIBRARY(GLUT_LIBRARY GLUT )
   FIND_LIBRARY(OpenGL_LIBRARY OpenGL )
   MARK_AS_ADVANCED (COCOA_LIBRARY
                     GLUT_LIBRARY
                     OpenGL_LIBRARY)
   SET(EXTRA_LIBS ${COCOA_LIBRARY} ${GLUT_LIBRARY} ${OpenGL_LIBRARY})
ENDIF (APPLE)

# Open GL
find_package(OpenGL)
if (OPENGL_FOUND)
   include_directories(SYSTEM ${OPENGL_INCLUDE_DIRS})
endif (OPENGL_FOUND)

## SDL
#find_package(SDL)
#if (SDL_FOUND)
#   include_directories(SYSTEM ${SDL_INCLUDE_DIRS})
#endif (SDL_FOUND)
#
## Glew
#find_package(GLEW)
#if (GLEW_FOUND)
#   include_directories(SYSTEM ${GLEW_INCLUDE_DIRS})
#endif (GLEW_FOUND)
#
## BOOST filesystem
#set(Boost_USE_STATIC_LIBS OFF)
#set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_RUNTIME OFF)
#set(BOOST_VERSION 1.55.0)
#set(BOOST_COMPONENTS filesystem)
#find_package(Boost ${BOOST_VERSION} COMPONENTS ${BOOST_COMPONENTS} REQUIRED)
#if (Boost_FOUND)
#   include_directories(SYSTEM ${BOOST_INCLUDE_DIRS})
#endif (Boost_FOUND)

hunter_add_package(glew)
find_package(glew CONFIG REQUIRED)
#hunter_add_package(Boost COMPONENTS filesystem system)
#find_package(Boost REQUIRED COMPONENTS filesystem system)
hunter_add_package(SDL2)
find_package(SDL2 CONFIG REQUIRED)
hunter_add_package(minizip)
find_package(minizip CONFIG REQUIRED)
hunter_add_package(CreateLaunchers)
find_package(CreateLaunchers CONFIG REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../deps/include)

#lodepng
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../deps/lodepng)

# readerwriterqueue
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../deps/readerwriterqueue)

#set(extra_sources
#    ${CMAKE_CURRENT_SOURCE_DIR}/../deps/lodepng/lodepng.cpp
#    )

#file(GLOB engine_files
#    "*.h"
#    "*.cpp"
#

set(SoA_headers
    AABBCollidableComponentUpdater.h
    AmbienceLibrary.h
    AmbiencePlayer.h
    AmbienceStream.h
    Animation.h
    App.h
    ARProcessor.h
    AtmosphereComponentRenderer.h
    atomicops.h
    AxisRotationComponentUpdater.h
    Biome.h
    BlendState.h
    BlockData.h
    BlockLoader.h
    BlockPack.h
    BlockTexture.h
    BlockTextureAtlas.h
    BlockTextureLoader.h
    BlockTextureMethods.h
    BlockTexturePack.h
    BloomRenderStage.h
    CAEngine.h
    Camera.h
    CellularAutomataTask.h
    Chunk.h
    ChunkAccessor.h
    ChunkAllocator.h
    ChunkGenerator.h
    ChunkGrid.h
    ChunkGridRenderStage.h
    ChunkHandle.h
    ChunkID.h
    ChunkIOManager.h
    ChunkMesh.h
    ChunkMesher.h
    ChunkMeshManager.h
    ChunkMeshTask.h
    ChunkQuery.h
    ChunkRenderer.h
    ChunkSphereComponentUpdater.h
    ChunkUpdater.h
    ClientState.h
    CloudsComponentRenderer.h
    Collision.h
    CollisionComponentUpdater.h
    ColoredFullQuadRenderer.h
    ColorFilterRenderStage.h
    CommonState.h
    Computer.h
    ConsoleFuncs.h
    ConsoleMain.h
    ConsoleTests.h
    Constants.h
    CutoutVoxelRenderStage.h
    DebugRenderer.h
    Density.h
    DevConsole.h
    DevConsoleView.h
    DevHudRenderStage.h
    DevScreen.h
    DLLAPI.h
    DLLLoader.h
    DualContouringMesher.h
    ECSTemplates.h
    Errors.h
    ExposureCalcRenderStage.h
    FarTerrainComponentRenderer.h
    FarTerrainComponentUpdater.h
    FarTerrainPatch.h
    Flora.h
    FloraGenerator.h
    FragFile.h
    FreeMoveComponentUpdater.h
    Frustum.h
    FrustumComponentUpdater.h
    GameManager.h
    GameplayLoadScreen.h
    GameplayRenderer.h
    GamePlayScreen.h
    GameRenderParams.h
    GameSystem.h
    GameSystemAssemblages.h
    GameSystemComponentBuilders.h
    GameSystemComponents.h
    GameSystemUpdater.h
    GasGiantComponentRenderer.h
    GenerateTask.h
    GeometrySorter.h
    HdrRenderStage.h
    HeadComponentUpdater.h
    ImageAssetLoader.h
    IniParser.h
    InitScreen.h
    InputMapper.h
    Inputs.h
    IRenderStage.h
    Item.h
    LenseFlareRenderer.h
    LiquidData.h
    LiquidVoxelRenderStage.h
    LoadBar.h
    LoadContext.h
    LoadMonitor.h
    LoadTaskBlockData.h
    LoadTaskGameManager.h
    LoadTaskStarSystem.h
    LoadTaskTextures.h
    MainMenuLoadScreen.h
    MainMenuRenderer.h
    MainMenuScreen.h
    MainMenuScriptedUI.h
    MainMenuSystemViewer.h
    MarchingCubesTable.h
    MaterialAtlas.h
    MaterialData.h
    MaterialStack.h
    MetaSection.h
    ModelMesher.h
    ModInformation.h
    ModPathResolver.h
    MTRenderState.h
    MTRenderStateManager.h
    MusicPlayer.h
    NightVisionRenderStage.h
    Noise.h
    Octree.h
    OpaqueVoxelRenderStage.h
    OptionsController.h
    OrbitComponentRenderer.h
    OrbitComponentUpdater.h
    ParkourComponentUpdater.h
    ParticleMesh.h
    PauseMenu.h
    PauseMenuRenderStage.h
    PDA.h
    PdaRenderStage.h
    PhysicsBlockRenderStage.h
    PhysicsComponentUpdater.h
#    Planet.h
    PlanetGenData.h
    PlanetGenerator.h
    PlanetGenLoader.h
    PlanetHeightData.h
#    PlanetRenderStage.h
    PlanetRingsComponentRenderer.h
    Positional.h
    ProceduralChunkGenerator.h
    ProgramGenDelegate.h
    qef.h
    readerwriterqueue.h
    RegionFileManager.h
    RenderUtils.h
    ShaderAssetLoader.h
    ShaderLoader.h
    SkyboxRenderer.h
    SkyboxRenderStage.h
    SoaController.h
    SoaEngine.h
    SoaFileSystem.h
    SoaOptions.h
    SoAState.h
    soaUtils.h
    SonarRenderStage.h
    SpaceSystem.h
    SpaceSystemAssemblages.h
    SpaceSystemComponentBuilders.h
    SpaceSystemComponents.h
    SpaceSystemComponentTables.h
    SpaceSystemLoader.h
    SpaceSystemLoadStructs.h
    SpaceSystemRenderStage.h
    SpaceSystemUpdater.h
    SphericalHeightmapGenerator.h
    SphericalTerrainComponentRenderer.h
    SphericalTerrainComponentUpdater.h
    SphericalVoxelComponentUpdater.h
    SsaoRenderStage.h
    StarComponentRenderer.h
    Startup.h
    stdafx.h
    svd.h
    SystemARRenderer.h
    SystemBodyLoader.h
    TerrainGenTextures.h
    TerrainPatch.h
    TerrainPatchConstants.h
    TerrainPatchMesh.h
    TerrainPatchMesher.h
    TerrainPatchMeshManager.h
    TerrainPatchMeshTask.h
    TestBiomeScreen.h
    TestBlockViewScreen.h
    TestConnectedTextureScreen.h
    TestConsoleScreen.h
    TestDeferredScreen.h
    TestDisplacementMappingScreen.h
    TestGasGiantScreen.h
    TestMappingScreen.h
    TestNewBlockAPIScreen.h
    TestNoiseScreen.h
    TestPlanetGenScreen.h
    TestScriptScreen.h
    TestStarScreen.h
    TestUIScreen.h
    TestVoxelModelScreen.h
    textureUtils.h
    Thread.h
    TransparentVoxelRenderStage.h
    Vertex.h
    VoxelBits.h
    VoxelCoordinateSpaces.h
    VoxelEditor.h
    VoxelLightEngine.h
    VoxelMatrix.h
    VoxelMesh.h
    VoxelMesher.h
    VoxelModel.h
    VoxelModelLoader.h
    VoxelModelMesh.h
    VoxelModelRenderer.h
    VoxelNodeSetter.h
    VoxelNodeSetterTask.h
    VoxelRay.h
    VoxelSpaceConversions.h
    VoxelSpaceUtils.h
    VoxelUpdateBufferer.h
    VoxelUtils.h
    VoxelVertices.h
    VoxPool.h
    VRayHelper.h
#    WorldIO.h
    WorldStructs.h
    WSO.h
    WSOAtlas.h
    WSOData.h
    WSOScanner.h
    ZipFile.h
)

set(SoA_inline
)

set(SoA_sources
    AABBCollidableComponentUpdater.cpp
    AmbienceLibrary.cpp
    AmbiencePlayer.cpp
    AmbienceStream.cpp
    App.cpp
    ARProcessor.cpp
    AtmosphereComponentRenderer.cpp
    AxisRotationComponentUpdater.cpp
    Biome.cpp
    BlockData.cpp
    BlockLoader.cpp
    BlockPack.cpp
    BlockTexture.cpp
    BlockTextureLoader.cpp
    BlockTextureMethods.cpp
    BlockTexturePack.cpp
    BloomRenderStage.cpp
    CAEngine.cpp
    Camera.cpp
    CellularAutomataTask.cpp
    Chunk.cpp
    ChunkAccessor.cpp
    ChunkAllocator.cpp
    ChunkGenerator.cpp
    ChunkGrid.cpp
    ChunkGridRenderStage.cpp
    ChunkIOManager.cpp
    ChunkMesh.cpp
    ChunkMesher.cpp
    ChunkMeshManager.cpp
    ChunkMeshTask.cpp
    ChunkQuery.cpp
    ChunkRenderer.cpp
    ChunkSphereComponentUpdater.cpp
    ChunkUpdater.cpp
#    CloseTerrainPatch.cpp
    CloudsComponentRenderer.cpp
    Collision.cpp
    CollisionComponentUpdater.cpp
    ColoredFullQuadRenderer.cpp
    ColorFilterRenderStage.cpp
    Computer.cpp
    ConsoleTests.cpp
    CutoutVoxelRenderStage.cpp
    DebugRenderer.cpp
    Density.cpp
    DevConsole.cpp
    DevConsoleView.cpp
    DevHudRenderStage.cpp
    DevScreen.cpp
    DualContouringMesher.cpp
    ECSTemplates.cpp
    Errors.cpp
    ExposureCalcRenderStage.cpp
    FarTerrainComponentRenderer.cpp
    FarTerrainComponentUpdater.cpp
    FarTerrainPatch.cpp
    Flora.cpp
    FloraGenerator.cpp
    FragFile.cpp
    FreeMoveComponentUpdater.cpp
    Frustum.cpp
    FrustumComponentUpdater.cpp
    GameManager.cpp
    GameplayLoadScreen.cpp
    GameplayRenderer.cpp
    GamePlayScreen.cpp
    GameRenderParams.cpp
    GameSystem.cpp
    GameSystemAssemblages.cpp
    GameSystemComponentBuilders.cpp
    GameSystemComponents.cpp
    GameSystemUpdater.cpp
    GasGiantComponentRenderer.cpp
    GenerateTask.cpp
    GeometrySorter.cpp
    HdrRenderStage.cpp
    HeadComponentUpdater.cpp
    ImageAssetLoader.cpp
    IniParser.cpp
    InitScreen.cpp
    InputMapper.cpp
    Inputs.cpp
    Item.cpp
    LenseFlareRenderer.cpp
    LiquidVoxelRenderStage.cpp
    LoadBar.cpp
    LoadMonitor.cpp
    main.cpp
    MainMenuLoadScreen.cpp
    MainMenuRenderer.cpp
    MainMenuScreen.cpp
    MainMenuScriptedUI.cpp
    MainMenuSystemViewer.cpp
    MetaSection.cpp
    ModelMesher.cpp
    ModPathResolver.cpp
    MTRenderStateManager.cpp
    MusicPlayer.cpp
    NightVisionRenderStage.cpp
    Noise.cpp
    Octree.cpp
    OpaqueVoxelRenderStage.cpp
    OptionsController.cpp
    OrbitComponentRenderer.cpp
    OrbitComponentUpdater.cpp
    ParkourComponentUpdater.cpp
    PauseMenu.cpp
    PauseMenuRenderStage.cpp
    PDA.cpp
    PdaRenderStage.cpp
    PhysicsBlockRenderStage.cpp
    PhysicsComponentUpdater.cpp
#    Planet.cpp
    PlanetGenData.cpp
    PlanetGenerator.cpp
    PlanetGenLoader.cpp
#    PlanetRenderStage.cpp
    PlanetRingsComponentRenderer.cpp
    ProceduralChunkGenerator.cpp
    qef.cpp
    RegionFileManager.cpp
    ShaderAssetLoader.cpp
    ShaderLoader.cpp
    SkyboxRenderer.cpp
    SkyboxRenderStage.cpp
    SoaController.cpp
    SoaEngine.cpp
    SoaFileSystem.cpp
    SoaOptions.cpp
    SoaState.cpp
    SonarRenderStage.cpp
    SpaceSystem.cpp
    SpaceSystemAssemblages.cpp
    SpaceSystemComponentBuilders.cpp
    SpaceSystemComponentTables.cpp
    SpaceSystemLoader.cpp
    SpaceSystemLoadStructs.cpp
    SpaceSystemRenderStage.cpp
    SpaceSystemUpdater.cpp
    SphericalHeightmapGenerator.cpp
    SphericalTerrainComponentRenderer.cpp
    SphericalTerrainComponentUpdater.cpp
    SphericalVoxelComponentUpdater.cpp
    SsaoRenderStage.cpp
    StarComponentRenderer.cpp
    Startup.cpp
    stdafx.cpp
    svd.cpp
    SystemARRenderer.cpp
    SystemBodyLoader.cpp
    TerrainGenTextures.cpp
    TerrainPatch.cpp
    TerrainPatchMesh.cpp
    TerrainPatchMesher.cpp
    TerrainPatchMeshManager.cpp
    TerrainPatchMeshTask.cpp
    TestBiomeScreen.cpp
    TestBlockViewScreen.cpp
    TestConnectedTextureScreen.cpp
    TestConsoleScreen.cpp
    TestDeferredScreen.cpp
    TestDisplacementMappingScreen.cpp
    TestGasGiantScreen.cpp
    TestMappingScreen.cpp
    TestNewBlockAPIScreen.cpp
    TestNoiseScreen.cpp
    TestPlanetGenScreen.cpp
    TestScriptScreen.cpp
    TestStarScreen.cpp
    TestUIScreen.cpp
    TestVoxelModelScreen.cpp
    TransparentVoxelRenderStage.cpp
    VoxelEditor.cpp
    VoxelLightEngine.cpp
    VoxelMatrix.cpp
    VoxelMesher.cpp
    VoxelModel.cpp
    VoxelModelLoader.cpp
    VoxelModelMesh.cpp
    VoxelModelRenderer.cpp
    VoxelNodeSetter.cpp
    VoxelNodeSetterTask.cpp
    VoxelRay.cpp
    VoxelSpaceConversions.cpp
    VoxelSpaceUtils.cpp
    VoxPool.cpp
    VRayHelper.cpp
#    WorldIO.cpp
    WorldStructs.cpp
    WSO.cpp
    WSOAtlas.cpp
    WSOScanner.cpp
    ZipFile.cpp
)

set(skip_these_for_now
)

#foreach(eng_file in ${skip_these_for_now})
#  list(REMOVE_ITEM engine_files ${CMAKE_CURRENT_SOURCE_DIR}/${eng_file})
#endforeach()

# doxygen
find_package(Doxygen)
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})

if(BUILD_DOCUMENTATION)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen is needed to build the documentation.")
    endif()

    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${doxyfile_in} ${doxyfile} @ONLY)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)

    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc)
endif()

add_executable(soa 
    ${SoA_headers}
    ${SoA_inline}
    ${SoA_sources}
#    ${extra_sources}
)

target_link_libraries(soa
#    ${OPENGL_INCLUDE_DIRS}
    OpenGL::GL
    SDL2::SDL2main
    SDL2::SDL2
    glew::glew
#    Boost::filesystem
#    Boost::system
    vorb
    minizip::minizip
#    ${SDL_INCLUDE_DIRS}
#    ${GLEW_INCLUDE_DIRS}
#    ${BOOST_INCLUDE_DIRS}
)

include(CreateLaunchers)

create_target_launcher(soa
    ARGS "-a"
	RUNTIME_LIBRARY_DIRS "${CMAKE_BINARY_DIR}"
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../game"
)
