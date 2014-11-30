#include "stdafx.h"
#include "Planet.h"

#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "Camera.h"
#include "DepthState.h"
#include "FileSystem.h"
#include "GameManager.h"
#include "InputManager.h"
#include "Inputs.h"
#include "ObjectLoader.h"
#include "Options.h"
#include "RasterizerState.h"
#include "Rendering.h"
#include "TerrainGenerator.h"
#include "TerrainPatch.h"
#include "TexturePackLoader.h"


ObjectLoader objectLoader;

Planet::Planet()
{
    bindex = 0;
    stormNoiseFunction = NULL;
    sunnyCloudyNoiseFunction = NULL;
    cumulusNoiseFunction = NULL;
    rotationTheta = 0;
    rotationMatrix = glm::mat4(1.0);
    invRotationMatrix = glm::inverse(rotationMatrix);
    axialZTilt = 0.4;
    baseTemperature = 100;
    baseRainfall = 100;
    minHumidity = 0.0f;
    maxHumidity = 100.0f;
    minCelsius = -20.0f;
    maxCelsius = 60.0f;
    solarX = solarY = solarZ = 0;
    scaledRadius = 0;
    gravityConstant = 1.0;
    density = 1.0;
}

Planet::~Planet()
{
    destroy();
}

void Planet::clearMeshes()
{
    TerrainMesh *tb;
    for (int k = 0; k < 6; k++){
        for (size_t i = 0; i < drawList[k].size(); i++){
            tb = drawList[k][i];
            if (tb->vaoID != 0) glDeleteVertexArrays(1, &(tb->vaoID));
            if (tb->vboID != 0) glDeleteBuffers(1, &(tb->vboID));
            if (tb->treeVboID != 0) glDeleteBuffers(1, &(tb->treeVboID));
            if (tb->vboIndexID != 0) glDeleteBuffers(1, &(tb->vboIndexID));
            delete tb;
        }

        drawList[k].clear();
    }


    for (size_t i = 0; i < 6U; i++){
        for (size_t j = 0; j < faces[i].size(); j++){
            for (size_t k = 0; k < faces[i][j].size(); k++){
                faces[i][j][k]->NullifyBuffers();
            }
        }
    }
}

void Planet::initialize(string filePath)
{
    #define MAP_WIDTH 256
    #define DEFAULT_RADIUS 1000000
    if (filePath == "Worlds/(Empty Planet)/"){
        radius = DEFAULT_RADIUS;
        scaledRadius = (radius - radius%CHUNK_WIDTH) / planetScale;
        int width = scaledRadius / TerrainPatchWidth * 2;
        if (width % 2 == 0){ // must be odd
            width++;
        }
        scaledRadius = (width*TerrainPatchWidth) / 2;
        radius = (int)(scaledRadius*planetScale);
        ColorRGB8* colorMap = GameManager::texturePackLoader->getColorMap("biome");
        ColorRGB8* waterColorMap = GameManager::texturePackLoader->getColorMap("water");

        for (int i = 0; i < MAP_WIDTH * MAP_WIDTH; i++){
            colorMap[i] = ColorRGB8(0, 255, 0);
        }

        for (int i = 0; i < MAP_WIDTH * MAP_WIDTH; i++){
            colorMap[i] = ColorRGB8(0, 0, 255);
        }

        for (int i = 0; i < 256; i++){
            for (int j = 0; j < 256; j++){
                int hexcode = 0;
                GameManager::terrainGenerator->BiomeMap[i][j] = hexcode;
            }
        }

        atmosphere.initialize("", scaledRadius);
    }
    else{
        loadData(filePath, 0);


        atmosphere.initialize(filePath + "Sky/properties.ini", scaledRadius);
    }
    dirName = filePath;

    rotationTheta = 0;
    glm::vec3 EulerAngles(0, rotationTheta, axialZTilt);
    rotateQuaternion = glm::quat(EulerAngles);
    rotationMatrix = glm::toMat4(rotateQuaternion);

    double mrad = (double)radius / 2.0;
    volume = (4.0 / 3.0)*M_PI*mrad*mrad*mrad;
    mass = volume*density;

    facecsGridWidth = (radius * 2) / CHUNK_WIDTH;
}

