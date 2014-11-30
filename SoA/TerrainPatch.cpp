#include "stdafx.h"
#include "TerrainPatch.h"

#include "BlockData.h"
#include "Camera.h"
#include "Chunk.h"
#include "FloraGenerator.h"
#include "GameManager.h"
#include "MessageManager.h"
#include "Options.h"
#include "Planet.h"
#include "WorldStructs.h"

#include "utils.h"

int lodDetailLevels[DetailLevels+3] = {8/scale, 16/scale, 32/scale, 64/scale, 128/scale, 256/scale, 512/scale, 1024/scale, 2048/scale, 4096/scale, 8192/scale, 16384/scale, 32768/scale, 65536/scale, 131072/scale, 262144/scale, 524228/scale};
int lodDistanceLevels[DetailLevels] = {500/scale, 1000/scale, 2000/scale, 4000/scale, 8000/scale, 16000/scale, 32000/scale, 64000/scale, 128000/scale, 256000/scale, 512000/scale, 1024000/scale, 4096000/scale, INT_MAX};

vector<TerrainVertex> tvboVerts(8192, TerrainVertex()); //this is bigger that it needs to be but whatever
vector<GLushort> lodIndices(32768);

int WaterIndexMap[(maxVertexWidth+3)*(maxVertexWidth+3)*2];
int MakeWaterQuadMap[(maxVertexWidth+3)*(maxVertexWidth+3)];

//a   b
//c   d
inline double BilinearInterpolation(int &a, int &b, int &c, int &d, int &step, float &x, float &z)
{
    double px, pz;
    px = ((double)(x))/step;
    pz = ((double)(z))/step;

    return  (a)*(1-px)*(1-pz) +
            (b)*px*(1-pz) +
            (c)*(1-px)*pz +
            (d)*px*pz;
}

TerrainPatch::TerrainPatch(int lodWidth){
    for (int i = 0; i < 4; i++){
        lods[i] = NULL;
    }
    width = lodWidth;
    terrainBuffers = NULL;
    hasBuffers = 0;
    updateVecIndex = -1;
    vecIndex = -1;
    lodMap = NULL;
}

TerrainPatch::~TerrainPatch(){
    DeleteChildren();
    ClearLODMap();
    ClearBuffers();
}

void TerrainPatch::ClearLODMap()
{
    if (lodMap != NULL){
        delete[] lodMap;
        lodMap = NULL;
    }
}

void TerrainPatch::NullifyBuffers()
{
    if (lods[0]){
        for (unsigned int i = 0; i < 4; i++){
            lods[i]->NullifyBuffers();
        }
    }
    terrainBuffers = NULL;
    hasBuffers = 0;
}

void TerrainPatch::ClearBuffers()
{
    if (terrainBuffers){
        TerrainMeshMessage *tmm = new TerrainMeshMessage;
        tmm->face = face;
        tmm->terrainBuffers = terrainBuffers;
        hasBuffers = 0;
        terrainBuffers = NULL;
        Message message(MessageID::TERRAIN_MESH, (void *)tmm);
        GameManager::messageManager->enqueue(ThreadId::UPDATE, message);
    }
}

void TerrainPatch::ClearTreeBuffers()
{
    if (terrainBuffers && terrainBuffers->treeIndexSize){
        TerrainMeshMessage *tmm = new TerrainMeshMessage;
        tmm->face = face;
        tmm->terrainBuffers = terrainBuffers;
        Message message(MessageID::REMOVE_TREES, (void *)tmm);
        GameManager::messageManager->enqueue(ThreadId::UPDATE, message);
    }
}

void TerrainPatch::DeleteChildren()
{
    if (lods[0]){
        for (unsigned int i = 0; i < 4; i++){
            delete lods[i];
            lods[i] = NULL;
        }
    }
}

//itinializes the LOD and computes distance
void TerrainPatch::Initialize(int x, int y, int z, int wx, int wy, int wz, int Radius, int Face, TerrainPatch *Parent, int ChildNum, int initialDetailLevel)
{
    childNum = ChildNum;
    terrainBuffers = NULL;
    hasBuffers = 0;
    drawTrees = 0;
    lodMap = NULL;

    for (unsigned int i = 0; i < 4; i++){
        lods[i] = NULL;
    }
    parent = Parent;
    updateCode = 1;
    vecIndex = -1;
    updateVecIndex = -1;
    hasSkirt = 1;
    radius = Radius;
    scaledRadius = radius/planetScale;
    face = Face;
    hasBoundingBox = 0;
    cullRadius = 0;
    hasProperDist = 0;

    X = x;
    Y = y;
    Z = z;

    double vx = X+width/2;
    double vy = Y+width/2;
    double vz = Z+width/2;

    //double tmph = currTerrainGenerator->GenerateSingleHeight(vx, vy, vz);

    double magnitude = sqrt(vx*vx+vy*vy+vz*vz);
    vx /= magnitude;
    vy /= magnitude;
    vz /= magnitude;

    worldX = vx*(scaledRadius+500*invPlanetScale);
    worldY = vy*(scaledRadius+500*invPlanetScale);
    worldZ = vz*(scaledRadius+500*invPlanetScale);

    drawX = worldX;
    drawY = worldY;
    drawZ = worldZ;

    closestPoint.x = worldX;
    closestPoint.y = worldY;
    closestPoint.z = worldZ;

    double dx = (double)(worldX - wx);
    double dy = (double)(worldY - wy);
    double dz = (double)(worldZ - wz);
    //approximate distance, not true distance until mesh has been made initially
    distance = sqrt(dx*dx + dy*dy + dz*dz) - width*0.5*1.4142;
    if (distance < 0) distance = 0;


    if (initialDetailLevel != -1){
        detailLevel = initialDetailLevel;
        step = lodDetailLevels[detailLevel+graphicsOptions.lodDetail];
        while (step > width){
            step /= 2;
            detailLevel--;
        }
    }else{
        CalculateDetailLevel(distance, 0);
    }

    if (detailLevel == 0) hasSkirt = 0;
    if ((width / step) > maxVertexWidth){
        CreateChildren(wx, wy, wz);
    }
}

