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

extern class GameMenu *currMenu;

struct PlanetUpdateMessage
{
    PlanetUpdateMessage(glm::mat4 &rot) : rotationMatrix(rot){}
    glm::mat4 rotationMatrix;
};

class OpenglManager
{
public:
    OpenglManager();
    ~OpenglManager();

    void initResolutions();

    void BeginThread(void (*func)(void));
    void EndThread();
    void endSession();

    void FreeFrameBuffer();
  
    void UpdateMeshDistances();

    class FrameBuffer *frameBuffer;

    CameraInfo *cameraInfo;
    int zoomState;

    std::mutex collisionLock;
private:
    std::thread *gameThread;
    vector <struct ChunkMesh *> chunkMeshes;
    vector <struct ParticleMesh *> particleMeshes;
    vector <struct PhysicsBlockMesh *> physicsBlockMeshes;
};

extern OpenglManager openglManager;