void Planet::initializeTerrain(const glm::dvec3 &startPosition)
{
    int lodx, lody, lodz;

    int wx = (int)startPosition.x, wy = (int)startPosition.y, wz = (int)startPosition.z;
    int width = scaledRadius/TerrainPatchWidth*2;
    if (width%2 == 0){ // must be odd
        width++;
    }
    cout << "RADIUS " << radius << " Width " << width << endl;
    cout << "XYZ " << solarX << " " << solarY << " " << solarZ << endl;

    int centerX = 0;
    int centerY = 0 + scaledRadius;
    int centerZ = 0;
    double magnitude;

    faces[P_TOP].resize(width);
    for (size_t i = 0; i < faces[P_TOP].size(); i++){
        faces[P_TOP][i].resize(width);
        for (size_t j = 0; j < faces[P_TOP][i].size(); j++){
            faces[P_TOP][i][j] = new TerrainPatch(TerrainPatchWidth);

            lodx = 0 + (j - width/2) * TerrainPatchWidth - TerrainPatchWidth/2;
            lody = centerY;
            lodz = 0 + (i - width/2) * TerrainPatchWidth - TerrainPatchWidth/2;
            magnitude = sqrt(lodx*lodx + lody*lody + lodz*lodz);

            faces[P_TOP][i][j]->Initialize(lodx, lody, lodz, wx, wy, wz, radius, P_TOP);
            while(!(faces[P_TOP][i][j]->CreateMesh())); //load all of the quadtrees
        }
    }

    centerX = 0 - scaledRadius;
    centerY = 0;
    centerZ = 0;
    faces[P_LEFT].resize(width);
    for (size_t i = 0; i < faces[P_LEFT].size(); i++){
        faces[P_LEFT][i].resize(width);
        for (size_t j = 0; j < faces[P_LEFT][i].size(); j++){
            faces[P_LEFT][i][j] = new TerrainPatch(TerrainPatchWidth);

            lodx = centerX;
            lody = 0 + (i - width/2) * TerrainPatchWidth - TerrainPatchWidth/2;
            lodz = 0 + (j - width/2) * TerrainPatchWidth - TerrainPatchWidth/2;
            magnitude = sqrt(lodx*lodx + lody*lody + lodz*lodz);

            faces[P_LEFT][i][j]->Initialize(lodx, lody, lodz, wx, wy, wz, radius, P_LEFT);
            while(!(faces[P_LEFT][i][j]->CreateMesh())); //load all of the quadtrees
        }
    }

    centerX = 0 + scaledRadius;
    centerY = 0;
    centerZ = 0;
    faces[P_RIGHT].resize(width);
    for (size_t i = 0; i < faces[P_RIGHT].size(); i++){
        faces[P_RIGHT][i].resize(width);
        for (size_t j = 0; j < faces[P_RIGHT][i].size(); j++){
            faces[P_RIGHT][i][j] = new TerrainPatch(TerrainPatchWidth);

            lodx = centerX;
            lody = 0 + (i - width/2) * TerrainPatchWidth - TerrainPatchWidth/2;
            lodz = 0 + (j - width/2) * TerrainPatchWidth - TerrainPatchWidth/2;
            magnitude = sqrt(lodx*lodx + lody*lody + lodz*lodz);

            faces[P_RIGHT][i][j]->Initialize(lodx, lody, lodz, wx, wy, wz, radius, P_RIGHT);

            while(!(faces[P_RIGHT][i][j]->CreateMesh())); //load all of the quadtrees
        }
    }

    centerX = 0;
    centerY = 0;
    centerZ = 0 + scaledRadius;
    faces[P_FRONT].resize(width);
    for (size_t i = 0; i < faces[P_FRONT].size(); i++){
        faces[P_FRONT][i].resize(width);
        for (size_t j = 0; j < faces[P_FRONT][i].size(); j++){
            faces[P_FRONT][i][j] = new TerrainPatch(TerrainPatchWidth);

            lodx = 0 + (j - width/2) * TerrainPatchWidth - TerrainPatchWidth/2;
            lody = 0 + (i - width/2) * TerrainPatchWidth - TerrainPatchWidth/2;
            lodz = centerZ;
            magnitude = sqrt(lodx*lodx + lody*lody + lodz*lodz);


            //TopFace[i][j]->Initialize(((double)lodx/magnitude)*radius, ((double)lody/magnitude)*radius, ((double)lodz/magnitude)*radius, centerX, centerY, centerZ);
            faces[P_FRONT][i][j]->Initialize(lodx, lody, lodz, wx, wy, wz, radius, P_FRONT);

            while(!(faces[P_FRONT][i][j]->CreateMesh())); //load all of the quadtrees
        }
    }

    centerX = 0;
    centerY = 0;
    centerZ = 0 - scaledRadius;
    faces[P_BACK].resize(width);
    for (size_t i = 0; i < faces[P_BACK].size(); i++){
        faces[P_BACK][i].resize(width);
        for (size_t j = 0; j < faces[P_BACK][i].size(); j++){
            faces[P_BACK][i][j] = new TerrainPatch(TerrainPatchWidth);

            lodx = 0 + (j - width/2) * TerrainPatchWidth - TerrainPatchWidth/2;
            lody = 0 + (i - width/2) * TerrainPatchWidth - TerrainPatchWidth/2;
            lodz = centerZ;
            magnitude = sqrt(lodx*lodx + lody*lody + lodz*lodz);

            faces[P_BACK][i][j]->Initialize(lodx, lody, lodz, wx, wy, wz, radius, P_BACK);

            while(!(faces[P_BACK][i][j]->CreateMesh())); //load all of the quadtrees
        }
    }

    centerX = 0;
    centerY = 0 - scaledRadius;
    centerZ = 0;

    faces[P_BOTTOM].resize(width);
    for (size_t i = 0; i < faces[P_BOTTOM].size(); i++){
        printf("%d ", i);
        faces[P_BOTTOM][i].resize(width);
        for (size_t j = 0; j < faces[P_BOTTOM][i].size(); j++){
            faces[P_BOTTOM][i][j] = new TerrainPatch(TerrainPatchWidth);

            lodx = 0 + (j - width/2) * TerrainPatchWidth - TerrainPatchWidth/2;
            lody = centerY;
            lodz = 0 + (i - width/2) * TerrainPatchWidth - TerrainPatchWidth/2;
            magnitude = sqrt(lodx*lodx + lody*lody + lodz*lodz);

            faces[P_BOTTOM][i][j]->Initialize(lodx, lody, lodz, wx, wy, wz, radius, P_BOTTOM);

            while(!(faces[P_BOTTOM][i][j]->CreateMesh())); //load all of the quadtrees
        }
    }
}