void TerrainPatch::Draw(TerrainMesh *tb, const Camera* camera, const glm::dvec3 &PlayerPos, const glm::dvec3 &rotPlayerPos, const glm::mat4 &VP, GLuint mvpID, GLuint worldOffsetID, bool onPlanet)
{//
    //calculate distance
    glm::dvec3 closestPoint;
    closestPoint.x = (rotPlayerPos.x <= tb->worldX) ? tb->worldX : ((rotPlayerPos.x > tb->worldX + tb->boundingBox.x) ? (tb->worldX + tb->boundingBox.x) : rotPlayerPos.x);
    closestPoint.y = (rotPlayerPos.y <= tb->worldY) ? tb->worldY : ((rotPlayerPos.y > tb->worldY + tb->boundingBox.y) ? (tb->worldY + tb->boundingBox.y) : rotPlayerPos.y);
    closestPoint.z = (rotPlayerPos.z <= tb->worldZ) ? tb->worldZ : ((rotPlayerPos.z > tb->worldZ + tb->boundingBox.z) ? (tb->worldZ + tb->boundingBox.z) : rotPlayerPos.z);

    double dx = (double)(closestPoint.x) - rotPlayerPos.x;
    double dy = (double)(closestPoint.y) - rotPlayerPos.y;
    double dz = (double)(closestPoint.z) - rotPlayerPos.z;
    double distance = sqrt(dx*dx + dy*dy + dz*dz);

    tb->distance = distance;
    if (distance < closestTerrainPatchDistance) closestTerrainPatchDistance = distance;
    if (camera->sphereInFrustum(f32v3(f64v3(tb->worldX, tb->worldY, tb->worldZ) + f64v3(tb->boundingBox) / 2.0 - rotPlayerPos), (float)tb->cullRadius)
        && (distance == 0 || CheckHorizon(rotPlayerPos, closestPoint))){
        tb->inFrustum = 1;

        glm::mat4 MVP;
        if (onPlanet){
            GlobalModelMatrix[3][0] = ((float)((double)tb->drawX - PlayerPos.x));
            GlobalModelMatrix[3][1] = ((float)((double)tb->drawY - PlayerPos.y));
            GlobalModelMatrix[3][2] = ((float)((double)tb->drawZ - PlayerPos.z));
            MVP = VP  * GlobalModelMatrix;
        }
        else{
            GlobalModelMatrix[3][0] = ((float)((double)-PlayerPos.x));
            GlobalModelMatrix[3][1] = ((float)((double)-PlayerPos.y));
            GlobalModelMatrix[3][2] = ((float)((double)-PlayerPos.z));
            MVP = VP  * GlobalModelMatrix * GameManager::planet->rotationMatrix;

            GlobalModelMatrix[3][0] = ((float)((double)tb->drawX));
            GlobalModelMatrix[3][1] = ((float)((double)tb->drawY));
            GlobalModelMatrix[3][2] = ((float)((double)tb->drawZ));
            MVP *= GlobalModelMatrix;
        }

        glUniformMatrix4fv(mvpID, 1, GL_FALSE, &MVP[0][0]);

        glUniform3f(worldOffsetID, tb->drawX, tb->drawY, tb->drawZ);

        glBindVertexArray(tb->vaoID);
        glDrawElements(GL_TRIANGLES, tb->indexSize, GL_UNSIGNED_SHORT, 0);
        glBindVertexArray(0);
    }
    else{
        tb->inFrustum = 0;
    }
}

void TerrainPatch::DrawTrees(TerrainMesh *tb, const vg::GLProgram* program, const glm::dvec3 &PlayerPos, const glm::mat4 &VP)
{

    if (tb->inFrustum){
        GlobalModelMatrix[3][0] = ((float)((double)tb->drawX - PlayerPos.x));
        GlobalModelMatrix[3][1] = ((float)((double)tb->drawY - PlayerPos.y));
        GlobalModelMatrix[3][2] = ((float)((double)tb->drawZ - PlayerPos.z));

        glm::mat4 MVP = VP * GlobalModelMatrix;

        glUniformMatrix4fv(program->getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(program->getUniform("M"), 1, GL_FALSE, &GlobalModelMatrix[0][0]);

        glBindBuffer(GL_ARRAY_BUFFER, tb->treeVboID);

        //position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TreeVertex), 0);
        //particle center
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TreeVertex), ((char *)NULL + (8)));
        //leaf color and size
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(TreeVertex), ((char *)NULL + (20)));
        //trunk color and ltex
        glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(TreeVertex), ((char *)NULL + (24)));
                
        glDrawElements(GL_TRIANGLES, tb->treeIndexSize, GL_UNSIGNED_INT, NULL);
    }
}

bool TerrainPatch::CheckHorizon(const glm::dvec3 &PlayerPos, const glm::dvec3 &ClosestPoint)
{
    int pLength = glm::length(PlayerPos);
    if (pLength < GameManager::planet->scaledRadius + 1) pLength = GameManager::planet->scaledRadius + 1;
    double horizonAngle = acos((double)GameManager::planet->scaledRadius / pLength);
    double lodAngle = acos(glm::dot(glm::normalize(PlayerPos), glm::normalize(ClosestPoint)));
    if (lodAngle < horizonAngle+0.1) return 1;
    return 0;
}

