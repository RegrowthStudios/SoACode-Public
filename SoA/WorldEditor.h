#pragma once
#include "AwesomiumUI.h"
#include "camera.h"
#include "FloraGenerator.h"
#include "VoxelWorld.h"

struct EditorTree
{
    EditorTree() : startChunk(NULL), startc(0), tt(NULL), dirty(0), hasSaved(0), needsToGrow(0){
        Refresh();
    }
    void Refresh()
    {
        dirty = 0;
        hasSaved = 0;
    }
    void grow();
    void unGrow();
    Chunk *startChunk;
    int startc;
    TreeData td;
    TreeType *tt;
    vector <TreeNode> wnodes;
    vector <TreeNode> lnodes;
    bool dirty;
    volatile int needsToGrow;
    bool hasSaved;
};

class WorldEditor
{
public:
    WorldEditor(void);
    ~WorldEditor(void);
    void initialize(Planet *planet);
    void initializeChunkList(int width, int height, int x, int y, int z);

    void update();
    void glUpdate();
    //void DrawSelectTexture();
    void changeState(int newState);
    int onQuit();

    void injectMouseMove(int x, int y);
    void injectMouseWheel(int yMov);
    //0 = left, 1 = middle, 2 = right
    void injectMouseDown(int mouseButton);
    void injectMouseUp(int mouseButton);
    void injectKeyboardEvent(const SDL_Event& event);

    //getters
    inline Camera &getChunkCamera() { return _chunkCamera; }

    volatile bool usingChunks;
private:
    void initializeEditorTrees();
    void initializeEditorBlocks();

    void changeClimateVariables();
    void changeTreeVariables();
    void changeBlockVariables();
    void changeTerrainVariables();
    void changeBiomeVariables();
    void initializeEditorTree(EditorTree *et);
    void changeEditorTree(bool resendData, TreeType *tt = NULL);
    void removeTree(EditorTree *et);

    void regenerateChunks(int loadType, bool clearDrawing);
    void enableChunks();
    void disableChunks();

    int saveAsEditorTree(EditorTree *et);
    int saveEditorTree(EditorTree *et, string fname);
    int loadEditorTree(EditorTree *et);
    int loadEditorBiome(Biome *currEditorBiome);

    void terrainEditorUpdate();
    void climateEditorUpdate();
    void mainEditorUpdate();
    void blockEditorUpdate();
    void treeEditorUpdate();
    void biomeEditorUpdate();
    void sendBlockList(int active);
    void sendNoise(int id);
    void sendNoiseList();
    void setEditorBlock(int id, int state);
    void setEditorBiome(Biome *biome);
    void sendIsBase();
    void sendBlockTextures();
    void refreshEditorBlocks(class Block &newBlock);
    void appInterfaceChangeState(int newState);
    void onClimateChange();
    void sendClimate();

    void queueAllChunksForLoad();

    int blockSaveChanges(int id);
    int onTreeEditorQuit();
    int onBlockEditorQuit();
    int onBiomeEditorQuit();
    int getSurfaceBlock(Chunk **ch, int &c, bool aboveSurface);


    EditorTree *_editorTree;
    AwesomiumUI _UI;
    VoxelWorld *_voxelWorld;
    Camera _chunkCamera;
    FaceData _worldFaceData;
    Planet *_planet;
};

