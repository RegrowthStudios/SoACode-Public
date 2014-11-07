#pragma once
#include <deque>

#include "WorldStructs.h"
#include "GLProgramManager.h"
#include "TextureCache.h"
#include "Vorb.h"

class ChunkSlot;
class Player;
class VoxelEditor;
class Chunk;

enum class GameStates { PLAY, PAUSE, INVENTORY, MAINMENU, ZOOMINGIN, ZOOMINGOUT, WORLDEDITOR, EXIT };

//This is where the main game components are contained
// TODO(Ben): Dependency injection.
class GameManager
{
public:

    static Player *player;

    static void initializeSystems();
    static void registerTexturesForLoad();
    static void getTextureHandles();

    static void initializeSound();
    static void saveState();
    static void savePlayerState();
    static int newGame(string saveName);
    static int loadGame(string saveName);
    static void loadPlanet(string filePath);
    static void initializePlanet(const glm::dvec3 cameraPos);
    static void initializeVoxelWorld(Player *plyr = nullptr);
    static void update();
    static void updatePlanet(glm::dvec3 worldPosition, GLuint maxTicks);
    static void drawMarkers();
    static void drawSpace(glm::mat4 &VP, bool connectedToPlanet);
    static void drawPlanet(glm::dvec3 worldPos, glm::mat4 &VP, const glm::mat4 &V, float ambVal, glm::vec3 lightPos, float fadeDist, bool connectedToPlanet);
    static void addMarker(glm::dvec3 pos, string name, glm::vec3 color);
    static void clickDragRay(bool isBreakRay);
    static void scanWSO();
    static void onQuit();
    static void endSession();

    static class VoxelWorld* voxelWorld;
    static class VoxelEditor* voxelEditor;
    static class PhysicsEngine* physicsEngine;
    static class CAEngine* caEngine;
    static class SoundEngine* soundEngine;
    static class ChunkManager* chunkManager;
    static class InputManager* inputManager;
    static class ChunkIOManager* chunkIOManager;
    static class MessageManager* messageManager;
    static class TerrainGenerator* terrainGenerator;
    static class EventManager* eventManager;

    static bool gameInitialized;
    static float fogStart, fogEnd;
    static Uint32 maxLodTicks;
    static vector <Marker> markers;
    static class Planet* planet;
    static class WSOAtlas* wsoAtlas;
    static class WSOScanner* wsoScanner;
    static class DebugRenderer* debugRenderer;
    static vcore::GLProgramManager* glProgramManager;
    static class TexturePackLoader* texturePackLoader; ///< Handles the loading of texture packs
    static class vg::TextureCache* textureCache;

    static GameStates gameState;
    static nString saveFilePath;
private:
    static bool _systemsInitialized;

};