const bool colorDebug = 0;
bool TerrainPatch::CreateMesh()
{
//    //DEBUGGING
//    if (step <= 0) {
//        pError("AHH OH JESUS!");
//    }
//
//    TerrainGenerator* generator = GameManager::terrainGenerator;
//
//    GLuint time0, time1, time2, time3;
//    bool removeChildren = 0;
//
//    //cout << updateCode << endl;
//    if (updateCode == 0) return 1; //doesnt need to be meshed
//
//    if (updateCode != 2){ //2 == remove children
//        if (lods[0]){ //if it is a parent, load the children
//            for (int i = 0; i < 4; i++){
//                if (lods[i]->updateCode != 0){
//                    lods[i]->CreateMesh();
//                    return 0;
//                }
//            }
//            
//            updateCode = 0;
//            ClearLODMap();
//            ClearBuffers();
//            return 1;
//        }
//    }else{
//        removeChildren = 1;
//        updateCode = 1;
//    }
//
//    int treen = 0;
//    
//    GLuint sticks = SDL_GetTicks();
//
//    //int step = step;
//    //its +3 instead of +2 because we need to have duplicated vertices on +z and +x edge to fill holes.
//    int size = width/step + 3;
//    //error 16384 32768
//
//    bool hasWater = 0;
//
//    //BEFORE: 8936
//    //NONE: 5652
//    bool genTrees = (step <= 32);
//
//    int ioff, joff;
//    int treeIndex = 0;
//    int index = 0;
//    int indice = 0;
//    int temperature;
//    int snowDepth, sandDepth;
//    int rainfall;
//    int tx, tz;
//    double th;
//    int rval;
//    int txc, tzc, tincorig;
//    float tinc;
//    if (step <= 16){
//        tincorig = 1;
//    }else{
//        tincorig = 2;
//    }
//    int treeI;
//    Biome *biome;
//    Block *bp;
//    TreeData treeData;
//    int ipos, jpos, rpos, irel, jrel;
//    glm::vec3 tangent;
//    glm::vec3 biTangent;
//    glm::vec3 deltaPos1, deltaPos2;
//    glm::mat3 normTransformMat;
//    float bY;
//    float h;
//    float lodYoff = -1.0f*invPlanetScale;
//    float skirtLength = (step/2 + 32);
//    double magnitude;
//    glm::dvec3 v1, v2, v3;
//    GLubyte colr, colg, colb;
//    GLubyte ltex;
//
////    bX = 0;
////    bY = 0;
//
//    glm::vec3 up, right, down, left, norm;
//
//    time0 = SDL_GetTicks();
//    
//    if (lodMap == NULL) lodMap = new HeightData[(maxVertexWidth+3)*(maxVertexWidth+3)];
//    lodMapStep = step; //for our future children to use
//    lodMapSize = size;
//    bool happened = 0;
//
//    switch (face){
//    case P_TOP: //top
//        ipos = 2;
//        jpos = 0;
//        rpos = 1;
//        irel = Z - step;
//        jrel = X - step;
//        rval = radius;
//        generator->SetLODFace(ipos, jpos, rpos, rval, planetScale);
//        if (parent != NULL && parent->lodMap != NULL){
//            //dont have to generate everything since the parent has some of it already loaded
//            if (parent->lodMapStep == step){ //same step as parent but half size
//                hasWater = 1;
//                switch (childNum){
//                    case 0:
//                        ioff = 0;
//                        joff = 0;
//                        break;
//                    case 1:
//                        ioff = 0;
//                        joff = size-3;
//                        break;
//                    case 2:
//                        ioff = size-3;
//                        joff = 0;
//                        break;
//                    case 3:
//                        ioff = size-3;
//                        joff = size-3;
//                        break;
//                }
//                for (int i = 0; i < size; i++){
//                    for (int j = 0; j < size; j++){
//                        lodMap[i*size+j] = parent->lodMap[(i+ioff)*(parent->lodMapSize)+(j+joff)];
//                    }
//                }
//            }else if (0 && step*2 == parent->lodMapStep && size == parent->lodMapSize){ //double step of parent, but same size
//                switch (childNum){
//                    case 0:
//                        ioff = 0;
//                        joff = 0;
//                        break;
//                    case 1:
//                        ioff = 0;
//                        joff = (parent->lodMapSize-3)/2+1;
//                        break;
//                    case 2:
//                        ioff = (parent->lodMapSize-3)/2+1;
//                        joff = 0;
//                        break;
//                    case 3:
//                        ioff = (parent->lodMapSize-3)/2+1;
//                        joff = (parent->lodMapSize-3)/2+1;
//                        break;
//                }
//                //cout << size << " " << (parent->lodMapSize-3)/2+1 << " " << endl;
//
//                generator->GenerateHeightMap(lodMap, irel, jrel, size, size, size, step, 0, &hasWater);
//                for (int i = 0; i < size-2; i+=2){
//                    for (int j = 0; j < size-2; j+=2){
//                        lodMap[(i+1)*size+(j+1)] = parent->lodMap[(i/2+ioff+1)*(parent->lodMapSize)+(j/2+joff+1)];
//                    }
//                }
//            }else{
//                generator->GenerateHeightMap(lodMap, irel, jrel, size, size, size, step, 0, &hasWater);
//            }
//            //else if (parent->lodMapStep == step*2){
//        //        generator->GenerateLODMap(lodMap, X-step, radius, Z-step, size, step, 2); 
//        //    }
//        }else{
//            //regenerate everything
//            generator->GenerateHeightMap(lodMap, irel, jrel, size, size, size, step, 0, &hasWater);
//        }
//        rval = scaledRadius;
//    //    Y = lodMap[(size*size)/2+1].height;
//        break;
//    case P_LEFT: //left
//        rval = -radius;
//        ipos = 1;
//        jpos = 2;
//        rpos = 0;
//        irel = Y - step;
//        jrel = Z - step;
//        generator->SetLODFace(ipos, jpos, rpos, rval, planetScale);
//        generator->GenerateHeightMap(lodMap, irel, jrel, size, size, size, step, 0, &hasWater);
//        rval = -scaledRadius;
//        break;
//    case P_RIGHT: //right
//        rval = radius;
//        ipos = 1;
//        jpos = 2;
//        rpos = 0;
//        irel = Y - step;
//        jrel = Z - step;
//        generator->SetLODFace(ipos, jpos, rpos, rval, planetScale);
//        generator->GenerateHeightMap(lodMap, irel, jrel, size, size, size, step, 0, &hasWater);
//        rval = scaledRadius;
//        break;
//    case P_FRONT: //front
//        rval = radius;
//        ipos = 1;
//        jpos = 0;
//        rpos = 2;
//        irel = Y - step;
//        jrel = X - step;
//        generator->SetLODFace(ipos, jpos, rpos, rval, planetScale);
//        generator->GenerateHeightMap(lodMap, irel, jrel, size, size, size, step, 0, &hasWater);
//        rval = scaledRadius;
//    //    Y = lodMap[(size*size)/2+1].height;
//        break;
//    case P_BACK: //back
//        rval = -radius;
//        ipos = 1;
//        jpos = 0;
//        rpos = 2;
//        irel = Y - step;
//        jrel = X - step;
//        generator->SetLODFace(ipos, jpos, rpos, rval, planetScale);
//        generator->GenerateHeightMap(lodMap, irel, jrel, size, size, size, step, 0, &hasWater);
//        rval = -scaledRadius;
//        break;
//    case P_BOTTOM: //bottom
//        ipos = 2;
//        jpos = 0;
//        rpos = 1;
//        irel = Z - step;
//        jrel = X - step;
//        rval = -radius;
//        generator->SetLODFace(ipos, jpos, rpos, rval, planetScale);
//        generator->GenerateHeightMap(lodMap, irel, jrel, size, size, size, step, 0, &hasWater);
//        rval = -scaledRadius;
//        break;
//    }
//
//    bY = 0;
//
//    int maxX = INT_MIN;
//    int minX = INT_MAX;
//    int maxY = INT_MIN;
//    int minY = INT_MAX;
//    int maxZ = INT_MIN;
//    int minZ = INT_MAX;
//    float texInc;
//    int tStartIndex;
//    //dir is  0 - 3 top right down left
//
//    int isize = size-2;
//    //**************************************** WATER *********************************************
//    texInc = step/1024.0;
//    double uStart = texInc*((jrel+step)/step);
//    double vStart = texInc*((irel+step)/step);
//    if (hasWater){    
//
//        int sindex = index;
//
//        //9530 1,227,270
//        for (int z = 1; z < size-1; z++){ //Z
//            for (int x = 1; x < size-1; x++){ //X
//
//                h = (int)lodMap[z*size+x].height;
//                //check and see if all adjacent heights are above water. If so, then we dont need this water vertex at all. TODO: maybe sentinalize this?
//                if (h >= 0 && (x == 1 || lodMap[z*size+(x-1)].height >= 0) //left
//                                                 && (z == 1 || lodMap[(z-1)*size+x].height >= 0) //back
//                                                 && ((x == 1 && z == 1) || lodMap[(z-1)*size+(x-1)].height >= 0) //left back
//                                                 && (x == size-2 || lodMap[z*size+(x+1)].height >= 0) //right
//                                                 && (z == size-2 || lodMap[(z+1)*size+x].height >= 0) //front
//                                                 && ((x == size-2 && z == 1) || lodMap[(z-1)*size+(x+1)].height >= 0) //right back
//                                                 && ((x == 1 && z == size-2) || lodMap[(z+1)*size+(x-1)].height >= 0) //left front
//                                                 && ((x == size-2 && z == size-2) || lodMap[(z+1)*size+(x+1)].height >= 0)) { //right front
//
//                    MakeWaterQuadMap[(z-1)*isize + (x-1)] = 0; //mark vertex as not being a quad start
//                    //now we have to check that other quads dont need to use this quad.
//                    if ((x == 1 || MakeWaterQuadMap[(z-1)*isize + (x-2)] == 0) && (z == 1 || MakeWaterQuadMap[(z-2)*isize + (x-1)] == 0) 
//                                                                        && ((x == 1 && z == 1) || MakeWaterQuadMap[(z-2)*isize + (x-2)] == 0)){
//                        continue; //skip this vertex since its useless. 
//                    }
//                }else{
//                    MakeWaterQuadMap[(z-1)*isize + (x-1)] = 1;
//                }
//
//                temperature = lodMap[z*size+x].temperature;
//                rainfall = lodMap[z*size+x].rainfall;
//
//                int depth = -h/5;
//                if (depth > 255){
//                    depth = 255;
//                }else if (depth < 0) {
//                    depth = 0;
//                }
//
//                v1[ipos] = (double)irel+(double)(z*step);
//                v1[jpos] = (double)jrel+(double)(x*step);
//                v1[rpos] = rval;
//
//                magnitude = sqrt(v1.x*v1.x+v1.y*v1.y+v1.z*v1.z);
//                v1 /= magnitude;
//
//                if (index > tvboVerts.size()){
//                    cout << index << " " << sindex << " " << size << endl;
//                    cout << tvboVerts.size() << endl;
//                    int q;
//                    cin >> q;
//                }
//                tvboVerts[index].normal = glm::vec3(v1); //size = 0 708
//
//            //    cout << v1.x << " " << v1.y << " " << v1.z << endl;
//
//                magnitude = (double)scaledRadius+lodYoff;// + invPlanetScale;
//                v1.x = v1.x*magnitude - (double)drawX;
//                v1.y = v1.y*magnitude - (double)drawY; 
//                v1.z = v1.z*magnitude - (double)drawZ;
//
//                minX = MIN(minX, v1.x);
//                maxX = MAX(maxX, v1.x);
//                minY = MIN(minY, v1.y);
//                maxY = MAX(maxY, v1.y);
//                minZ = MIN(minZ, v1.z);
//                maxZ = MAX(maxZ, v1.z);
//
//                tvboVerts[index].location[0] = (GLfloat)v1.x;
//                tvboVerts[index].location[1] = (GLfloat)v1.y;
//                tvboVerts[index].location[2] = (GLfloat)v1.z;
//                tvboVerts[index].color[0] = 255;
//                tvboVerts[index].color[1] = 255;
//                tvboVerts[index].color[2] = 255;
//                tvboVerts[index].tex[0] = uStart + texInc*(x-1);
//                tvboVerts[index].tex[1] = vStart + texInc*(z-1);
//                tvboVerts[index].textureUnit = 1;
//
//                tvboVerts[index].temperature = (GLubyte)temperature;
//                tvboVerts[index].rainfall = 255 - (GLubyte)depth; //water needs depth
//                tvboVerts[index].specular = 255;
//
//                WaterIndexMap[(z-1)*isize + (x-1)] = index;
//                index++;
//            }
//        }
//
//        int isize = size-2;
//        int istart;
//        for (int z = 0; z < isize-1; z++){
//            for (int x = 0; x < isize-1; x++){
//                if (MakeWaterQuadMap[z*isize + x]){ //check for skipped quad
//                    istart = WaterIndexMap[z*isize + x];
//                    lodIndices[indice] = istart;
//                    lodIndices[indice+1] = WaterIndexMap[(z+1)*isize + x];
//                    lodIndices[indice+2] = WaterIndexMap[(z+1)*isize + (x+1)];
//                    lodIndices[indice+3] = WaterIndexMap[(z+1)*isize + (x+1)];
//                    lodIndices[indice+4] = WaterIndexMap[z*isize + (x+1)];
//                    lodIndices[indice+5] = istart;        
//                    indice += 6;    
//                }
//            }
//        }
//    }
//    tStartIndex = index;
//    texInc = 2.0f/(size-2);
//    //*************************************** TERRAIN ********************************************
//    time0 = SDL_GetTicks() - time0;
//    time1 = SDL_GetTicks();
//    for (int i = 1; i < size-1; i++){ //Z
//        for (int j = 1; j < size-1; j++){ //X
//            temperature = lodMap[i*size+j].temperature;
//            rainfall = lodMap[i*size+j].rainfall;
//            h = lodMap[i*size+j].height * invPlanetScale;
//            snowDepth = (int)(lodMap[i*size+j].snowDepth * invPlanetScale);
//            sandDepth = (int)(lodMap[i*size+j].sandDepth * invPlanetScale);
//            biome = lodMap[i*size+j].biome;
//
//            if (biome->treeChance > 0.08){
//                tinc = 2;
//            }else if (biome->treeChance < 0.015){
//                tinc = 1;
//            }else{
//                tinc = tincorig;
//            }
//            tinc = 1*invPlanetScale;
//
//            if (genTrees && snowDepth < 7*invPlanetScale){
//                for (float z1 = 0; z1 < step; z1+=tinc){
//                    for (float x1 = 0; x1 < step; x1+=tinc){
//                        if (h > 0){
//                            tx = jrel + radius + j*step + x1; tz = irel + radius + i*step + z1; //gotta add radius since irel and jrel are -rad to rad while trees are 0-2rad
//
//                            if (tx < 0){
//                                txc = 31 + tx%CHUNK_WIDTH;
//                            }else{
//                                txc = tx%CHUNK_WIDTH;
//                            }
//                            if (tz < 0){
//                                tzc = 31 + tz%CHUNK_WIDTH;
//                            }else{
//                                tzc = tz%CHUNK_WIDTH;
//                            }
//
//                            //20 fps
//                            if (txc && tzc){
//                                treeI = FloraGenerator::getTreeIndex(biome, tx, tz);
//                                if (treeI != -1){
//                                    if (!FloraGenerator::makeLODTreeData(treeData, GameManager::planet->treeTypeVec[treeI], txc, tzc, tx - txc, tz - tzc)){
//                                        if (treeIndex >= TREE_VERTS_SIZE) exit(1);
//
//                                        th = BilinearInterpolation(lodMap[i*size + j].height, lodMap[i*size + j+1].height, lodMap[(i+1)*size + j].height, lodMap[(i+1)*size + j+1].height, step, z1, x1) * invPlanetScale;
//                                        if (th <= 0) continue;
//
//                                        float size = treeData.treeHeight * invPlanetScale;
//                                        if (size < 1.0f) size = 1.0f;
//
//                                        v1[ipos] = (double)irel+(double)(i*step) + z1;
//                                        v1[jpos] = (double)jrel+(double)(j*step) + x1;
//                                        v1[rpos] = rval;
//                                        magnitude = sqrt(v1.x*v1.x+v1.y*v1.y+v1.z*v1.z);
//                                        v1 /= magnitude;
//
//                                        magnitude = (double)scaledRadius+(double)th+(double)snowDepth+(double)sandDepth+lodYoff;
//                                        v1.x = v1.x*magnitude - (double)drawX;
//                                        v1.y = v1.y*magnitude - (double)drawY;
//                                        v1.z = v1.z*magnitude - (double)drawZ;
//
//                                        if (treeData.leafColor && GETBLOCK((treeData.treeType->idLeaves)).altColors.size() > treeData.leafColor - 1){
//                                            colr = Blocks[treeData.treeType->idLeaves].altColors[treeData.leafColor-1].r;
//                                            colg = Blocks[treeData.treeType->idLeaves].altColors[treeData.leafColor-1].g;
//                                            colb = Blocks[treeData.treeType->idLeaves].altColors[treeData.leafColor-1].b;
//                                        } else {
//                                            colr = Blocks[treeData.treeType->idLeaves].color.r;
//                                            colg = Blocks[treeData.treeType->idLeaves].color.g;
//                                            colb = Blocks[treeData.treeType->idLeaves].color.b;
//                                        }
//
//                                        switch (treeData.treeType->leafCapShape){
//                                            case TreeLeafShape::CLUSTER:
//                                                ltex = 0; //none
//                                                break;
//                                            case TreeLeafShape::PINE:
//                                                ltex = 2; //pine
//                                                break;
//                                            case TreeLeafShape::MUSHROOM:
//                                                ltex = 3; //mushroom
//                                                break;
//                                            default:
//                                                ltex = 1; //standard
//                                                break;
//                                        }
//
//                        
//                                        treeVerts[treeIndex].pos = glm::vec2(0.5f,1.0f);
//                                        treeVerts[treeIndex].lr = colr;
//                                        treeVerts[treeIndex].lg = colg;
//                                        treeVerts[treeIndex].lb = colb;
//                                        treeVerts[treeIndex].size = size;
//                                        treeVerts[treeIndex].center = glm::vec3(v1);
//                                        treeVerts[treeIndex].ltex = ltex;
//                                        treeIndex++;
//                                        treeVerts[treeIndex].pos = glm::vec2(-0.5f,1.0f);
//                                        treeVerts[treeIndex].lr = colr;
//                                        treeVerts[treeIndex].lg = colg;
//                                        treeVerts[treeIndex].lb = colb;
//                                        treeVerts[treeIndex].size = size;
//                                        treeVerts[treeIndex].center = glm::vec3(v1);
//                                        treeVerts[treeIndex].ltex = ltex;
//                                        treeIndex++;
//                                        treeVerts[treeIndex].pos = glm::vec2(-0.5f,0.0f);
//                                        treeVerts[treeIndex].lr = colr;
//                                        treeVerts[treeIndex].lg = colg;
//                                        treeVerts[treeIndex].lb = colb;
//                                        treeVerts[treeIndex].size = size;
//                                        treeVerts[treeIndex].center = glm::vec3(v1);
//                                        treeVerts[treeIndex].ltex = ltex;
//                                        treeIndex++;
//                                        treeVerts[treeIndex].pos = glm::vec2(0.5f,0.0f);
//                                        treeVerts[treeIndex].lr = colr;
//                                        treeVerts[treeIndex].lg = colg;
//                                        treeVerts[treeIndex].lb = colb;
//                                        treeVerts[treeIndex].size = size;
//                                        treeVerts[treeIndex].center = glm::vec3(v1);
//                                        treeVerts[treeIndex].ltex = ltex;
//                                        treeIndex++;
//                                        
//                                    }
//                                }
//                            }
//                        }
//                    }
//                }
//            }
//
//            v1[ipos] = (double)irel+(double)(i*step);
//            v1[jpos] = (double)jrel+(double)(j*step);
//            v1[rpos] = rval;
//
//            //need v2 and v3 for computing tangent and bitangent
//            v2[ipos] = (double)irel+(double)(i*step);
//            v2[jpos] = (double)jrel+(double)((j+1)*step);
//            v2[rpos] = rval;
//
//            v3[ipos] = (double)irel+(double)((i+1)*step);
//            v3[jpos] = (double)jrel+(double)(j*step);
//            v3[rpos] = rval;
//
//            up = -glm::vec3(0,lodMap[(i-1)*size+j].height - h, -step);
//            right = glm::vec3(step, lodMap[i*size+j+1].height - h, 0);
//            down = -glm::vec3(0,lodMap[(i+1)*size+j].height - h, step);
//            left = glm::vec3(-step, lodMap[i*size+j-1].height - h, 0);
//            norm = (glm::cross(up, right) + glm::cross(right, down) + glm::cross(down, left) + glm::cross(left, up));
//
//            norm = glm::normalize(norm);
//            //8891
//
//            //normalize
//            magnitude = sqrt(v1.x*v1.x+v1.y*v1.y+v1.z*v1.z);
//
//            v1 /= magnitude;
//            magnitude = sqrt(v2.x*v2.x+v2.y*v2.y+v2.z*v2.z);
//            v2 /= magnitude;
//            magnitude = sqrt(v3.x*v3.x+v3.y*v3.y+v3.z*v3.z);
//            v3 /= magnitude;
//
//            tangent = glm::vec3(glm::normalize(v2 - v1));
//            biTangent = glm::vec3(glm::normalize(v3 - v1));
//
//            //compute matrix multiplication manually
//
//            norm = glm::vec3(tangent.x*norm.x + v1.x*norm.y + biTangent.x*norm.z,
//                             tangent.y*norm.x + v1.y*norm.y + biTangent.y*norm.z,
//                             tangent.z*norm.x + v1.z*norm.y + biTangent.z*norm.z);
//
//            
//            //get final vertex position
//            magnitude = (double)scaledRadius+(double)h+(double)snowDepth+(double)sandDepth+lodYoff + 0.1; //0.1 so that it never overlaps water
//            v1.x = v1.x*magnitude - (double)drawX;
//            v1.y = v1.y*magnitude - (double)drawY;
//            v1.z = v1.z*magnitude - (double)drawZ;
//
//            minX = MIN(minX, v1.x);
//            maxX = MAX(maxX, v1.x);
//            minY = MIN(minY, v1.y);
//            maxY = MAX(maxY, v1.y);
//            minZ = MIN(minZ, v1.z);
//            maxZ = MAX(maxZ, v1.z);
//            
//        //    vx = vx*dh - (double)X;
//        //    vy = vy*dh - (double)(radius + Y);
//        //    vz = vz*dh - (double)Z;
//            
//
//            //cout << vx << " " << vy << " " << vz << endl;
//
//            tvboVerts[index].location[0] = (GLfloat)v1.x;
//            tvboVerts[index].location[1] = (GLfloat)v1.y;
//            tvboVerts[index].location[2] = (GLfloat)v1.z;
//            tvboVerts[index].tex[0] = texInc*(j-1);
//            tvboVerts[index].tex[1] = texInc*(i-1);
//            tvboVerts[index].textureUnit = 0;
//            tvboVerts[index].temperature = (GLubyte)temperature;
//            tvboVerts[index].rainfall = (GLubyte)rainfall;
//            tvboVerts[index].specular = 32;
//                            
//            tvboVerts[index].normal = glm::normalize(norm); //normalize is optional?
//
//            //slope color
//            bp = &(Blocks[biome->surfaceLayers[biome->looseSoilDepth + 1]]);
//            tvboVerts[index].slopeColor = bp->averageColor;
//
//            //beach color
//            bp = &(Blocks[biome->beachBlock]);
//            tvboVerts[index].beachColor = bp->averageColor;
//
//            float yn = tvboVerts[index].normal.y;
//
//            if (0 && h <= 0 && biome->beachBlock == SAND){
//                colr = 194;
//                colg = 178;
//                colb = 128;
//            }else{
//                colr = colg = colb = 255;
//            }
//            if (biome->hasAltColor == 2){
//                colr = biome->r;
//                colg = biome->g;
//                colb = biome->b;
//            }else if (snowDepth || biome->surfaceBlock == SNOW){
//                colr = 255;
//                colg = 255;
//                colb = 255;
//            }else if (sandDepth || biome->surfaceBlock == SAND){
//                colr = 233;
//                colg = 207;
//                colb = 169;
//            }else if (biome->hasAltColor){
//                colr = biome->r;
//                colg = biome->g;
//                colb = biome->b;
//            }
//
//            if (colorDebug == 1){
//                colr = detailLevel%4 * 64;
//                colg = detailLevel%2 * 128;
//                colb = detailLevel%8 * 32;
//            }
//        
//            tvboVerts[index].color[0] = colr;
//            tvboVerts[index].color[1] = colg;
//            tvboVerts[index].color[2] = colb;
//        
//
//            index++;
//        }
//    }
//
//    time1 = SDL_GetTicks() - time1;
//
//    time2 = SDL_GetTicks();
//    int c;
//    for (int z = 0; z < isize-1; z++){
//        for (int x = 0; x < isize-1; x++){ //Z
//            c = tStartIndex + z * isize + x;
//            if (c%2){
//                lodIndices[indice] = c;
//                lodIndices[indice+1] = c + isize;
//                lodIndices[indice+2] = c + 1;
//                lodIndices[indice+3] = c + 1;
//                lodIndices[indice+4] = c + isize;
//                lodIndices[indice+5] = c + isize + 1;
//            }else{
//                lodIndices[indice] = c;
//                lodIndices[indice+1] = c + isize;
//                lodIndices[indice+2] = c + isize + 1;
//                lodIndices[indice+3] = c + isize + 1;
//                lodIndices[indice+4] = c + 1;
//                lodIndices[indice+5] = c;
//            }
//            indice += 6;
//        }
//    }
//
//    if (1 || hasSkirt){
//        glm::vec3 rnormal; //reverse normal
//        glm::vec3 relativeLocation = glm::vec3(drawX, drawY, drawZ); //add this to location to get world location
//
//        //neg Z
//        for (int x = 0; x < isize-1; x++){
//            lodIndices[indice] = tStartIndex + x;
//            lodIndices[indice+1] = tStartIndex + x + 1;
//            lodIndices[indice+2] = index + 1;
//            lodIndices[indice+3] = index + 1;
//            lodIndices[indice+4] = index;
//            lodIndices[indice+5] = tStartIndex + x;    
//            indice += 6;
//
//            tvboVerts[index] = tvboVerts[tStartIndex + x]; //make it a copy of the above vertex        
//            rnormal = -glm::normalize(tvboVerts[index].location + relativeLocation);    
//            tvboVerts[index++].location += rnormal*skirtLength;
//        }
//        tvboVerts[index] = tvboVerts[tStartIndex + isize-1]; //final vertex in the skirt (dont want to add indices on this one)
//        rnormal = -glm::normalize(tvboVerts[index].location + relativeLocation);    
//        tvboVerts[index++].location += rnormal*skirtLength;
//
//        //positive x
//        for (int z = 0; z < isize-1; z++){
//            lodIndices[indice] = tStartIndex + isize*z + isize-1;
//            lodIndices[indice+1] =  tStartIndex + isize*(z+1) + isize-1;
//            lodIndices[indice+2] = index + 1;
//            lodIndices[indice+3] = index + 1;
//            lodIndices[indice+4] = index;
//            lodIndices[indice+5] =  tStartIndex + isize*z + isize-1;    
//            indice += 6;
//
//            tvboVerts[index] = tvboVerts[tStartIndex + isize*z + isize-1]; //make it a copy of the above vertex        
//            rnormal = -glm::normalize(tvboVerts[index].location + relativeLocation);    
//            tvboVerts[index++].location += rnormal*skirtLength;
//        }
//        tvboVerts[index] = tvboVerts[tStartIndex + isize*(isize-1) + isize-1];         
//        rnormal = -glm::normalize(tvboVerts[index].location + relativeLocation);    
//        tvboVerts[index++].location += rnormal*skirtLength;
//
//        //positive Z
//        for (int x = 0; x < isize-1; x++){
//            lodIndices[indice] = tStartIndex + isize*(isize-1) + x;
//            lodIndices[indice+1] = index;
//            lodIndices[indice+2] = index + 1;
//            lodIndices[indice+3] = index + 1;
//            lodIndices[indice+4] = tStartIndex + isize*(isize-1) + x + 1;
//            lodIndices[indice+5] = tStartIndex + isize*(isize-1) + x;    
//            indice += 6;
//
//            tvboVerts[index] = tvboVerts[tStartIndex + isize*(isize-1) + x]; //make it a copy of the above vertex        
//            rnormal = -glm::normalize(tvboVerts[index].location + relativeLocation);    
//            tvboVerts[index++].location += rnormal*skirtLength;
//        }
//        tvboVerts[index] = tvboVerts[tStartIndex + isize*(isize-1) + isize-1];         
//        rnormal = -glm::normalize(tvboVerts[index].location + relativeLocation);    
//        tvboVerts[index++].location += rnormal*skirtLength;
//
//        //neg x
//        for (int z = 0; z < isize-1; z++){
//            lodIndices[indice] = tStartIndex + isize*z;
//            lodIndices[indice+1] = index;
//            lodIndices[indice+2] = index + 1;
//            lodIndices[indice+3] = index + 1;
//            lodIndices[indice+4] = tStartIndex + isize*(z+1);
//            lodIndices[indice+5] =  tStartIndex + isize*z;    
//            indice += 6;
//
//            tvboVerts[index] = tvboVerts[tStartIndex + isize*z]; //make it a copy of the above vertex        
//            rnormal = -glm::normalize(tvboVerts[index].location + relativeLocation);    
//            tvboVerts[index++].location += rnormal*skirtLength;
//        }
//        tvboVerts[index] = tvboVerts[tStartIndex + isize*(isize-1)];    
//        rnormal = -glm::normalize(tvboVerts[index].location + relativeLocation);    
//        tvboVerts[index++].location += rnormal*skirtLength;
//    }
//
//    time2 = SDL_GetTicks() - time2;
//    time3 = SDL_GetTicks();
//
//    //calculate new worldX worldY worldZ and bounding box for distance and frustum culling
//
//    /*if (!hasMesh){
//        cout << endl;
//        cout << worldX << " " << worldY << " " << worldZ << endl;
//    }*/
//    worldX = minX + drawX;
//    worldY = minY + drawY;
//    worldZ = minZ + drawZ;
//    /*if (!hasMesh){
//        cout << worldX << " " << worldY << " " << worldZ << endl;
//    }*/
//    worldNormal = glm::vec3(glm::normalize(glm::dvec3(worldX, worldY, worldZ)));
//
//    boundingBox.x = maxX - minX;
//    boundingBox.y = maxY - minY;
//    boundingBox.z = maxZ - minZ;
//    hasBoundingBox = 1;
//    cullRadius = MAX(boundingBox.x, boundingBox.y);
//    cullRadius = MAX(cullRadius, boundingBox.z);
//
//    updateCode = 0;
//
//    if (indice > lodIndices.size()){
//        pError("Index out of bounds for lodIndices");
//        exit(15);
//    }
//    else if (index > tvboVerts.size()){
//        pError(("Index out of bounds for tvboVerts. " + to_string(index) + " / " + to_string(tvboVerts.size())).c_str());
//        exit(15);
//    }
//
//    
//    if (index > 32768){
//
//        cout << "INDEX: " << endl;
//    }
//
//    TerrainMeshMessage *tmm = new TerrainMeshMessage;
//    if (terrainBuffers == NULL){
//        terrainBuffers = new TerrainMesh();
//    }
//    tmm->boundingBox = boundingBox;
//    tmm->drawX = drawX;
//    tmm->drawY = drawY;
//    tmm->drawZ = drawZ;
//    tmm->worldX = worldX;
//    tmm->worldY = worldY;
//    tmm->worldZ = worldZ;
//    tmm->cullRadius = cullRadius;
//    tmm->terrainBuffers = terrainBuffers;
//    tmm->face = face;
//    if (indice){
//        tmm->verts = new TerrainVertex[index];
//        memcpy(tmm->verts, &(tvboVerts[0]), index*sizeof(TerrainVertex));
//        tmm->indices = new GLushort[indice];
//        memcpy(tmm->indices, &(lodIndices[0]), indice*sizeof(GLushort));
//
//        if (treeIndex){
//            tmm->treeVerts = new TreeVertex[treeIndex];
//            memcpy(tmm->treeVerts, &(treeVerts[0]), treeIndex*sizeof(TreeVertex));
//        }
//    }
//    if (indice == 0){ //this still happens
//        pError("Indice = 0! " + to_string(size) + " " + to_string(isize) + " " + to_string(width) + " " + to_string(step));
//    }
//    tmm->indexSize = indice;
//    tmm->treeIndexSize = treeIndex * 6 / 4;
//    tmm->index = index;
//
//    Message message(MessageID::TERRAIN_MESH, (void *)tmm);
//    GameManager::messageManager->enqueue(ThreadId::UPDATE, message);
//
//    hasBuffers = 1;
//
//    if (removeChildren){
//        //    ExtractChildData();
//        DeleteChildren();
//    }
////    if (happened){
////        cout << "HAPPEN " << size << " " << time0 << " ";
////    }else{
////        cout << "S  " << size << " " << time0 << " ";
////    }
//
//    //GLuint time = SDL_GetTicks() - sticks;
//    //if (time > 10){
//    //    cout << "MESH : " << time << "  " << size << endl;
//    //    cout << "     " << time0 << " " << time1 << " " << time2 << " " << time3 << endl;
//    //}
//    return 1;
    return 1;
}

