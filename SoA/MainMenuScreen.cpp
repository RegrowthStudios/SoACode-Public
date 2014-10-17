#include "stdafx.h"

#include <glm\gtc\matrix_transform.hpp>
#include <glm\glm.hpp>

#include "MainMenuScreen.h"
#include "ChunkManager.h"
#include "Errors.h"
#include "Player.h"
#include "Planet.h"
#include "InputManager.h"
#include "GameManager.h"
#include "OpenglManager.h"
#include "Sound.h"
#include "Options.h"
#include "MessageManager.h"
#include "VoxelEditor.h"
#include "Frustum.h"
#include "TerrainPatch.h"

#include "SpriteBatch.h"
#include "colors.h"
#include "DepthState.h"
#include "SamplerState.h"
#include "RasterizerState.h"
#define THREAD ThreadName::PHYSICS

CTOR_APP_SCREEN_DEF(MainMenuScreen, App) ,
_updateThread(nullptr),
_threadRunning(false){
    // Empty
}

i32 MainMenuScreen::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}
i32 MainMenuScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void MainMenuScreen::build() {
    // Empty
}

void MainMenuScreen::destroy(const GameTime& gameTime) {
    // Empty
}

void MainMenuScreen::onEntry(const GameTime& gameTime) {
    _updateThread = new thread(&MainMenuScreen::updateThreadFunc, this);
}

void MainMenuScreen::onExit(const GameTime& gameTime) {
    _threadRunning = false;
    _updateThread->join();
}

void MainMenuScreen::onEvent(const SDL_Event& e) {
    // Empty
}

void MainMenuScreen::update(const GameTime& gameTime) {
    mainMenuCamera.update();
    GameManager::inputManager->update();

    TerrainMeshMessage* tmm;
    Message message;
    while (GameManager::messageManager->tryDeque(ThreadName::RENDERING, message)) {
        switch (message.id) {
            case MessageID::TERRAIN_MESH:
                UpdateTerrainMesh(static_cast<TerrainMeshMessage*>(message.data));
                break;
            case MessageID::REMOVE_TREES:
                tmm = static_cast<TerrainMeshMessage*>(message.data);
                if (tmm->terrainBuffers->treeVboID != 0) glDeleteBuffers(1, &(tmm->terrainBuffers->treeVboID));
                tmm->terrainBuffers->treeVboID = 0;
                delete tmm;
                break;
        }
    }

    bdt += glSpeedFactor * 0.01;
}

