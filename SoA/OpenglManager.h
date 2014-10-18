#pragma once
#include <thread>

#include "Constants.h"
#include "Camera.h"
#include "DebugRenderer.h"
#include "readerwriterqueue.h"

// TODO: Remove This
using namespace std;

struct CameraInfo
{
    CameraInfo(const glm::dvec3 &pos, const glm::vec3 &wup, const glm::vec3 &wdir, const glm::mat4 &view, const glm::mat4 &proj) : position(pos), worldUp(wup), worldDir(wdir), viewMatrix(view), projectionMatrix(proj){}
    glm::dvec3 position;
    glm::vec3 worldUp, worldDir;
    glm::mat4 viewMatrix, projectionMatrix;
};

enum GL_MESSAGE { GL_M_ERROR, GL_M_DONE, GL_M_QUIT, GL_M_TERRAINMESH, GL_M_REMOVETREES, GL_M_UPDATECAMERA, GL_M_UPDATEPLANET, GL_M_STATETRANSITION, GL_M_NEWPLANET, GL_M_DELETEALLMESHES, 
    GL_M_INITIALIZEVOXELS, GL_M_CHUNKMESH, GL_M_PARTICLEMESH, GL_M_PHYSICSBLOCKMESH, GL_M_PLACEBLOCKS, GL_M_REBUILD_TERRAIN, GL_M_ENABLE_CHUNKS, GL_M_DISABLE_CHUNKS, GL_M_ENDSESSION };

extern CinematicCamera mainMenuCamera;
extern class GameMenu *currMenu;

struct PlanetUpdateMessage
{
    PlanetUpdateMessage(glm::mat4 &rot) : rotationMatrix(rot){}
    glm::mat4 rotationMatrix;
};

struct OMessage
{
    OMessage() : code(0), data(NULL){}
    OMessage(int i, void *d) : code(i), data(d) {}
    int code;
    void *data;
};

extern moodycamel::ReaderWriterQueue <OMessage> gameToGl;
extern moodycamel::ReaderWriterQueue <OMessage> glToGame;

class OpenglManager
{
public:
    OpenglManager();
    ~OpenglManager();
    void glThreadLoop();

    void initResolutions();

    void BeginThread(void (*func)(void));
    void EndThread();
    void endSession();
    OMessage WaitForMessage(int i);
    void glWaitForMessage(int i);
    void ProcessMessages(int waitForMessage = -1);
    void InitializeFrameBuffer();
    void BindFrameBuffer();
    void FreeFrameBuffer();
    void DrawFrameBuffer();
    void UpdateTerrainMesh(struct TerrainMeshMessage *tmm);
    void UpdateChunkMesh(struct ChunkMeshData *cmd);
    void UpdateParticleMesh(struct ParticleMeshMessage *pmm);
    void UpdatePhysicsBlockMesh(struct PhysicsBlockMeshMessage *pbmm);
    void UpdateMeshDistances();
    void Draw(Camera &chunkCamera, Camera &worldCamera);
    void DrawSonar(glm::mat4 &VP, glm::dvec3 &position);
    void drawBlocks(const glm::mat4 &VP, const glm::dvec3 &position, glm::vec3 &lightPos, glm::vec3 &lightColor, GLfloat lightActive, GLfloat sunVal, GLfloat fogEnd, GLfloat fogStart, GLfloat *fogColor, const GLfloat *eyeDir);
    void drawCutoutBlocks(const glm::mat4 &VP, const glm::dvec3 &position, glm::vec3 &lightPos, glm::vec3 &lightColor, GLfloat lightActive, GLfloat sunVal, GLfloat fogEnd, GLfloat fogStart, GLfloat *fogColor, const GLfloat *eyeDir);
    void drawTransparentBlocks(const glm::mat4 &VP, const glm::dvec3 &position, glm::vec3 &lightPos, glm::vec3 &lightColor, GLfloat lightActive, GLfloat sunVal, GLfloat fogEnd, GLfloat fogStart, GLfloat *fogColor, const GLfloat *eyeDir);
    void DrawPhysicsBlocks(glm::mat4 &VP, const glm::dvec3 &position, glm::vec3 &lightPos, glm::vec3 &lightColor, GLfloat lightActive, GLfloat sunVal, GLfloat fogEnd, GLfloat fogStart, GLfloat *fogColor, const GLfloat *eyeDir);
    void DrawWater(glm::mat4 &VP, const glm::dvec3 &position, GLfloat sunVal, GLfloat fogEnd, GLfloat fogStart, GLfloat *fogColor, glm::vec3 &lightPos, glm::vec3 &lightColor, bool underWater);
    void DrawHud();

    class FrameBuffer *frameBuffer;

    CameraInfo *cameraInfo;
    int zoomState;

    std::mutex collisionLock;

    DebugRenderer* debugRenderer;

private:
    std::thread *gameThread;
    vector <struct ChunkMesh *> chunkMeshes;
    vector <struct ParticleMesh *> particleMeshes;
    vector <struct PhysicsBlockMesh *> physicsBlockMeshes;
};

extern OpenglManager openglManager;