void Planet::loadData(string filePath, bool ignoreBiomes)
{    

    loadProperties(filePath + "properties.ini");
    saveProperties(filePath + "properties.ini"); //save em to update them


    TerrainGenerator* generator = GameManager::terrainGenerator;

    GameManager::planet = this;
    vg::TextureCache* textureCache = GameManager::textureCache;
   
    sunColorMapTexture = textureCache->addTexture(filePath + "/Sky/sunColor.png", &SamplerState::LINEAR_CLAMP_MIPMAP);

#define MAP_WIDTH 256

    GLubyte buffer[MAP_WIDTH * MAP_WIDTH][3];
    if (!ignoreBiomes){
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, GameManager::planet->biomeMapTexture.ID);
        
        glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_BYTE, buffer);

        //convert rgb values into a hex int
        for (int i = 0; i < MAP_WIDTH; i++){
            for (int j = 0; j < MAP_WIDTH; j++){
                int hexcode = 0;
                hexcode |= (((int)buffer[i * MAP_WIDTH + j][2]) << 16); //converts bgr to rgb
                hexcode |= (((int)buffer[i * MAP_WIDTH + j][1]) << 8);
                hexcode |= ((int)buffer[i * MAP_WIDTH + j][0]);
                generator->BiomeMap[i][j] = hexcode;
            }
        }
    }

    ColorRGB8* biomeMap = GameManager::texturePackLoader->getColorMap("biome");
    ColorRGB8* waterMap = GameManager::texturePackLoader->getColorMap("water");

    //color map!
    glBindTexture(GL_TEXTURE_2D, GameManager::planet->colorMapTexture.ID);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_BYTE, buffer);
    for (int y = 0; y < MAP_WIDTH; y++){
        for (int x = 0; x < MAP_WIDTH; x++) {
            biomeMap[(MAP_WIDTH - y - 1) * MAP_WIDTH + x] = ColorRGB8(buffer[y * MAP_WIDTH + x][2], //convert bgr to rgb
                                                                      buffer[y * MAP_WIDTH + x][1],
                                                                      buffer[y * MAP_WIDTH + x][0]);
        }
    }

    glBindTexture(GL_TEXTURE_2D, GameManager::planet->waterColorMapTexture.ID);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_BYTE, buffer);
    for (int y = 0; y < MAP_WIDTH; y++){
        for (int x = 0; x < MAP_WIDTH; x++) {
            waterMap[(MAP_WIDTH - y - 1) * MAP_WIDTH + x] = ColorRGB8(buffer[y * MAP_WIDTH + x][2], //convert bgr to rgb
                                                                      buffer[y * MAP_WIDTH + x][1],
                                                                      buffer[y * MAP_WIDTH + x][0]);
        }
    }
    
}

void Planet::saveData()
{
    fileManager.saveBiomeData(this, "World/");
    fileManager.saveAllBiomes(this);
}

void Planet::rotationUpdate()
{
    float rotationInput = GameManager::inputManager->getAxis(INPUT_PLANET_ROTATION);
    if (std::abs(rotationInput) > 0) {
        rotationTheta += rotationInput * 0.01 * physSpeedFactor;
    }else{
        rotationTheta += 0.00002*physSpeedFactor;//0.00001
    }
    if (rotationTheta > 3.14){
        rotationTheta = -3.14;
    }else if (rotationTheta < -3.14){
        rotationTheta = 3.14;
    }

    glm::vec3 EulerAngles(0, rotationTheta, axialZTilt);
    rotateQuaternion = glm::quat(EulerAngles);
    //gameToGl.enqueue(OMessage(GL_M_UPDATEPLANET, (void *)(new PlanetUpdateMessage(glm::toMat4(rotateQuaternion)))));
    rotationMatrix = glm::toMat4(rotateQuaternion);
    invRotationMatrix = glm::inverse(rotationMatrix);
}

