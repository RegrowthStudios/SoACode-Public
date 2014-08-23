#include "stdafx.h"
#include "TerrainPatch.h"

#include "BlockData.h"
#include "Chunk.h"
#include "FloraGenerator.h"
#include "GameManager.h"
#include "Options.h"
#include "Planet.h"
#include "WorldStructs.h"
#include "shader.h"
#include "utils.h"

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
	if (indexSize){
		if (distance < closestTerrainPatchDistance) closestTerrainPatchDistance = distance;
		if (SphereInFrustum((float)(boundingBox.x / 2 - rotPlayerPos.x), (float)(boundingBox.y / 2 - rotPlayerPos.y), (float)(boundingBox.z / 2 - rotPlayerPos.z), (float)cullRadius, worldFrustum)){

			glm::mat4 MVP;

			GlobalModelMatrix[3][0] = ((float)((double)X - PlayerPos.x));
			GlobalModelMatrix[3][1] = ((float)((double)Y - PlayerPos.y));
			GlobalModelMatrix[3][2] = ((float)((double)Z - PlayerPos.z));
			MVP = VP  * GlobalModelMatrix;

			glUniformMatrix4fv(mvpID, 1, GL_FALSE, &MVP[0][0]); //some kind of crash here :/

//			glUniform3f(worldOffsetID, drawX, drawY, drawZ);


			//	glBindBuffer(GL_ARRAY_BUFFER, vboID);
			//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndexID);


			glBindVertexArray(vaoID);
			glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_SHORT, 0);
			glBindVertexArray(0);
		}
	}
	else if (lods[0]){
		lods[0]->Draw(PlayerPos, rotPlayerPos, VP, mvpID, worldOffsetID, onPlanet);
		lods[1]->Draw(PlayerPos, rotPlayerPos, VP, mvpID, worldOffsetID, onPlanet);
		lods[2]->Draw(PlayerPos, rotPlayerPos, VP, mvpID, worldOffsetID, onPlanet);
		lods[3]->Draw(PlayerPos, rotPlayerPos, VP, mvpID, worldOffsetID, onPlanet);
	}
}

void CloseTerrainPatch::DrawTrees(glm::dvec3 &PlayerPos, glm::mat4 &VP)
{
	//if (treeIndexSize){
	//	if (SphereInFrustum((float)(worldX + boundingBox.x / 2 - PlayerPos.x), (float)(worldY + boundingBox.y / 2 - PlayerPos.y), (float)(worldZ + boundingBox.z / 2 - PlayerPos.z), (float)cullRadius)
	//		&& CheckHorizon(PlayerPos)){

	//		GlobalModelMatrix[3][0] = ((float)((double)drawX - PlayerPos.x));
	//		GlobalModelMatrix[3][1] = ((float)((double)drawY - PlayerPos.y));
	//		GlobalModelMatrix[3][2] = ((float)((double)drawZ - PlayerPos.z));

	//		glm::mat4 MVP = VP * GlobalModelMatrix;

	//		glUniformMatrix4fv(treeShader.mvpID, 1, GL_FALSE, &MVP[0][0]);
	//		glUniformMatrix4fv(treeShader.mID, 1, GL_FALSE, &GlobalModelMatrix[0][0]);
	//		//glUniformMatrix4fv(treeShader.mvpID, 1, GL_FALSE, &MVP[0][0]);
	//		//glUniformMatrix4fv(treeShader.gVPID, 1, GL_FALSE, &VP[0][0]);
	//		//glUniform3f(treeShader.upID, worldNormal.x, worldNormal.y, worldNormal.z);

	//		totVertices += treeIndexSize;
	//		totDrawCalls++;

	//		glBindBuffer(GL_ARRAY_BUFFER, treeVboID);

	//		//position
	//		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TreeVertex), 0);
	//		//particle center
	//		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TreeVertex), ((char *)NULL + (8)));
	//		//leaf color and size
	//		glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(TreeVertex), ((char *)NULL + (20)));
	//		//trunk color and ltex
	//		glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(TreeVertex), ((char *)NULL + (24)));

	//		glDrawElements(GL_TRIANGLES, treeIndexSize, GL_UNSIGNED_INT, NULL);
	//	}
	//}
	//else if (lods[0]){
	//	lods[0]->DrawTrees(PlayerPos, VP);
	//	lods[1]->DrawTrees(PlayerPos, VP);
	//	lods[2]->DrawTrees(PlayerPos, VP);
	//	lods[3]->DrawTrees(PlayerPos, VP);
	//}
}