void TerrainPatch::CreateChildren(int wx, int wy, int wz, int initialDetail)
{
    int hw = width/2;

    lods[0] = new TerrainPatch(hw);
    lods[1] = new TerrainPatch(hw);
    lods[2] = new TerrainPatch(hw);
    lods[3] = new TerrainPatch(hw);

    switch (face){
        case P_TOP:
            lods[0]->Initialize(X, scaledRadius, Z, wx, wy, wz, radius, face, this, 0, initialDetail);  
            lods[1]->Initialize(X+hw, scaledRadius, Z, wx, wy, wz, radius, face, this, 1, initialDetail);
            lods[2]->Initialize(X, scaledRadius, Z+hw, wx, wy, wz, radius, face, this, 2, initialDetail);
            lods[3]->Initialize(X+hw, scaledRadius, Z+hw, wx, wy, wz, radius, face, this, 3, initialDetail);
            break;
        case P_LEFT:
            lods[0]->Initialize(-scaledRadius, Y, Z, wx, wy, wz, radius, face, this, 0, initialDetail);  
            lods[1]->Initialize(-scaledRadius, Y, Z+hw, wx, wy, wz, radius, face, this, 1, initialDetail);
            lods[2]->Initialize(-scaledRadius, Y+hw, Z, wx, wy, wz, radius, face, this, 2, initialDetail);
            lods[3]->Initialize(-scaledRadius, Y+hw, Z+hw, wx, wy, wz, radius, face, this, 3, initialDetail);
            break;
        case P_RIGHT:
            lods[0]->Initialize(scaledRadius, Y, Z, wx, wy, wz, radius, face, this, 0, initialDetail);  
            lods[1]->Initialize(scaledRadius, Y, Z+hw, wx, wy, wz, radius, face, this, 1, initialDetail);
            lods[2]->Initialize(scaledRadius, Y+hw, Z, wx, wy, wz, radius, face, this, 2, initialDetail);
            lods[3]->Initialize(scaledRadius, Y+hw, Z+hw, wx, wy, wz, radius, face, this, 3, initialDetail);
            break;
        case P_FRONT:
            lods[0]->Initialize(X, Y, scaledRadius, wx, wy, wz, radius, face, this, 0, initialDetail);  
            lods[1]->Initialize(X+hw, Y, scaledRadius, wx, wy, wz, radius, face, this, 1, initialDetail);
            lods[2]->Initialize(X, Y+hw, scaledRadius, wx, wy, wz, radius, face, this, 2, initialDetail);
            lods[3]->Initialize(X+hw, Y+hw, scaledRadius, wx, wy, wz, radius, face, this, 3, initialDetail);
            break;
        case P_BACK:
            lods[0]->Initialize(X, Y, -scaledRadius, wx, wy, wz, radius, face, this, 0, initialDetail);  
            lods[1]->Initialize(X+hw, Y, -scaledRadius, wx, wy, wz, radius, face, this, 1, initialDetail);
            lods[2]->Initialize(X, Y+hw, -scaledRadius, wx, wy, wz, radius, face, this, 2, initialDetail);
            lods[3]->Initialize(X+hw, Y+hw, -scaledRadius, wx, wy, wz, radius, face, this, 3, initialDetail);
            break;
        case P_BOTTOM:
            lods[0]->Initialize(X, -scaledRadius, Z, wx, wy, wz, radius, face, this, 0, initialDetail);  
            lods[1]->Initialize(X+hw, -scaledRadius, Z, wx, wy, wz, radius, face, this, 1, initialDetail);
            lods[2]->Initialize(X, -scaledRadius, Z+hw, wx, wy, wz, radius, face, this, 2, initialDetail);
            lods[3]->Initialize(X+hw, -scaledRadius, Z+hw, wx, wy, wz, radius, face, this, 3, initialDetail);
            break;
    }
}

