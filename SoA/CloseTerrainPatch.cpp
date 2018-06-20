#include "stdafx.h"
#include "TerrainPatch.h"

#include <Vorb/utils.h>

#include "BlockData.h"
#include "Chunk.h"
#include "FloraGenerator.h"
#include "GameManager.h"
//#include "Options.h"
#include "Planet.h"
#include "WorldStructs.h"

inline double BilinearInterpolation(int &a, int &b, int &c, int &d, int &step, float &x, float &z)
{
    double px, pz;
    px = ((double)(x)) / step;
    pz = ((double)(z)) / step;

    return  (a)*(1 - px)*(1 - pz) +
        (b)*px*(1 - pz) +
        (c)*(1 - px)*pz +
        (d)*px*pz;
}

CloseTerrainPatch::CloseTerrainPatch(int lodWidth){
    for (int i = 0; i < 4; i++){
        lods[i] = NULL;
    }
    width = lodWidth;
    vboID = 0;
    vaoID = 0;
    vboIndexID = 0;
    updateVecIndex = -1;
    vecIndex = -1;
    treeVboID = 0;
    treeIndexSize = 0;
    lodMap = NULL;
}

CloseTerrainPatch::~CloseTerrainPatch(){
    ClearLODMap();
    ClearBuffers();
    DeleteChildren();
}

void CloseTerrainPatch::ClearLODMap()
{
    if (lodMap != NULL){
        delete[] lodMap;
        lodMap = NULL;
    }
}

void CloseTerrainPatch::ClearBuffers()
{
    indexSize = 0;
    if (vboID != 0){
        glDeleteVertexArrays(1, &vaoID);
        glDeleteBuffers(1, &vboID);
        vaoID = 0;
        vboID = 0;
    }
    if (vboIndexID != 0){
        glDeleteBuffers(1, &vboIndexID);
        vboIndexID = 0;
    }
    ClearTreeBuffers();
}

void CloseTerrainPatch::ClearTreeBuffers()
{
    treeIndexSize = 0;
    if (treeVboID != 0){
        glDeleteBuffers(1, &treeVboID);
        treeVboID = 0;
    }
}

void CloseTerrainPatch::DeleteChildren()
{
    if (lods[0]){
        for (unsigned int i = 0; i < 4; i++){
            lods[i]->ClearBuffers();
            lods[i]->DeleteChildren();
            delete lods[i]; //calls delete children
            lods[i] = NULL;
        }
    }
}

//itinializes the LOD and computes distance
void CloseTerrainPatch::Initialize(int x, int y, int z, int wx, int wy, int wz, CloseTerrainPatch *Parent, int ChildNum, int initialDetailLevel)
{
    childNum = ChildNum;
    waitForMesh = 0;
    vboID = 0;
    vaoID = 0;
    treeVboID = 0;
    drawTrees = 0;
    lodMap = NULL;
    vboIndexID = 0;
    for (unsigned int i = 0; i < 4; i++){
        lods[i] = NULL;
    }
    indexSize = 0;
    treeIndexSize = 0;
    parent = Parent;
    updateCode = 1;
    vecIndex = -1;
    updateVecIndex = -1;
    hasSkirt = 1;
    hasBoundingBox = 0;
    cullRadius = 0;
    hasProperDist = 0;

    X = x;
    Y = y;
    Z = z;

    double vx = X + width / 2;
    double vy = Y + width / 2;
    double vz = Z + width / 2;

    //double tmph = currTerrainGenerator->GenerateSingleHeight(vx, vy, vz);

    double magnitude = sqrt(vx*vx + vy*vy + vz*vz);
    vx /= magnitude;
    vy /= magnitude;
    vz /= magnitude;

    closestPoint.x = X + width/2;
    closestPoint.y = Y + width / 2;
    closestPoint.z = Z + width / 2;

    double dx = (double)(X - wx);
    double dy = (double)(Y - wy);
    double dz = (double)(Z - wz);
    //approximate distance, not true distance until mesh has been made initially
    distance = sqrt(dx*dx + dy*dy + dz*dz) - width*0.5*1.4142;
    if (distance < 0) distance = 0;

    hasMesh = 0;
    if (initialDetailLevel != -1){
        detailLevel = initialDetailLevel;
        step = lodDetailLevels[detailLevel + graphicsOptions.lodDetail];
        waitForMesh = 1;
    }
    else{
        CalculateDetailLevel(distance, 0);
    }

    if (detailLevel == 0) hasSkirt = 0;
    if ((width / step) > maxVertexWidth){
        CreateChildren(wx, wy, wz);
    }
}