void Planet::draw(float theta, const Camera* camera, glm::vec3 lightPos, GLfloat sunVal, float fadeDistance, bool connectedToPlanet)
{    
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, terrainTexture.ID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, waterNormalTexture.ID);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, biomeMapTexture.ID);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, waterColorMapTexture.ID);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, waterNoiseTexture.ID);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, sunColorMapTexture.ID);

    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, colorMapTexture.ID);

    closestTerrainPatchDistance = 999999999999.0;

    const f64v3& playerPos = camera->getPosition();

    glm::dvec3 rotPlayerPos;
    glm::dvec3 nPlayerPos;
    glm::vec3 rotLightPos;
    rotLightPos = glm::vec3((GameManager::planet->invRotationMatrix) * glm::vec4(lightPos, 1.0));
    bool onPlanet;
    if (connectedToPlanet){
        rotPlayerPos = playerPos;
        nPlayerPos = playerPos;
        onPlanet = 1;
    }else{
        rotPlayerPos = glm::dvec3((glm::dmat4(GameManager::planet->invRotationMatrix)) * glm::dvec4(playerPos, 1.0));
        nPlayerPos = playerPos;
        onPlanet = 0;
    }

    f32m4 VP = camera->getProjectionMatrix() * camera->getViewMatrix();

    if (glm::length(playerPos) > atmosphere.radius) {
        drawGroundFromSpace(theta, camera, VP, rotLightPos, nPlayerPos, rotPlayerPos, onPlanet);
    }else{
        drawGroundFromAtmosphere(theta, camera, VP, rotLightPos, nPlayerPos, rotPlayerPos, fadeDistance, onPlanet);
    }
}

void Planet::drawTrees(const glm::mat4 &VP, const glm::dvec3 &PlayerPos, GLfloat sunVal)
{
    glm::vec3 worldUp = glm::vec3(glm::normalize(PlayerPos));
//    glDisable(GL_CULL_FACE);
    vg::GLProgram* program = GameManager::glProgramManager->getProgram("TreeBillboard");
    program->use();
    program->enableVertexAttribArrays();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, treeTrunkTexture1.ID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalLeavesTexture.ID);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, pineLeavesTexture.ID);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, mushroomCapTexture.ID);

    glUniform3f(program->getUniform("worldUp"), worldUp.x, worldUp.y, worldUp.z);
    glUniform1f(program->getUniform("FadeDistance"), (GLfloat)((csGridWidth / 2) * CHUNK_WIDTH)*invPlanetScale);
    glUniform1f(program->getUniform("sunVal"), sunVal);

    for (size_t f = 0; f < 6; f++){
        for (size_t i = 0; i < drawList[f].size(); i++){
            if (drawList[f][i]->treeIndexSize) TerrainPatch::DrawTrees(drawList[f][i], program, PlayerPos, VP);
        }
    }

    program->disableVertexAttribArrays();
    program->unuse();

}