//returns the update code 0 = no update, 1 = update, 2 = remove children and update
int TerrainPatch::update(int wx, int wy, int wz)
{
    int rv = 0;
    if (lods[0]){
        for (unsigned int i = 0; i < 4; i++){
            if (lods[i]->update(wx, wy, wz)) rv = 1; //if a child needs to change its mesh, we will return 1
        }

        SortChildren(); //sort children for better mesh updates
    }

    double dx, dy, dz;

    int oldDetailLevel = detailLevel;
    int oldStep = step;

    if (hasBoundingBox){ //bounding box distance
        closestPoint.x = (wx <= worldX) ? worldX : ((wx > worldX + boundingBox.x) ? (worldX + boundingBox.x) : wx);
        closestPoint.y = (wy <= worldY) ? worldY : ((wy > worldY + boundingBox.y) ? (worldY + boundingBox.y) : wy);
        closestPoint.z = (wz <= worldZ) ? worldZ : ((wz > worldZ + boundingBox.z) ? (worldZ + boundingBox.z) : wz);

        dx = (double)(closestPoint.x) - wx;
        dy = (double)(closestPoint.y) - wy;
        dz = (double)(closestPoint.z) - wz;
        distance = sqrt(dx*dx + dy*dy + dz*dz);

        if (hasProperDist){
            CalculateDetailLevel(distance, 100);
        }else{
            CalculateDetailLevel(distance, 0);
            hasProperDist = 1;
        }
    }else{ //approximate distance

        closestPoint.x = worldX;
        closestPoint.y = worldY;
        closestPoint.z = worldZ;
        
        dx = (double)(worldX) - wx;
        dy = (double)(worldY) - wy;
        dz = (double)(worldZ) - wz;
        distance = sqrt(dx*dx + dy*dy + dz*dz) - width*0.5*1.4142;
        if (distance < 0) distance = 0;
        CalculateDetailLevel(distance, 0); //shorten the distance by the diagonal radius of an LOD
    }

    if (detailLevel == 0){ //if detail is lowest, we dont need to display a skirt
        hasSkirt = 0;
    }else{
        hasSkirt = 1;
    }

    //if the detail changed, then we need to reload the mesh.
    if (detailLevel != oldDetailLevel || step != oldStep){ 

//        cout << detailLevel << " " << oldDetailLevel << " " << (width / step) << " " << width << " " << step << endl;
        //if there will be too many vertices for this size of LOD, split it into 4 children
        if ((width / step) > maxVertexWidth){
            //if we already have children, we simply return 1 to indicate a needed update.
            if (lods[0]){
                updateCode = 1;
                return 1; //update Children
            }

            CreateChildren(wx, wy, wz, oldDetailLevel);
            updateCode = 1;
            return 1; 
        }else if (lods[0]){  //if we have children but no longer need them, flag for deletion
            updateCode = 2;
            return 2; //remove children
        }
        updateCode = 1;
        return 1;//push onto update list
    } 

    if (rv){
        updateCode = 1;
    }
    return rv;
}

void TerrainPatch::CalculateDetailLevel(double dist, int threshold)
{
    for (int i = 0; i < DetailLevels; i++){
        //check if the distance is inside the range for the different detail levels
        if (dist < lodDistanceLevels[i] - threshold && (i == 0 || dist >= lodDistanceLevels[i-1] + threshold)){
            //set the step for meshing and store the new detail level
            step = lodDetailLevels[i+graphicsOptions.lodDetail];
            while (step > width){
                step /= 2;
                i--;
            }
            detailLevel = i;
            break;
        }
    }
}

void TerrainPatch::SortChildren()
{
    TerrainPatch *temp;
    int j;
    for (unsigned int i = 1; i < 4; i++)
    {
        temp = lods[i];

        for (j = i - 1; (j >= 0) && (temp->distance < lods[j]->distance); j-- )
        {
            lods[j+1] = lods[j];
            lods[j+1]->vecIndex = j+1;
        }

        lods[j+1] = temp;
        lods[j+1]->vecIndex = j+1;
    }
}