const bool colorDebug = 0;
bool CloseTerrainPatch::CreateMesh()
{
	int minX, maxX, minY, maxY, minZ, maxZ;
	int index = 0;
	int indice = 0;
	bool removeChildren = 0;
	int tincorig;

	//cout << updateCode << endl;
	if (updateCode == 0) return 1; //doesnt need to be meshed

	if (updateCode != 2){ //2 == remove children
		if (lods[0]){ //if it is a parent, load the children
			//		cout << numChildrenLoaded << endl;
			//load children
			for (int i = 0; i < 4; i++){
				if (lods[i]->updateCode != 0){
					lods[i]->CreateMesh();
					return 0;
				}
			}

			updateCode = 0;
			ClearLODMap();
			ClearBuffers();
			return 1;
		}
	}
	else{
		removeChildren = 1;
		updateCode = 1;
	}

	int treen = 0;
	bool genTrees = (step <= 32);

	if (treeVboID != 0){
		glDeleteBuffers(1, &(treeVboID));
		treeVboID = 0;
		treeIndexSize = 0;
	}

	//used for tree coords
	int wx = faceData.jpos*CHUNK_WIDTH * FaceOffsets[faceData.face][faceData.rotation][0];
	int wz = faceData.ipos*CHUNK_WIDTH * FaceOffsets[faceData.face][faceData.rotation][0];
	//swap em if rot%2
	if (faceData.rotation % 2){
		int t = wx;
		wx = wz;
		wz = t;
	}

	int rot = faceData.rotation;
	int face = faceData.face;
	int ipos = FaceCoords[face][rot][0];
	int jpos = FaceCoords[face][rot][1];
	int rpos = FaceCoords[face][rot][2];
	int idir = FaceOffsets[face][rot][0];
	int jdir = FaceOffsets[face][rot][1];

	currTerrainGenerator->SetLODFace(ipos, jpos, rpos, FaceRadSign[face] * GameManager::planet->radius, idir, jdir, 1.0);

	int size = width / step + 3;
	int h;
	int temperature, rainfall;
	if (lodMap == NULL) lodMap = new HeightData[(maxVertexWidth + 3)*(maxVertexWidth + 3)];
	lodMapStep = step; //for our future children to use
	lodMapSize = size;
	bool happened = 0;
	bool hasWater = 0;

	if (step <= 16){
		tincorig = 1;
	}
	else{
		tincorig = 2;
	}

	int irel = faceData.ipos*CHUNK_WIDTH - GameManager::planet->radius;
	int jrel = faceData.jpos*CHUNK_WIDTH - GameManager::planet->radius;
	currTerrainGenerator->GenerateHeightMap(lodMap, irel, jrel, size, size, size, step, 0, &hasWater);
	
	//**************************************** WATER *********************************************
	int isize = size - 2;
	float texInc = step / 1024.0;
	double uStart = texInc*((jrel + step) / step);
	double vStart = texInc*((irel + step) / step);
	if (hasWater){

		int sindex = index;

		//9530 1,227,270
		for (int z = 1; z < size - 1; z++){ //Z
			for (int x = 1; x < size - 1; x++){ //X
				h = (int)lodMap[z*size + x].height;
				//check and see if all adjacent heights are above water. If so, then we dont need this water vertex at all. TODO: maybe sentinalize this?
				if (h >= 0 && (x == 1 || lodMap[z*size + (x - 1)].height >= 0) //left
					&& (z == 1 || lodMap[(z - 1)*size + x].height >= 0) //back
					&& ((x == 1 && z == 1) || lodMap[(z - 1)*size + (x - 1)].height >= 0) //left back
					&& (x == size - 2 || lodMap[z*size + (x + 1)].height >= 0) //right
					&& (z == size - 2 || lodMap[(z + 1)*size + x].height >= 0) //front
					&& ((x == size - 2 && z == 1) || lodMap[(z - 1)*size + (x + 1)].height >= 0) //right back
					&& ((x == 1 && z == size - 2) || lodMap[(z + 1)*size + (x - 1)].height >= 0) //left front
					&& ((x == size - 2 && z == size - 2) || lodMap[(z + 1)*size + (x + 1)].height >= 0)) { //right front

					MakeWaterQuadMap[(z - 1)*isize + (x - 1)] = 0; //mark vertex as not being a quad start
					//now we have to check that other quads dont need to use this quad.
					if ((x == 1 || MakeWaterQuadMap[(z - 1)*isize + (x - 2)] == 0) && (z == 1 || MakeWaterQuadMap[(z - 2)*isize + (x - 1)] == 0)
						&& ((x == 1 && z == 1) || MakeWaterQuadMap[(z - 2)*isize + (x - 2)] == 0)){
						continue; //skip this vertex since its useless. 
					}
				}
				else{
					MakeWaterQuadMap[(z - 1)*isize + (x - 1)] = 1;
				}

				temperature = lodMap[z*size + x].temperature;
				rainfall = lodMap[z*size + x].rainfall;

				int depth = -h / 5;
				if (depth > 255){
					depth = 255;
				}
				else if (depth < 0) {
					depth = 0;
				}

				tvboVerts[index].location[0] = (GLfloat)jrel + x*step;
				tvboVerts[index].location[1] = (GLfloat)0;
				tvboVerts[index].location[2] = (GLfloat)irel + z*step;
				tvboVerts[index].color[0] = 255;
				tvboVerts[index].color[1] = 255;
				tvboVerts[index].color[2] = 255;
				tvboVerts[index].tex[0] = uStart + texInc*(x - 1);
				tvboVerts[index].tex[1] = vStart + texInc*(z - 1);
				tvboVerts[index].textureUnit = 1;

				tvboVerts[index].temperature = (GLubyte)temperature;
				tvboVerts[index].rainfall = 255 - (GLubyte)depth; //water needs depth
				tvboVerts[index].specular = 255;

				WaterIndexMap[(z - 1)*isize + (x - 1)] = index;
				index++;
			}
		}

		int isize = size - 2;
		int istart;
		for (int z = 0; z < isize - 1; z++){
			for (int x = 0; x < isize - 1; x++){
				if (MakeWaterQuadMap[z*isize + x]){ //check for skipped quad
					istart = WaterIndexMap[z*isize + x];
					lodIndices[indice] = istart;
					lodIndices[indice + 1] = WaterIndexMap[(z + 1)*isize + x];
					lodIndices[indice + 2] = WaterIndexMap[(z + 1)*isize + (x + 1)];
					lodIndices[indice + 3] = WaterIndexMap[(z + 1)*isize + (x + 1)];
					lodIndices[indice + 4] = WaterIndexMap[z*isize + (x + 1)];
					lodIndices[indice + 5] = istart;
					indice += 6;
				}
			}
		}
	}


	int tStartIndex = index;
	texInc = 2.0f / (size - 2);
	int tinc;
	int snowDepth, sandDepth, flags;
	Biome *biome;
	int tx, tz, ltex;
	glm::vec3 v1;
	Block *bp;
	int txc, tzc, treeI, treeIndex, th;
	int colr, colg, colb;

	//*************************************** TERRAIN ********************************************
	
	for (int i = 1; i < size - 1; i++){ //Z
		for (int j = 1; j < size - 1; j++){ //X
			temperature = lodMap[i*size + j].temperature;
			rainfall = lodMap[i*size + j].rainfall;
			flags = lodMap[i*size + j].flags;
			h = lodMap[i*size + j].height * invPlanetScale;
			snowDepth = (int)(lodMap[i*size + j].snowDepth * invPlanetScale);
			sandDepth = (int)(lodMap[i*size + j].sandDepth * invPlanetScale);
			biome = lodMap[i*size + j].biome;

			if (biome->treeChance > 0.08){
				tinc = 2;
			}
			else if (biome->treeChance < 0.015){
				tinc = 1;
			}
			else{
				tinc = tincorig;
			}
			tinc = 1 * invPlanetScale;

			TreeData treeData;

			if (genTrees && snowDepth < 7 * invPlanetScale){
				for (float z1 = 0; z1 < step; z1 += tinc){
					for (float x1 = 0; x1 < step; x1 += tinc){
						if (h > 0){
							tx = jrel + GameManager::planet->radius + j*step + x1; tz = irel + GameManager::planet->radius + i*step + z1; //gotta add radius since irel and jrel are -rad to rad while trees are 0-2rad

							if (tx < 0){
								txc = 31 + tx%CHUNK_WIDTH;
							}
							else{
								txc = tx%CHUNK_WIDTH;
							}
							if (tz < 0){
								tzc = 31 + tz%CHUNK_WIDTH;
							}
							else{
								tzc = tz%CHUNK_WIDTH;
							}

							//20 fps
							if (txc && tzc){
								treeI = FloraGenerator::getTreeIndex(biome, tx, tz);
								if (treeI != -1){
                                    if (!FloraGenerator::makeLODTreeData(treeData, GameManager::planet->treeTypeVec[treeI], txc, tzc, tx - txc, tz - tzc)){
										if (treeIndex >= TREE_VERTS_SIZE) exit(1);

										th = BilinearInterpolation(lodMap[i*size + j].height, lodMap[i*size + j + 1].height, lodMap[(i + 1)*size + j].height, lodMap[(i + 1)*size + j + 1].height, step, z1, x1) * invPlanetScale;
										if (th <= 0) continue;

										float size = treeData.treeHeight * invPlanetScale;
										if (size < 1.0f) size = 1.0f;

										v1.x = (double)irel + (double)(i*step) + x1;
										v1.z = (double)jrel + (double)(j*step) + z1;
										v1.y = (double)th + (double)snowDepth + (double)sandDepth;
									
										if (treeData.leafColor && GETBLOCK((treeData.treeType->idLeaves)).altColors.size() > treeData.leafColor - 1){
											colr = Blocks[treeData.treeType->idLeaves].altColors[treeData.leafColor - 1].r;
											colg = Blocks[treeData.treeType->idLeaves].altColors[treeData.leafColor - 1].g;
											colb = Blocks[treeData.treeType->idLeaves].altColors[treeData.leafColor - 1].b;
										}
										else {
											colr = Blocks[treeData.treeType->idLeaves].color[0];
											colg = Blocks[treeData.treeType->idLeaves].color[1];
											colb = Blocks[treeData.treeType->idLeaves].color[2];
										}

										switch (treeData.treeType->leafCapShape){
										case 0:
											ltex = 0; //none
											break;
										case 3:
											ltex = 2; //pine
											break;
										case 4:
											ltex = 3; //mushroom
											break;
										default:
											ltex = 1; //standard
											break;
										}


										treeVerts[treeIndex].pos = glm::vec2(0.5f, 1.0f);
										treeVerts[treeIndex].lr = colr;
										treeVerts[treeIndex].lg = colg;
										treeVerts[treeIndex].lb = colb;
										treeVerts[treeIndex].size = size;
										treeVerts[treeIndex].center = glm::vec3(v1);
										treeVerts[treeIndex].ltex = ltex;
										treeIndex++;
										treeVerts[treeIndex].pos = glm::vec2(-0.5f, 1.0f);
										treeVerts[treeIndex].lr = colr;
										treeVerts[treeIndex].lg = colg;
										treeVerts[treeIndex].lb = colb;
										treeVerts[treeIndex].size = size;
										treeVerts[treeIndex].center = glm::vec3(v1);
										treeVerts[treeIndex].ltex = ltex;
										treeIndex++;
										treeVerts[treeIndex].pos = glm::vec2(-0.5f, 0.0f);
										treeVerts[treeIndex].lr = colr;
										treeVerts[treeIndex].lg = colg;
										treeVerts[treeIndex].lb = colb;
										treeVerts[treeIndex].size = size;
										treeVerts[treeIndex].center = glm::vec3(v1);
										treeVerts[treeIndex].ltex = ltex;
										treeIndex++;
										treeVerts[treeIndex].pos = glm::vec2(0.5f, 0.0f);
										treeVerts[treeIndex].lr = colr;
										treeVerts[treeIndex].lg = colg;
										treeVerts[treeIndex].lb = colb;
										treeVerts[treeIndex].size = size;
										treeVerts[treeIndex].center = glm::vec3(v1);
										treeVerts[treeIndex].ltex = ltex;
										treeIndex++;

									}
								}
							}
						}
					}
				}
			}

			v1.z = (double)irel + (double)(i*step);
			v1.x = (double)jrel + (double)(j*step);
			v1.y = (double)th + (double)snowDepth + (double)sandDepth;

			minX = MIN(minX, v1.x);
			maxX = MAX(maxX, v1.x);
			minY = MIN(minY, v1.y);
			maxY = MAX(maxY, v1.y);
			minZ = MIN(minZ, v1.z);
			maxZ = MAX(maxZ, v1.z);

			//	vx = vx*dh - (double)X;
			//	vy = vy*dh - (double)(radius + Y);
			//	vz = vz*dh - (double)Z;


			//cout << vx << " " << vy << " " << vz << endl;

			tvboVerts[index].location[0] = (GLfloat)v1.x;
			tvboVerts[index].location[1] = (GLfloat)v1.y;
			tvboVerts[index].location[2] = (GLfloat)v1.z;
			tvboVerts[index].tex[0] = texInc*(j - 1);
			tvboVerts[index].tex[1] = texInc*(i - 1);
			tvboVerts[index].textureUnit = 0;
			tvboVerts[index].temperature = (GLubyte)temperature;
			tvboVerts[index].rainfall = (GLubyte)rainfall;
			tvboVerts[index].specular = 32;

			tvboVerts[index].normal = glm::normalize(glm::vec3(0.0, 1.0, 0.0)); //normalize is optional?

			//slope color
			bp = &(Blocks[biome->surfaceLayers[biome->looseSoilDepth + 1]]);
			tvboVerts[index].slopeColor[0] = bp->averageColor[0];
            tvboVerts[index].slopeColor[1] = bp->averageColor[1];
            tvboVerts[index].slopeColor[2] = bp->averageColor[2];

			//beach color
			bp = &(Blocks[biome->beachBlock]);
            tvboVerts[index].beachColor[0] = bp->averageColor[0];
            tvboVerts[index].beachColor[1] = bp->averageColor[1];
            tvboVerts[index].beachColor[2] = bp->averageColor[2];

			float yn = tvboVerts[index].normal.y;

			if (0 && h <= 0 && biome->beachBlock == SAND){
				colr = 194;
				colg = 178;
				colb = 128;
			}
			else if (flags & VOLCANO){
				colr = colg = colb = 60;
			}
			else{
				colr = colg = colb = 255;
			}
			if (biome->hasAltColor == 2){
				colr = biome->r;
				colg = biome->g;
				colb = biome->b;
			}
			else if (snowDepth || biome->surfaceBlock == SNOW){
				colr = 255;
				colg = 255;
				colb = 255;
			}
			else if (sandDepth || biome->surfaceBlock == SAND){
				colr = 233;
				colg = 207;
				colb = 169;
			}
			else if (biome->hasAltColor){
				colr = biome->r;
				colg = biome->g;
				colb = biome->b;
			}

			if (colorDebug == 1){
				colr = detailLevel % 4 * 64;
				colg = detailLevel % 2 * 128;
				colb = detailLevel % 8 * 32;
			}

			tvboVerts[index].color[0] = colr;
			tvboVerts[index].color[1] = colg;
			tvboVerts[index].color[2] = colb;


			index++;
		}
	}

	int c;
	for (int z = 0; z < isize - 1; z++){
		for (int x = 0; x < isize - 1; x++){ //Z
			c = tStartIndex + z * isize + x;
			if (c % 2){
				lodIndices[indice] = c;
				lodIndices[indice + 1] = c + isize;
				lodIndices[indice + 2] = c + 1;
				lodIndices[indice + 3] = c + 1;
				lodIndices[indice + 4] = c + isize;
				lodIndices[indice + 5] = c + isize + 1;
			}
			else{
				lodIndices[indice] = c;
				lodIndices[indice + 1] = c + isize;
				lodIndices[indice + 2] = c + isize + 1;
				lodIndices[indice + 3] = c + isize + 1;
				lodIndices[indice + 4] = c + 1;
				lodIndices[indice + 5] = c;
			}
			indice += 6;
		}
	}

	boundingBox.x = maxX - minX;
	boundingBox.y = maxY - minY;
	boundingBox.z = maxZ - minZ;
	hasBoundingBox = 1;
	cullRadius = MAX(boundingBox.x, boundingBox.y);
	cullRadius = MAX(cullRadius, boundingBox.z);

	updateCode = 0;

	if (indice > lodIndices.size()){
		pError("Index out of bounds for lodIndices");
		exit(15);
	}
	else if (index > tvboVerts.size()){
		pError(("Index out of bounds for tvboVerts. " + to_string(index) + " / " + to_string(tvboVerts.size())).c_str());
		exit(15);
	}

	if (vaoID == 0) glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);

	if (vboID == 0) glGenBuffers(1, &(vboID));
	glBindBuffer(GL_ARRAY_BUFFER, vboID); // Bind the buffer (vertex array data)
	glBufferData(GL_ARRAY_BUFFER, index * sizeof(TerrainVertex), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, index * sizeof(TerrainVertex), &(tvboVerts[0].location)); //crash

	if (vboIndexID == 0) glGenBuffers(1, &(vboIndexID));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndexID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indice * sizeof(GLushort), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indice * sizeof(GLushort), &(lodIndices[0]));

	//vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), 0);
	//UVs
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), ((char *)NULL + (12)));
	//normals
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), ((char *)NULL + (20)));
	//colors
	glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(TerrainVertex), ((char *)NULL + (32)));
	//slope color
	glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(TerrainVertex), ((char *)NULL + (36)));
	//beach color
	glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(TerrainVertex), ((char *)NULL + (40)));
	//texureUnit, temperature, rainfall, specular
	glVertexAttribPointer(6, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(TerrainVertex), ((char *)NULL + (44)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
	glEnableVertexAttribArray(6);
	glBindVertexArray(0); // Disable our Vertex Buffer Object  


	if (treeIndex){
		treeIndexSize = treeIndex * 6 / 4;
		glGenBuffers(1, &(treeVboID));
		glBindBuffer(GL_ARRAY_BUFFER, treeVboID); // Bind the buffer (vertex array data)
		glBufferData(GL_ARRAY_BUFFER, treeIndex * sizeof(TreeVertex), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, treeIndex * sizeof(TreeVertex), treeVerts);
	}
	else{
		ClearTreeBuffers();
	}

	hasMesh = 1;
	indexSize = indice;

	waitForMesh = 0;

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

		//		cout << detailLevel << " " << oldDetailLevel << " " << (width / step) << " " << width << " " << step << endl;
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