void CloseTerrainPatch::Draw(glm::dvec3 &PlayerPos, glm::dvec3 &rotPlayerPos, glm::mat4 &VP, GLuint mvpID, GLuint worldOffsetID, bool onPlanet)
{
//    if (indexSize){
//        if (distance < closestTerrainPatchDistance) closestTerrainPatchDistance = distance;
//        if (SphereInFrustum((float)(boundingBox.x / 2 - rotPlayerPos.x), (float)(boundingBox.y / 2 - rotPlayerPos.y), (float)(boundingBox.z / 2 - rotPlayerPos.z), (float)cullRadius, worldFrustum)){
//
//            glm::mat4 MVP;
//
//            GlobalModelMatrix[3][0] = ((float)((double)X - PlayerPos.x));
//            GlobalModelMatrix[3][1] = ((float)((double)Y - PlayerPos.y));
//            GlobalModelMatrix[3][2] = ((float)((double)Z - PlayerPos.z));
//            MVP = VP  * GlobalModelMatrix;
//
//            glUniformMatrix4fv(mvpID, 1, GL_FALSE, &MVP[0][0]); //some kind of crash here :/
//
////            glUniform3f(worldOffsetID, drawX, drawY, drawZ);
//
//
//            //    glBindBuffer(GL_ARRAY_BUFFER, vboID);
//            //    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndexID);
//
//
//            glBindVertexArray(vaoID);
//            glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_SHORT, 0);
//            glBindVertexArray(0);
//        }
//    }
//    else if (lods[0]){
//        lods[0]->Draw(PlayerPos, rotPlayerPos, VP, mvpID, worldOffsetID, onPlanet);
//        lods[1]->Draw(PlayerPos, rotPlayerPos, VP, mvpID, worldOffsetID, onPlanet);
//        lods[2]->Draw(PlayerPos, rotPlayerPos, VP, mvpID, worldOffsetID, onPlanet);
//        lods[3]->Draw(PlayerPos, rotPlayerPos, VP, mvpID, worldOffsetID, onPlanet);
//    }
}

void CloseTerrainPatch::DrawTrees(glm::dvec3 &PlayerPos, glm::mat4 &VP)
{
    //if (treeIndexSize){
    //    if (SphereInFrustum((float)(worldX + boundingBox.x / 2 - PlayerPos.x), (float)(worldY + boundingBox.y / 2 - PlayerPos.y), (float)(worldZ + boundingBox.z / 2 - PlayerPos.z), (float)cullRadius)
    //        && CheckHorizon(PlayerPos)){

    //        GlobalModelMatrix[3][0] = ((float)((double)drawX - PlayerPos.x));
    //        GlobalModelMatrix[3][1] = ((float)((double)drawY - PlayerPos.y));
    //        GlobalModelMatrix[3][2] = ((float)((double)drawZ - PlayerPos.z));

    //        glm::mat4 MVP = VP * GlobalModelMatrix;

    //        glUniformMatrix4fv(treeShader.mvpID, 1, GL_FALSE, &MVP[0][0]);
    //        glUniformMatrix4fv(treeShader.mID, 1, GL_FALSE, &GlobalModelMatrix[0][0]);
    //        //glUniformMatrix4fv(treeShader.mvpID, 1, GL_FALSE, &MVP[0][0]);
    //        //glUniformMatrix4fv(treeShader.gVPID, 1, GL_FALSE, &VP[0][0]);
    //        //glUniform3f(treeShader.upID, worldNormal.x, worldNormal.y, worldNormal.z);

    //        totVertices += treeIndexSize;
    //        totDrawCalls++;

    //        glBindBuffer(GL_ARRAY_BUFFER, treeVboID);

    //        //position
    //        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TreeVertex), 0);
    //        //particle center
    //        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TreeVertex), ((char *)NULL + (8)));
    //        //leaf color and size
    //        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(TreeVertex), ((char *)NULL + (20)));
    //        //trunk color and ltex
    //        glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(TreeVertex), ((char *)NULL + (24)));

    //        glDrawElements(GL_TRIANGLES, treeIndexSize, GL_UNSIGNED_INT, NULL);
    //    }
    //}
    //else if (lods[0]){
    //    lods[0]->DrawTrees(PlayerPos, VP);
    //    lods[1]->DrawTrees(PlayerPos, VP);
    //    lods[2]->DrawTrees(PlayerPos, VP);
    //    lods[3]->DrawTrees(PlayerPos, VP);
    //}
}