void MainMenuScreen::draw(const GameTime& gameTime) {

    openglManager.BindFrameBuffer();
  
    mainMenuCamera.setClippingPlane(1000000.0f, 30000000.0f);
    mainMenuCamera.updateProjection();
    glm::mat4 VP = mainMenuCamera.projectionMatrix() * mainMenuCamera.viewMatrix();

    GameManager::drawSpace(VP, 0);
    
    double clip = closestTerrainPatchDistance / (sqrt(1.0f + pow(tan(graphicsOptions.fov / 2.0), 2.0) * (pow((double)graphicsOptions.screenWidth / graphicsOptions.screenHeight, 2.0) + 1.0))*2.0);
    if (clip < 100) clip = 100;

    mainMenuCamera.setClippingPlane(clip, MAX(300000000.0 / planetScale, closestTerrainPatchDistance + 10000000));
    mainMenuCamera.updateProjection();

    VP = mainMenuCamera.projectionMatrix() * mainMenuCamera.viewMatrix();

    glm::dmat4 fvm;
    fvm = glm::lookAt(
        glm::dvec3(0.0),           // Camera is here
        glm::dvec3(glm::dmat4(GameManager::planet->invRotationMatrix) * glm::dvec4(mainMenuCamera.direction(), 1.0)), // and looks here : at the same position, plus "direction"
        glm::dvec3(mainMenuCamera.up())                  // Head is up (set to 0,-1,0 to look upside-down)
        );

    ExtractFrustum(glm::dmat4(mainMenuCamera.projectionMatrix()), fvm, worldFrustum);
    GameManager::drawPlanet(mainMenuCamera.position(), VP, mainMenuCamera.viewMatrix(), 1.0, glm::vec3(1.0, 0.0, 0.0), 1000, 0);



    glDisable(GL_DEPTH_TEST);
    openglManager.DrawFrameBuffer();
    glEnable(GL_DEPTH_TEST);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MainMenuScreen::updateThreadFunc() {

    _threadRunning = true;

    Message message;

    MessageManager* messageManager = GameManager::messageManager;
    /*
    messageManager->waitForMessage(THREAD, MessageID::DONE, message);
    if (message.id == MessageID::QUIT) {
        std::terminate();
    }*/

    FpsLimiter fpsLimiter;
    fpsLimiter.init(maxPhysicsFps);

    while (_threadRunning) {

        fpsLimiter.begin();

        GameManager::soundEngine->SetMusicVolume(soundOptions.musicVolume / 100.0f);
        GameManager::soundEngine->SetEffectVolume(soundOptions.effectVolume / 100.0f);
        GameManager::soundEngine->update();

        while (messageManager->tryDeque(THREAD, message)) {
            // Process the message
            switch (message.id) {
                case MessageID::NEW_PLANET:
                    messageManager->enqueue(THREAD, Message(MessageID::NEW_PLANET, NULL));
                    messageManager->enqueue(THREAD, Message(MessageID::DONE, NULL));
                    messageManager->waitForMessage(THREAD, MessageID::DONE, message);
                    break;
            }
        }

        glm::dvec3 camPos;

        camPos = glm::dvec3((glm::dmat4(GameManager::planet->invRotationMatrix)) * glm::dvec4(mainMenuCamera.position(), 1.0));

        GameManager::planet->rotationUpdate();
        GameManager::updatePlanet(camPos, 10);
        
        physicsFps = fpsLimiter.end();
    }
}

void MainMenuScreen::UpdateTerrainMesh(TerrainMeshMessage *tmm)
{
    TerrainBuffers *tb = tmm->terrainBuffers;

    vcore::GLProgram* program = GameManager::glProgramManager->getProgram("GroundFromSpace");

    if (tmm->indexSize){
        if (tb->vaoID == 0) glGenVertexArrays(1, &(tb->vaoID));
        glBindVertexArray(tb->vaoID);

        if (tb->vboID == 0) glGenBuffers(1, &(tb->vboID));
        glBindBuffer(GL_ARRAY_BUFFER, tb->vboID); // Bind the buffer (vertex array data)
        glBufferData(GL_ARRAY_BUFFER, tmm->index * sizeof(TerrainVertex), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, tmm->index * sizeof(TerrainVertex), &(tmm->verts[0]));

        if (tb->vboIndexID == 0) glGenBuffers(1, &(tb->vboIndexID));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tb->vboIndexID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, tmm->indexSize * sizeof(GLushort), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, tmm->indexSize * sizeof(GLushort), &(tmm->indices[0]));

        //vertices
        glVertexAttribPointer(program->getAttribute("vertexPosition_modelspace"), 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), 0);
        //UVs
        glVertexAttribPointer(program->getAttribute("vertexUV"), 2, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), ((char *)NULL + (12)));
        //normals
        glVertexAttribPointer(program->getAttribute("vertexNormal_modelspace"), 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), ((char *)NULL + (20)));
        //colors
        glVertexAttribPointer(program->getAttribute("vertexColor"), 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(TerrainVertex), ((char *)NULL + (32)));
        //slope color
        glVertexAttribPointer(program->getAttribute("vertexSlopeColor"), 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(TerrainVertex), ((char *)NULL + (36)));
        //beach color
        //glVertexAttribPointer(program->getAttribute("vertexBeachColor"), 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(TerrainVertex), ((char *)NULL + (40)));
        //texureUnit, temperature, rainfall, specular
        glVertexAttribPointer(program->getAttribute("texTempRainSpec"), 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(TerrainVertex), ((char *)NULL + (44)));

        program->enableVertexAttribArrays();
        glBindVertexArray(0); // Disable our Vertex Buffer Object  

        if (tmm->treeIndexSize){
            int treeIndex = (tmm->treeIndexSize * 4) / 6;
            glGenBuffers(1, &(tb->treeVboID));
            glBindBuffer(GL_ARRAY_BUFFER, tb->treeVboID); // Bind the buffer (vertex array data)
            glBufferData(GL_ARRAY_BUFFER, treeIndex * sizeof(TreeVertex), NULL, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, treeIndex * sizeof(TreeVertex), tmm->treeVerts);
            delete[] tmm->treeVerts;
        } else{
            if (tb->treeVboID != 0) glDeleteBuffers(1, &(tb->treeVboID));
            tb->treeVboID = 0;
        }
        tb->boundingBox = tmm->boundingBox;
        tb->drawX = tmm->drawX;
        tb->drawY = tmm->drawY;
        tb->drawZ = tmm->drawZ;
        tb->worldX = tmm->worldX;
        tb->worldY = tmm->worldY;
        tb->worldZ = tmm->worldZ;
        tb->cullRadius = tmm->cullRadius;
        tb->indexSize = tmm->indexSize;
        tb->treeIndexSize = tmm->treeIndexSize;
        delete[] tmm->verts;
        delete[] tmm->indices;
        if (tb->vecIndex == -1){
            tb->vecIndex = GameManager::planet->drawList[tmm->face].size();
            GameManager::planet->drawList[tmm->face].push_back(tb);
        }
    } else{
        if (tb->vecIndex != -1){
            GameManager::planet->drawList[tmm->face][tb->vecIndex] = GameManager::planet->drawList[tmm->face].back();
            GameManager::planet->drawList[tmm->face][tb->vecIndex]->vecIndex = tb->vecIndex;
            GameManager::planet->drawList[tmm->face].pop_back();
        }
        if (tb->vaoID != 0) glDeleteVertexArrays(1, &(tb->vaoID));
        if (tb->vboID != 0) glDeleteBuffers(1, &(tb->vboID));
        if (tb->treeVboID != 0) glDeleteBuffers(1, &(tb->treeVboID));
        if (tb->vboIndexID != 0) glDeleteBuffers(1, &(tb->vboIndexID));
        delete tb; //possible race condition
    }
    delete tmm;
}