void Planet::drawGroundFromAtmosphere(float theta, const Camera* camera, const glm::mat4 &VP, glm::vec3 lightPos, const glm::dvec3 &PlayerPos, const glm::dvec3 &rotPlayerPos, float fadeDistance, bool onPlanet)
{
    vg::GLProgram* shader = GameManager::glProgramManager->getProgram("GroundFromAtmosphere");
    shader->use();

    const int txv[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    // TODO(Cristian): Investigate why getUniform doesnt work
    glUniform1iv(glGetUniformLocation(shader->getID(), "textures"), 6, txv);
    glUniform1i(glGetUniformLocation(shader->getID(), "sunColorTexture"), txv[6]);
    glUniform1i(glGetUniformLocation(shader->getID(), "colorTexture"), txv[7]);
    glUniform1i(glGetUniformLocation(shader->getID(), "waterColorTexture"), txv[3]);
    /*glUniform1iv(shader->getUniform("textures"), 6, txv);
    glUniform1i(shader->getUniform("sunColorTexture"), txv[6]);
    glUniform1i(shader->getUniform("colorTexture"), txv[7]);
    glUniform1i(shader->getUniform("waterColorTexture"), txv[3]);*/

    float m_Kr4PI = atmosphere.m_Kr*4.0f*M_PI;
    float m_Km4PI = atmosphere.m_Km*4.0f*M_PI;
    float m_fScale = 1.0 / (atmosphere.radius - scaledRadius);

    glUniform3f(shader->getUniform("cameraPos"), (float)rotPlayerPos.x, (float)rotPlayerPos.y, (float)rotPlayerPos.z);

    glUniform3f(shader->getUniform("lightPos"), lightPos.x, lightPos.y, lightPos.z);

    glUniform3f(shader->getUniform("invWavelength"), 1 / atmosphere.m_fWavelength4[0], 1 / atmosphere.m_fWavelength4[1], 1 / atmosphere.m_fWavelength4[2]);
    float height = glm::length(rotPlayerPos);
    if (height < scaledRadius+1) height = scaledRadius+1;
    glUniform1f(shader->getUniform("cameraHeight"), height);

    glUniform1f(shader->getUniform("dt"), bdt);

    glUniform1f(shader->getUniform("specularExponent"), graphicsOptions.specularExponent);
    glUniform1f(shader->getUniform("specularIntensity"), graphicsOptions.specularIntensity);

    glUniform1f(shader->getUniform("freezeTemp"), FREEZETEMP / 255.0f);
    
    glUniform1f(shader->getUniform("innerRadius"), scaledRadius);
    glUniform1f(shader->getUniform("krESun"), atmosphere.m_Kr*atmosphere.m_ESun);
    glUniform1f(shader->getUniform("kmESun"), atmosphere.m_Km*atmosphere.m_ESun);
    glUniform1f(shader->getUniform("kr4PI"), m_Kr4PI);
    glUniform1f(shader->getUniform("km4PI"), m_Km4PI);
    glUniform1f(shader->getUniform("fScale"), m_fScale);
    glUniform1f(shader->getUniform("scaleDepth"), atmosphere.m_fRayleighScaleDepth);
    glUniform1f(shader->getUniform("fScaleOverScaleDepth"), m_fScale / atmosphere.m_fRayleighScaleDepth);
    glUniform1f(shader->getUniform("fSamples"), atmosphere.fSamples);
    glUniform1i(shader->getUniform("nSamples"), atmosphere.nSamples);

    glUniform1f(shader->getUniform("secColorMult"), graphicsOptions.secColorMult);

    glUniform1f(shader->getUniform("FadeDistance"), fadeDistance);

    const ui32& mvpID = shader->getUniform("MVP");
    const ui32& worldOffsetID = shader->getUniform("worldOffset");

    shader->enableVertexAttribArrays();

    for (size_t i = 0; i < drawList[P_TOP].size(); i++){
        TerrainPatch::Draw(drawList[P_TOP][i], camera, PlayerPos, rotPlayerPos, VP, mvpID, worldOffsetID, onPlanet);
    }
    for (size_t i = 0; i < drawList[P_RIGHT].size(); i++){
        TerrainPatch::Draw(drawList[P_RIGHT][i], camera, PlayerPos, rotPlayerPos, VP, mvpID, worldOffsetID, onPlanet);
    }
    for (size_t i = 0; i < drawList[P_BACK].size(); i++){
        TerrainPatch::Draw(drawList[P_BACK][i], camera, PlayerPos, rotPlayerPos, VP, mvpID, worldOffsetID, onPlanet);
    }
    glFrontFace(GL_CW);
    for (size_t i = 0; i < drawList[P_BOTTOM].size(); i++){
        TerrainPatch::Draw(drawList[P_BOTTOM][i], camera, PlayerPos, rotPlayerPos, VP, mvpID, worldOffsetID, onPlanet);
    }
    for (size_t i = 0; i < drawList[P_LEFT].size(); i++){
        TerrainPatch::Draw(drawList[P_LEFT][i], camera, PlayerPos, rotPlayerPos, VP, mvpID, worldOffsetID, onPlanet);
    }
    for (size_t i = 0; i < drawList[P_FRONT].size(); i++){
        TerrainPatch::Draw(drawList[P_FRONT][i], camera, PlayerPos, rotPlayerPos, VP, mvpID, worldOffsetID, onPlanet);
    }
    glFrontFace(GL_CCW);

    shader->disableVertexAttribArrays();
    shader->unuse();
}

void Planet::drawGroundFromSpace(float theta, const Camera* camera, const glm::mat4 &VP, glm::vec3 lightPos, const glm::dvec3 &PlayerPos, const glm::dvec3 &rotPlayerPos, bool onPlanet)
{
    vg::GLProgram* shader = GameManager::glProgramManager->getProgram("GroundFromSpace");
    shader->use();

    const int textureUnits[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

    // TODO(Ben or Cristian): Figure out why getUniform doesnt work for the samplers here
    glUniform1iv(glGetUniformLocation(shader->getID(), "textures"), 6, textureUnits);
    glUniform1i(glGetUniformLocation(shader->getID(), "sunColorTexture"), textureUnits[6]);
    glUniform1i(glGetUniformLocation(shader->getID(), "colorTexture"), textureUnits[7]);
    glUniform1i(glGetUniformLocation(shader->getID(), "waterColorTexture"), textureUnits[3]);
//    glUniform1i(shader->getUniform("sunColorTexture"), txv[6]);
//    glUniform1i(shader->getUniform("colorTexture"), txv[7]);
//    glUniform1i(shader->getUniform("waterColorTexture"), txv[3]);

    float m_Kr4PI = atmosphere.m_Kr*4.0f*M_PI;
    float m_Km4PI = atmosphere.m_Km*4.0f*M_PI;
    float m_fScale = 1 / (atmosphere.radius - scaledRadius);

    glUniform3f(shader->getUniform("cameraPos"), (float)rotPlayerPos.x, (float)rotPlayerPos.y, (float)rotPlayerPos.z);

    glUniform3f(shader->getUniform("lightPos"), lightPos.x, lightPos.y, lightPos.z);

    glUniform3f(shader->getUniform("invWavelength"), 1 / atmosphere.m_fWavelength4[0], 1 / atmosphere.m_fWavelength4[1], 1 / atmosphere.m_fWavelength4[2]);

    glUniform1f(shader->getUniform("specularExponent"), graphicsOptions.specularExponent);
    glUniform1f(shader->getUniform("specularIntensity"), graphicsOptions.specularIntensity);
    
    #define MAX_FREEZE_TEMP 255.0f
    glUniform1f(shader->getUniform("freezeTemp"), FREEZETEMP / MAX_FREEZE_TEMP);

    glUniform1f(shader->getUniform("cameraHeight2"), glm::length(PlayerPos)*glm::length(PlayerPos));
    glUniform1f(shader->getUniform("outerRadius"), atmosphere.radius);
    glUniform1f(shader->getUniform("outerRadius2"), atmosphere.radius*atmosphere.radius);
    glUniform1f(shader->getUniform("innerRadius"), scaledRadius);

    glUniform1f(shader->getUniform("krESun"), atmosphere.m_Kr*atmosphere.m_ESun);
    glUniform1f(shader->getUniform("kmESun"), atmosphere.m_Km*atmosphere.m_ESun);
    glUniform1f(shader->getUniform("kr4PI"), m_Kr4PI);
    glUniform1f(shader->getUniform("km4PI"), m_Km4PI);
    glUniform1f(shader->getUniform("fScale"), m_fScale);
    glUniform1f(shader->getUniform("scaleDepth"), atmosphere.m_fRayleighScaleDepth);
    glUniform1f(shader->getUniform("fScaleOverScaleDepth"), m_fScale / atmosphere.m_fRayleighScaleDepth);
    glUniform1f(shader->getUniform("fSamples"), atmosphere.fSamples);
    glUniform1f(shader->getUniform("drawMode"), planetDrawMode);
    glUniform1i(shader->getUniform("nSamples"), atmosphere.nSamples);

    glUniform1f(shader->getUniform("dt"), bdt);

    glUniform1f(shader->getUniform("secColorMult"), graphicsOptions.secColorMult);

    shader->enableVertexAttribArrays();

    const ui32& mvpID = shader->getUniform("MVP");
    const ui32& worldOffsetID = shader->getUniform("worldOffset");

    for (size_t i = 0; i < drawList[0].size(); i++){
        TerrainPatch::Draw(drawList[0][i], camera, PlayerPos, rotPlayerPos, VP, mvpID, worldOffsetID, onPlanet);
    }
    for (size_t i = 0; i < drawList[2].size(); i++){
        TerrainPatch::Draw(drawList[2][i], camera, PlayerPos, rotPlayerPos, VP, mvpID, worldOffsetID, onPlanet);
    }
    for (size_t i = 0; i < drawList[4].size(); i++){
        TerrainPatch::Draw(drawList[4][i], camera, PlayerPos, rotPlayerPos, VP, mvpID, worldOffsetID, onPlanet);
    }
    glFrontFace(GL_CW);
    for (size_t i = 0; i < drawList[5].size(); i++){
        TerrainPatch::Draw(drawList[5][i], camera, PlayerPos, rotPlayerPos, VP, mvpID, worldOffsetID, onPlanet);
    }
    for (size_t i = 0; i < drawList[1].size(); i++){
        TerrainPatch::Draw(drawList[1][i], camera, PlayerPos, rotPlayerPos, VP, mvpID, worldOffsetID, onPlanet);
    }
    for (size_t i = 0; i < drawList[3].size(); i++){
        TerrainPatch::Draw(drawList[3][i], camera, PlayerPos, rotPlayerPos, VP, mvpID, worldOffsetID, onPlanet);
    }
    glFrontFace(GL_CCW);

    shader->disableVertexAttribArrays();

    shader->unuse();
}

void Planet::updateLODs(glm::dvec3 &worldPosition, GLuint maxTicks)
{
    static int k = 0;

    glm::dvec3 rWorldPosition = worldPosition;
    //glm::dvec3 rWorldPosition = glm::dvec3((glm::dmat4(GameManager::planet->invRotationMatrix)) * glm::dvec4(worldPosition, 1.0));

    int mx = (int)rWorldPosition.x, my = (int)rWorldPosition.y, mz = (int)rWorldPosition.z;
    for (int n = 0; n < 6; n++){
        for (unsigned int a = 0; a < faces[n].size(); a++){
            for (unsigned int b = 0; b < faces[n][a].size(); b++){
                if (faces[n][a][b]->update(mx, my, mz)){
                    if (faces[n][a][b]->updateVecIndex == -1){
                        faces[n][a][b]->updateVecIndex = LODUpdateList.size();
                        LODUpdateList.push_back(faces[n][a][b]);
                    }
                }
            }
        }
    }

    GLuint startTicks = SDL_GetTicks();

    if (LODUpdateList.empty()) return;
    //cout << LODUpdateList.size() << " ";
//    GLuint starttime = SDL_GetTicks();
    TerrainPatch *lod;

    int wx = (0+CHUNK_WIDTH*csGridWidth/2);
    int wz = (0+CHUNK_WIDTH*csGridWidth/2);
    int size = LODUpdateList.size()-1;

    sortUpdateList(); //sorts every update, but insertion sort is fast

    for (int i = size; LODUpdateList.size(); i--)
    {
        lod = LODUpdateList.back();

        if (lod->CreateMesh()){
            LODUpdateList.pop_back();
            lod->updateVecIndex = -1;
        }
        if (SDL_GetTicks() - startTicks > maxTicks){ return; }
    }
    
    k++;
//    cout << SDL_GetTicks() - starttime << "     L\n";
}

void RecursiveFlagLOD(TerrainPatch *lod){
    lod->updateCode = 1;
    if (lod->lods[0]){
        for(int i = 0; i < 4; i++){
            RecursiveFlagLOD(lod->lods[i]);
        }
    }
}

void Planet::flagTerrainForRebuild()
{
    for (int n = 0; n < 6; n++){
        for (unsigned int a = 0; a < faces[n].size(); a++){
            for (unsigned int b = 0; b < faces[n][a].size(); b++){
                RecursiveFlagLOD(faces[n][a][b]);
                if (faces[n][a][b]->updateVecIndex == -1){
                    faces[n][a][b]->updateVecIndex = LODUpdateList.size();
                    LODUpdateList.push_back(faces[n][a][b]);
                }
                
            }
        }
    }
}

void Planet::sortUpdateList()
{
    TerrainPatch *temp;
    int j;
    for (unsigned int i = 1; i < LODUpdateList.size(); i++)
    {
        temp = LODUpdateList[i];

        for (j = i - 1; (j >= 0) && (temp->distance > LODUpdateList[j]->distance); j-- ){
            LODUpdateList[j+1] = LODUpdateList[j];
            LODUpdateList[j+1]->vecIndex = j+1;
        }

        LODUpdateList[j+1] = temp;
        LODUpdateList[j+1]->vecIndex = j+1;
    }
}

void Planet::destroy()
{
    for (size_t i = 0; i < 6; i++){
        for (size_t j = 0; j < faces[i].size(); j++){
            for (size_t k = 0; k < faces[i][j].size(); k++){
                delete faces[i][j][k];
            }
        }
        faces[i].clear();
    }
    for (size_t i = 0; i < allBiomesLookupVector.size(); i++){
        delete allBiomesLookupVector[i];
    }
    allBiomesLookupVector.clear();

    for (size_t i = 0; i < floraNoiseFunctions.size(); i++){
        delete floraNoiseFunctions[i];
    }
    floraNoiseFunctions.clear();
    for (size_t i = 0; i < treeTypeVec.size(); i++){
        delete treeTypeVec[i];
    }
    treeTypeVec.clear();
    for (size_t i = 0; i < floraTypeVec.size(); i++){
        delete floraTypeVec[i];
    }
    floraTypeVec.clear();

    vg::TextureCache* textureCache = GameManager::textureCache;
    textureCache->freeTexture(biomeMapTexture);
    textureCache->freeTexture(sunColorMapTexture);
    textureCache->freeTexture(sunColorMapTexture);
    textureCache->freeTexture(waterColorMapTexture);
}

void Planet::clearBiomes() //MEMORY LEAKS ARE ON PURPOSE. NOT MEANT FOR FINAL GAME
{
    bindex = 0;
    allBiomesLookupVector.clear();
    mainBiomesVector.clear();
    childBiomesVector.clear();
    baseBiomesLookupMap.clear();
}

void Planet::addBaseBiome(Biome *baseBiome, int mapColor)
{
    baseBiome->vecIndex = bindex++;
    baseBiomesLookupMap.insert(make_pair(mapColor, baseBiome));
    allBiomesLookupVector.push_back(baseBiome);

    //biome = new Biome; //freed in chunkmanager destructor
    //biome->name = name;
    //biome->r = color.r;
    //biome->g = color.g;
    //biome->b = color.b;
    //biome->surfaceBlock = surfaceBlock;
    //biome->treeChance = treeChance;
    //biome->vecIndex = index++;
    //biome->possibleTrees = treetypes;
    //biome->treeProbabilities = treeprobs;
    //biome->possibleFlora = floratypes;
    //biome->floraProbabilities = floraprobs;
    //biomesLookupMap.insert(make_pair(mapColor, biome));
    //biomesLookupVector.push_back(biome);
}

void Planet::addMainBiome(Biome *mainBiome)
{
    mainBiome->vecIndex = bindex++;
    allBiomesLookupVector.push_back(mainBiome);
    mainBiomesVector.push_back(mainBiome);
}

void Planet::addChildBiome(Biome *childBiome)
{
    childBiome->vecIndex = bindex++;
    allBiomesLookupVector.push_back(childBiome);
    childBiomesVector.push_back(childBiome);
}

double Planet::getGravityAccel(double dist)
{
    if (dist < radius) dist = radius;
    dist = dist*0.5; //convert to meters

    return (mass*M_G/(dist*dist)) * 0.01666 * 0.01666 * 2.0; //divide by 60 twice to account for frames
}

double Planet::getAirFrictionForce(double dist, double velocity)
{
    if (dist > atmosphere.radius) return 0.0; //no friction in space duh
    velocity *= 0.5; //reduce to m/s
    return 0.25*getAirDensity(dist)*velocity*velocity; //assuming drag of 0.5
}


//earths density is 1.2 kg/m3
//barometric formula
double Planet::getAirDensity(double dist)
{
    double maxDensity = 1.2;
    double mult;
    if (dist <= radius) return maxDensity;
    if (dist >= atmosphere.radius) return 0;
    mult = (dist-radius)/(atmosphere.radius-radius);
    return (pow(0.8, mult*20.0)-0.0115) * maxDensity;
}

Atmosphere::Atmosphere()
{
    m_Kr = 0.0025f;        // Rayleigh scattering constant
    m_Km = 0.0020f;        // Mie scattering constant
    m_ESun = 25.0f;        // Sun brightness constant
    m_g = -0.990f;        // The Mie phase asymmetry factor
    m_fExposure = 2.0f;
    m_fRayleighScaleDepth = 0.25f; //0.25

    m_fWavelength[0] = 0.650f;        // 650 nm for red
    m_fWavelength[1] = 0.570f;        // 570 nm for green
    m_fWavelength[2] = 0.475f;        // 475 nm for blue
    m_fWavelength4[0] = powf(m_fWavelength[0], 4.0f);
    m_fWavelength4[1] = powf(m_fWavelength[1], 4.0f);
    m_fWavelength4[2] = powf(m_fWavelength[2], 4.0f);

    nSamples = 3;
    fSamples = (float)nSamples;
}

void Atmosphere::initialize(string filePath, float PlanetRadius)
{
    if (filePath.empty()){
        m_Kr = 0.0025;
        m_Km = 0.0020;
        m_ESun = 25.0;
        m_g = -0.990;
        m_fExposure = 2.0;
        m_fWavelength[0] = 0.650;
        m_fWavelength[1] = 0.570;
        m_fWavelength[2] = 0.475;
        nSamples = 3;

        fSamples = (float)nSamples;
        m_fWavelength4[0] = powf(m_fWavelength[0], 4.0f);
        m_fWavelength4[1] = powf(m_fWavelength[1], 4.0f);
        m_fWavelength4[2] = powf(m_fWavelength[2], 4.0f);
    }
    else{
        loadProperties(filePath);
    }

    radius = PlanetRadius*(1.025);
    planetRadius = PlanetRadius;

    cout << "Loading Objects/icosphere.obj... ";
    objectLoader.load("Objects/icosphere.obj", vertices, indices);
    cout << "Done!\n";

    glGenBuffers(1, &(vboID)); // Create the buffer ID
    glBindBuffer(GL_ARRAY_BUFFER, vboID); // Bind the buffer (vertex array data)
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(ColorVertex), NULL, GL_STATIC_DRAW);

    glGenBuffers(1, &(vboIndexID));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndexID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), NULL, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(ColorVertex), &(vertices[0].position));
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLushort), &(indices[0]));
    indexSize = indices.size();

    vertices.clear();
    indices.clear();
}

