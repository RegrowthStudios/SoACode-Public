#include "stdafx.h"
#include "PlanetRenderStage.h"

#include "Chunk.h"
#include "ChunkManager.h"
#include "ChunkIOManager.h"
#include "DebugRenderer.h"
#include "FrameBuffer.h"
#include "GLProgramManager.h"
#include "GameManager.h"
#include "GameWindow.h"
#include "MeshManager.h"
#include "Options.h"
#include "Player.h"
#include "Planet.h"
#include "SkyboxRenderer.h"
#include "Texture2d.h"
#include "VoxelEditor.h"
#include "VoxelWorld.h"

PlanetRenderStage::PlanetRenderStage(Camera* camera) :
    IRenderStage(camera) {
    // Empty
}

void PlanetRenderStage::draw()
{
    //far znear for maximum Terrain Patch z buffer precision
    //this is currently incorrect
    double nearClip = MIN((csGridWidth / 2.0 - 3.0)*32.0*0.7, 75.0) - ((double)(GameManager::chunkIOManager->getLoadListSize()) / (double)(csGridWidth*csGridWidth*csGridWidth))*55.0;
    if (nearClip < 0.1) nearClip = 0.1;
    double a = 0.0;

    a = closestTerrainPatchDistance / (sqrt(1.0f + pow(tan(graphicsOptions.fov / 2.0), 2.0) * (pow((double)4.0/3.0/*_gameWindow->getAspectRatio()*/, 2.0) + 1.0))*2.0);
    if (a < 0) a = 0;

    double clip = MAX(nearClip / planetScale * 0.5, a);

    _camera->setClippingPlane(clip, MAX(300000000.0 / planetScale, closestTerrainPatchDistance + 10000000));
    _camera->updateProjection();

    f32m4 VP = _camera->projectionMatrix() * _camera->viewMatrix();

    GameManager::planet->draw(0, VP, _camera->viewMatrix(), f32v3(1.0f, 0.0f, 0.0f), _camera->position(), 0.1 /*_ambientLight + 0.1*/, nearClip / planetScale, true /*connectedToPlanet*/);

    if (true /*connectedToPlanet*/) {
        if (!drawMode) GameManager::planet->atmosphere.draw((float)0, VP, glm::vec3((GameManager::planet->invRotationMatrix) * glm::vec4(1.0f, 0.0f, 0.0f, 1.0)), _camera->position());
    } else {
        if (!drawMode) GameManager::planet->atmosphere.draw((float)0, VP, f32v3(1.0f, 0.0f, 0.0f), _camera->position());
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Chunk::vboIndicesID);
    GameManager::planet->drawTrees(VP, _camera->position(), 0.1f /*ambVal*/);
}