const bool colorDebug = 0;
bool CloseTerrainPatch::CreateMesh()
{
    

    return 1;
}

void CloseTerrainPatch::CreateChildren(int wx, int wy, int wz, int initialDetail)
{
    int hw = width / 2;

    lods[0] = new CloseTerrainPatch(hw);
    lods[1] = new CloseTerrainPatch(hw);
    lods[2] = new CloseTerrainPatch(hw);
    lods[3] = new CloseTerrainPatch(hw);

    lods[0]->Initialize(X, 0, Z, wx, wy, wz, this, 0, initialDetail);
    lods[1]->Initialize(X + hw, 0, Z, wx, wy, wz, this, 1, initialDetail);
    lods[2]->Initialize(X, 0, Z + hw, wx, wy, wz, this, 2, initialDetail);
    lods[3]->Initialize(X + hw, 0, Z + hw, wx, wy, wz, this, 3, initialDetail);

    /*switch (face){
    case P_TOP:
        lods[0]->Initialize(X, scaledRadius, Z, wx, wy, wz, radius, face, this, 0, initialDetail);
        lods[1]->Initialize(X + hw, scaledRadius, Z, wx, wy, wz, radius, face, this, 1, initialDetail);
        lods[2]->Initialize(X, scaledRadius, Z + hw, wx, wy, wz, radius, face, this, 2, initialDetail);
        lods[3]->Initialize(X + hw, scaledRadius, Z + hw, wx, wy, wz, radius, face, this, 3, initialDetail);
        break;
    case P_LEFT:
        lods[0]->Initialize(-scaledRadius, Y, Z, wx, wy, wz, radius, face, this, 0, initialDetail);
        lods[1]->Initialize(-scaledRadius, Y, Z + hw, wx, wy, wz, radius, face, this, 1, initialDetail);
        lods[2]->Initialize(-scaledRadius, Y + hw, Z, wx, wy, wz, radius, face, this, 2, initialDetail);
        lods[3]->Initialize(-scaledRadius, Y + hw, Z + hw, wx, wy, wz, radius, face, this, 3, initialDetail);
        break;
    case P_RIGHT:
        lods[0]->Initialize(scaledRadius, Y, Z, wx, wy, wz, radius, face, this, 0, initialDetail);
        lods[1]->Initialize(scaledRadius, Y, Z + hw, wx, wy, wz, radius, face, this, 1, initialDetail);
        lods[2]->Initialize(scaledRadius, Y + hw, Z, wx, wy, wz, radius, face, this, 2, initialDetail);
        lods[3]->Initialize(scaledRadius, Y + hw, Z + hw, wx, wy, wz, radius, face, this, 3, initialDetail);
        break;
    case P_FRONT:
        lods[0]->Initialize(X, Y, scaledRadius, wx, wy, wz, radius, face, this, 0, initialDetail);
        lods[1]->Initialize(X + hw, Y, scaledRadius, wx, wy, wz, radius, face, this, 1, initialDetail);
        lods[2]->Initialize(X, Y + hw, scaledRadius, wx, wy, wz, radius, face, this, 2, initialDetail);
        lods[3]->Initialize(X + hw, Y + hw, scaledRadius, wx, wy, wz, radius, face, this, 3, initialDetail);
        break;
    case P_BACK:
        lods[0]->Initialize(X, Y, -scaledRadius, wx, wy, wz, radius, face, this, 0, initialDetail);
        lods[1]->Initialize(X + hw, Y, -scaledRadius, wx, wy, wz, radius, face, this, 1, initialDetail);
        lods[2]->Initialize(X, Y + hw, -scaledRadius, wx, wy, wz, radius, face, this, 2, initialDetail);
        lods[3]->Initialize(X + hw, Y + hw, -scaledRadius, wx, wy, wz, radius, face, this, 3, initialDetail);
        break;
    case P_BOTTOM:
        lods[0]->Initialize(X, -scaledRadius, Z, wx, wy, wz, radius, face, this, 0, initialDetail);
        lods[1]->Initialize(X + hw, -scaledRadius, Z, wx, wy, wz, radius, face, this, 1, initialDetail);
        lods[2]->Initialize(X, -scaledRadius, Z + hw, wx, wy, wz, radius, face, this, 2, initialDetail);
        lods[3]->Initialize(X + hw, -scaledRadius, Z + hw, wx, wy, wz, radius, face, this, 3, initialDetail);
        break;
    }*/
}