void Atmosphere::draw(float theta, const glm::mat4 &MVP, glm::vec3 lightPos, const glm::dvec3 &ppos) {
    vg::GLProgram* shader = GameManager::glProgramManager->getProgram("Sky");
    shader->use();

    glm::mat4 GlobalModelMatrix(1.0);
    GlobalModelMatrix[0][0] = radius;
    GlobalModelMatrix[1][1] = radius;
    GlobalModelMatrix[2][2] = radius;
    GlobalModelMatrix[3][0] = (f32)-ppos.x;
    GlobalModelMatrix[3][1] = (f32)-ppos.y;
    GlobalModelMatrix[3][2] = (f32)-ppos.z;

    // Have to rotate it and draw it again to make a sphere
    f32v3 EulerAngles(M_PI, 0, 0);
    f32m4 RotationMatrix = glm::toMat4(glm::quat(EulerAngles));
    f32m4 MVPr = MVP * GlobalModelMatrix;
    f32m4 M = GlobalModelMatrix;

    f32 m_Kr4PI = m_Kr * 4.0f* M_PI;
    f32 m_Km4PI = m_Km * 4.0f* M_PI;
    f32 m_fScale = 1.0 / (radius - planetRadius);

    glUniformMatrix4fv(shader->getUniform("unWVP"), 1, GL_FALSE, &MVPr[0][0]);
    glUniform3f(shader->getUniform("unCameraPos"), (float)ppos.x, (float)ppos.y, (float)ppos.z);
    glUniform3f(shader->getUniform("unLightPos"), lightPos.x, lightPos.y, lightPos.z);
    glUniform3f(shader->getUniform("unInvWavelength"), 1 / m_fWavelength4[0], 1 / m_fWavelength4[1], 1 / m_fWavelength4[2]);
    glUniform1f(shader->getUniform("unCameraHeight2"), glm::length(ppos) * glm::length(ppos));
    glUniform1f(shader->getUniform("unOuterRadius"), radius);
    glUniform1f(shader->getUniform("unOuterRadius2"), radius * radius);
    glUniform1f(shader->getUniform("unInnerRadius"), planetRadius);
    glUniform1f(shader->getUniform("unKrESun"), m_Kr*m_ESun);
    glUniform1f(shader->getUniform("unKmESun"), m_Km*m_ESun);
    glUniform1f(shader->getUniform("unKr4PI"), m_Kr4PI);
    glUniform1f(shader->getUniform("unKm4PI"), m_Km4PI);
    glUniform1f(shader->getUniform("unScale"), m_fScale);
    glUniform1f(shader->getUniform("unScaleDepth"), m_fRayleighScaleDepth);
    glUniform1f(shader->getUniform("unScaleOverScaleDepth"), m_fScale/m_fRayleighScaleDepth);
    glUniform1f(shader->getUniform("unG"), m_g);
    glUniform1f(shader->getUniform("unG2"), m_g*m_g);
    glUniform1i(shader->getUniform("unNumSamples"), nSamples);
    glUniform1f(shader->getUniform("unNumSamplesF"), fSamples);

    glBindBuffer(GL_ARRAY_BUFFER, vboID);

    shader->enableVertexAttribArrays();

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ColorVertex), (void*)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndexID);

    glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_SHORT, 0);

    shader->disableVertexAttribArrays();

    shader->unuse();
}