//returns the update code 0 = no update, 1 = update, 2 = remove children and update
int CloseTerrainPatch::update(int wx, int wy, int wz)
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
        closestPoint.x = (wx <= X) ? X : ((wx > X + boundingBox.x) ? (X + boundingBox.x) : wx);
        closestPoint.y = (wy <= Y) ? Y : ((wy > Y + boundingBox.y) ? (Y + boundingBox.y) : wy);
        closestPoint.z = (wz <= Z) ? Z : ((wz > Z + boundingBox.z) ? (Z + boundingBox.z) : wz);

        dx = (double)(closestPoint.x) - wx;
        dy = (double)(closestPoint.y) - wy;
        dz = (double)(closestPoint.z) - wz;
        distance = sqrt(dx*dx + dy*dy + dz*dz);

        if (hasProperDist){
            CalculateDetailLevel(distance, 100);
        }
        else{
            CalculateDetailLevel(distance, 0);
            hasProperDist = 1;
        }
    }
    else{ //approximate distance

        closestPoint.x = X;
        closestPoint.y = Y;
        closestPoint.z = Z;

        dx = (double)(X)-wx;
        dy = (double)(Y)-wy;
        dz = (double)(Z)-wz;
        distance = sqrt(dx*dx + dy*dy + dz*dz) - width*0.5*1.4142;
        if (distance < 0) distance = 0;
        if (waitForMesh == 0) CalculateDetailLevel(distance, 0); //shorten the distance by the diagonal radius of an LOD
    }

    if (detailLevel == 0){ //if detail is lowest, we dont need to display a skirt
        hasSkirt = 0;
    }
    else{
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
        }
        else if (lods[0]){  //if we have children but no longer need them, flag for deletion
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

void CloseTerrainPatch::CalculateDetailLevel(double dist, int threshold)
{
    for (int i = 0; i < DetailLevels; i++){
        //check if the distance is inside the range for the different detail levels
        if (dist < lodDistanceLevels[i] - threshold && (i == 0 || dist >= lodDistanceLevels[i - 1] + threshold)){
            //set the step for meshing and store the new detail level
            step = lodDetailLevels[i + graphicsOptions.lodDetail];
            detailLevel = i;
            break;
        }
    }
}

void CloseTerrainPatch::SortChildren()
{
    CloseTerrainPatch *temp;
    int j;
    for (unsigned int i = 1; i < 4; i++)
    {
        temp = lods[i];

        for (j = i - 1; (j >= 0) && (temp->distance < lods[j]->distance); j--)
        {
            lods[j + 1] = lods[j];
            lods[j + 1]->vecIndex = j + 1;
        }

        lods[j + 1] = temp;
        lods[j + 1]->vecIndex = j + 1;
    }
}
