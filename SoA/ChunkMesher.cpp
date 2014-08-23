#include "stdafx.h"
#include "ChunkMesher.h"

#include <random>

#include "BlockData.h"
#include "Chunk.h"
#include "Errors.h"
#include "GameManager.h"
#include "Options.h"
#include "Planet.h"
#include "RenderTask.h"
#include "TerrainGenerator.h"
#include "ThreadPool.h"
#include "utils.h"
#include "VoxelUtils.h"

ChunkMesher::ChunkMesher()
{

	chunkMeshData = NULL;

}

ChunkMesher::~ChunkMesher()
{

}

void ChunkMesher::bindVBOIndicesID()
{
	vector<GLuint> indices;
	indices.resize(589824);

	int j = 0;
	for (Uint32 i = 0; i < indices.size()-12; i += 6){
		indices[i] = j;
		indices[i+1] = j+1;
		indices[i+2] = j+2;
		indices[i+3] = j+2;
		indices[i+4] = j+3;
		indices[i+5] = j;
		j += 4;
	}

	if (Chunk::vboIndicesID != 0){
		glDeleteBuffers(1, &(Chunk::vboIndicesID));
	}
	glGenBuffers(1, &(Chunk::vboIndicesID));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (Chunk::vboIndicesID));
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 500000 * sizeof(GLuint), NULL, GL_STATIC_DRAW);
		
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, 500000 * sizeof(GLuint), &(indices[0])); //arbitrarily set to 300000
}

#define CompareVertices(v1, v2) (v1.color[0] == v2.color[0] && v1.sunLight == v2.sunLight && v1.light == v2.light && v1.color[1] == v2.color[1] && v1.color[2] == v2.color[2] \
    && v1.overlayColor[0] == v2.overlayColor[0] && v1.overlayColor[1] == v1.overlayColor[1] && v2.overlayColor[2] == v2.overlayColor[2] \
    && v1.textureAtlas == v2.textureAtlas && v1.textureIndex == v2.textureIndex && v1.overlayTextureAtlas == v2.overlayTextureAtlas && v1.overlayTextureIndex == v2.overlayTextureIndex)

#define CompareVerticesLight(v1, v2) (v1.sunLight == v2.sunLight && v1.light == v2.light && v1.color[0] == v2.color[0] && v1.color[1] == v2.color[1] && v1.color[2] == v2.color[2])

inline void makeFloraFace(BlockVertex *Verts, const ui8* positions, const i8* normals, int vertexOffset, int waveEffect, i32v3& pos, int vertexIndex, int textureIndex, int overlayTextureIndex, const GLubyte color[], const GLubyte overlayColor[], const GLubyte light[2], const BlockTexture& texInfo)
{

    //get the face index so we can determine the axis alignment
    int faceIndex = vertexOffset / 12;
    //Multiply the axis by the sign bit to get the correct offset
    GLubyte uOffset = (GLubyte)(pos[cubeFaceAxis[faceIndex][0]] * cubeFaceAxisSign[faceIndex][0]);
    GLubyte vOffset = (GLubyte)(pos[cubeFaceAxis[faceIndex][1]] * cubeFaceAxisSign[faceIndex][1]);

    // 7 per coord
    pos.x *= 7;
    pos.y *= 7;
    pos.z *= 7;

    //Blend type. The 6 LSBs are used to encode alpha blending, add/subtract, and multiplication factors.
    //They are used in the shader to determine how to blend.
    ubyte blendMode = 0x25; //0x25 = 00 10 01 01
    switch (texInfo.blendMode) {
    case BlendType::BLEND_TYPE_REPLACE:
        blendMode++; //Sets blendType to 00 01 01 10
        break;
    case BlendType::BLEND_TYPE_ADD:
        blendMode += 4; //Sets blendType to 00 01 10 01
        break;
    case BlendType::BLEND_TYPE_SUBTRACT:
        blendMode -= 4; //Sets blendType to 00 01 00 01
        break;
    case BlendType::BLEND_TYPE_MULTIPLY:
        blendMode -= 16; //Sets blendType to 00 01 01 01
        break;
    }
    Verts[vertexIndex].blendMode = blendMode;
    Verts[vertexIndex + 1].blendMode = blendMode;
    Verts[vertexIndex + 2].blendMode = blendMode;
    Verts[vertexIndex + 3].blendMode = blendMode;

    GLubyte texAtlas = (GLubyte)(textureIndex / 256);
    textureIndex %= 256;

    GLubyte overlayTexAtlas = (GLubyte)(overlayTextureIndex / 256);
    GLubyte overlayTex = (GLubyte)(overlayTextureIndex % 256);

    Verts[vertexIndex].textureWidth = (ubyte)texInfo.base.size.x;
    Verts[vertexIndex].textureHeight = (ubyte)texInfo.base.size.y;
    Verts[vertexIndex + 1].textureWidth = (ubyte)texInfo.base.size.x;
    Verts[vertexIndex + 1].textureHeight = (ubyte)texInfo.base.size.y;
    Verts[vertexIndex + 2].textureWidth = (ubyte)texInfo.base.size.x;
    Verts[vertexIndex + 2].textureHeight = (ubyte)texInfo.base.size.y;
    Verts[vertexIndex + 3].textureWidth = (ubyte)texInfo.base.size.x;
    Verts[vertexIndex + 3].textureHeight = (ubyte)texInfo.base.size.y;

    Verts[vertexIndex].overlayTextureWidth = (ubyte)texInfo.overlay.size.x;
    Verts[vertexIndex].overlayTextureHeight = (ubyte)texInfo.overlay.size.y;
    Verts[vertexIndex + 1].overlayTextureWidth = (ubyte)texInfo.overlay.size.x;
    Verts[vertexIndex + 1].overlayTextureHeight = (ubyte)texInfo.overlay.size.y;
    Verts[vertexIndex + 2].overlayTextureWidth = (ubyte)texInfo.overlay.size.x;
    Verts[vertexIndex + 2].overlayTextureHeight = (ubyte)texInfo.overlay.size.y;
    Verts[vertexIndex + 3].overlayTextureWidth = (ubyte)texInfo.overlay.size.x;
    Verts[vertexIndex + 3].overlayTextureHeight = (ubyte)texInfo.overlay.size.y;

    Verts[vertexIndex].position.x = pos.x + positions[vertexOffset];
    Verts[vertexIndex].position.y = pos.y + positions[vertexOffset + 1];
    Verts[vertexIndex].position.z = pos.z + positions[vertexOffset + 2];
    Verts[vertexIndex + 1].position.x = pos.x + positions[vertexOffset + 3];
    Verts[vertexIndex + 1].position.y = pos.y + positions[vertexOffset + 4];
    Verts[vertexIndex + 1].position.z = pos.z + positions[vertexOffset + 5];
    Verts[vertexIndex + 2].position.x = pos.x + positions[vertexOffset + 6];
    Verts[vertexIndex + 2].position.y = pos.y + positions[vertexOffset + 7];
    Verts[vertexIndex + 2].position.z = pos.z + positions[vertexOffset + 8];
    Verts[vertexIndex + 3].position.x = pos.x + positions[vertexOffset + 9];
    Verts[vertexIndex + 3].position.y = pos.y + positions[vertexOffset + 10];
    Verts[vertexIndex + 3].position.z = pos.z + positions[vertexOffset + 11];

    Verts[vertexIndex].color[0] = (GLubyte)(color[0]);
    Verts[vertexIndex].color[1] = (GLubyte)(color[1]);
    Verts[vertexIndex].color[2] = (GLubyte)(color[2]);
    Verts[vertexIndex + 1].color[0] = (GLubyte)(color[0]);
    Verts[vertexIndex + 1].color[1] = (GLubyte)(color[1]);
    Verts[vertexIndex + 1].color[2] = (GLubyte)(color[2]);
    Verts[vertexIndex + 2].color[0] = (GLubyte)(color[0]);
    Verts[vertexIndex + 2].color[1] = (GLubyte)(color[1]);
    Verts[vertexIndex + 2].color[2] = (GLubyte)(color[2]);
    Verts[vertexIndex + 3].color[0] = (GLubyte)(color[0]);
    Verts[vertexIndex + 3].color[1] = (GLubyte)(color[1]);
    Verts[vertexIndex + 3].color[2] = (GLubyte)(color[2]);

    Verts[vertexIndex].overlayColor[0] = (GLubyte)(overlayColor[0]);
    Verts[vertexIndex].overlayColor[1] = (GLubyte)(overlayColor[1]);
    Verts[vertexIndex].overlayColor[2] = (GLubyte)(overlayColor[2]);
    Verts[vertexIndex + 1].overlayColor[0] = (GLubyte)(overlayColor[0]);
    Verts[vertexIndex + 1].overlayColor[1] = (GLubyte)(overlayColor[1]);
    Verts[vertexIndex + 1].overlayColor[2] = (GLubyte)(overlayColor[2]);
    Verts[vertexIndex + 2].overlayColor[0] = (GLubyte)(overlayColor[0]);
    Verts[vertexIndex + 2].overlayColor[1] = (GLubyte)(overlayColor[1]);
    Verts[vertexIndex + 2].overlayColor[2] = (GLubyte)(overlayColor[2]);
    Verts[vertexIndex + 3].overlayColor[0] = (GLubyte)(overlayColor[0]);
    Verts[vertexIndex + 3].overlayColor[1] = (GLubyte)(overlayColor[1]);
    Verts[vertexIndex + 3].overlayColor[2] = (GLubyte)(overlayColor[2]);

    Verts[vertexIndex].normal[0] = normals[vertexOffset];
    Verts[vertexIndex].normal[1] = normals[vertexOffset + 1];
    Verts[vertexIndex].normal[2] = normals[vertexOffset + 2];
    Verts[vertexIndex + 1].normal[0] = normals[vertexOffset + 3];
    Verts[vertexIndex + 1].normal[1] = normals[vertexOffset + 4];
    Verts[vertexIndex + 1].normal[2] = normals[vertexOffset + 5];
    Verts[vertexIndex + 2].normal[0] = normals[vertexOffset + 6];
    Verts[vertexIndex + 2].normal[1] = normals[vertexOffset + 7];
    Verts[vertexIndex + 2].normal[2] = normals[vertexOffset + 8];
    Verts[vertexIndex + 3].normal[0] = normals[vertexOffset + 9];
    Verts[vertexIndex + 3].normal[1] = normals[vertexOffset + 10];
    Verts[vertexIndex + 3].normal[2] = normals[vertexOffset + 11];

    Verts[vertexIndex].light = light[0];
    Verts[vertexIndex + 1].light = light[0];
    Verts[vertexIndex + 2].light = light[0];
    Verts[vertexIndex + 3].light = light[0];

    Verts[vertexIndex].sunLight = light[1];
    Verts[vertexIndex + 1].sunLight = light[1];
    Verts[vertexIndex + 2].sunLight = light[1];
    Verts[vertexIndex + 3].sunLight = light[1];

    Verts[vertexIndex].merge = 0;
    Verts[vertexIndex + 1].merge = 0;
    Verts[vertexIndex + 2].merge = 0;
    Verts[vertexIndex + 3].merge = 0;

    if (waveEffect == 2){
        Verts[vertexIndex].color[3] = 255;
        Verts[vertexIndex + 1].color[3] = 255;
        Verts[vertexIndex + 2].color[3] = 255;
        Verts[vertexIndex + 3].color[3] = 255;
    } else if (waveEffect == 1){
        Verts[vertexIndex].color[3] = 255;
        Verts[vertexIndex + 1].color[3] = 0;
        Verts[vertexIndex + 2].color[3] = 0;
        Verts[vertexIndex + 3].color[3] = 255;
    } else{
        Verts[vertexIndex].color[3] = 0;
        Verts[vertexIndex + 1].color[3] = 0;
        Verts[vertexIndex + 2].color[3] = 0;
        Verts[vertexIndex + 3].color[3] = 0;
    }

    Verts[vertexIndex].tex[0] = 128 + uOffset;
    Verts[vertexIndex].tex[1] = 129 + vOffset;
    Verts[vertexIndex + 1].tex[0] = 128 + uOffset;
    Verts[vertexIndex + 1].tex[1] = 128 + vOffset;
    Verts[vertexIndex + 2].tex[0] = 129 + uOffset;
    Verts[vertexIndex + 2].tex[1] = 128 + vOffset;
    Verts[vertexIndex + 3].tex[0] = 129 + uOffset;
    Verts[vertexIndex + 3].tex[1] = 129 + vOffset;

    // *********** Base Texture
    Verts[vertexIndex].textureIndex = (GLubyte)textureIndex;
    Verts[vertexIndex + 1].textureIndex = (GLubyte)textureIndex;
    Verts[vertexIndex + 2].textureIndex = (GLubyte)textureIndex;
    Verts[vertexIndex + 3].textureIndex = (GLubyte)textureIndex;

    Verts[vertexIndex].textureAtlas = (GLubyte)texAtlas;
    Verts[vertexIndex + 1].textureAtlas = (GLubyte)texAtlas;
    Verts[vertexIndex + 2].textureAtlas = (GLubyte)texAtlas;
    Verts[vertexIndex + 3].textureAtlas = (GLubyte)texAtlas;

    // *********** Overlay texture
    Verts[vertexIndex].overlayTextureIndex = (GLubyte)overlayTex;
    Verts[vertexIndex + 1].overlayTextureIndex = (GLubyte)overlayTex;
    Verts[vertexIndex + 2].overlayTextureIndex = (GLubyte)overlayTex;
    Verts[vertexIndex + 3].overlayTextureIndex = (GLubyte)overlayTex;

    Verts[vertexIndex].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
    Verts[vertexIndex + 1].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
    Verts[vertexIndex + 2].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
    Verts[vertexIndex + 3].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
}

inline void makeCubeFace(BlockVertex *Verts, int vertexOffset, int waveEffect, i32v3& pos, int vertexIndex, int textureIndex, int overlayTextureIndex, const GLubyte color[], const GLubyte overlayColor[], GLfloat ambientOcclusion[], const BlockTexture& texInfo)
{

    //get the face index so we can determine the axis alignment
    int faceIndex = vertexOffset / 12;
    //Multiply the axis by the sign bit to get the correct offset
    GLubyte uOffset = (GLubyte)(pos[cubeFaceAxis[faceIndex][0]] * cubeFaceAxisSign[faceIndex][0]);
    GLubyte vOffset = (GLubyte)(pos[cubeFaceAxis[faceIndex][1]] * cubeFaceAxisSign[faceIndex][1]);

    // 7 per coord
    pos.x *= 7;
    pos.y *= 7;
    pos.z *= 7;

    //Blend type. The 6 LSBs are used to encode alpha blending, add/subtract, and multiplication factors.
    //They are used in the shader to determine how to blend.
    ubyte blendMode = 0x25; //0x25 = 00 10 01 01
    switch (texInfo.blendMode) {
    case BlendType::BLEND_TYPE_REPLACE:
        blendMode++; //Sets blendType to 00 01 01 10
        break;
    case BlendType::BLEND_TYPE_ADD:
        blendMode += 4; //Sets blendType to 00 01 10 01
        break;
    case BlendType::BLEND_TYPE_SUBTRACT:
        blendMode -= 4; //Sets blendType to 00 01 00 01
        break;
    case BlendType::BLEND_TYPE_MULTIPLY:
        blendMode -= 16; //Sets blendType to 00 01 01 01
        break;
    }
    Verts[vertexIndex].blendMode = blendMode;
    Verts[vertexIndex + 1].blendMode = blendMode;
    Verts[vertexIndex + 2].blendMode = blendMode;
    Verts[vertexIndex + 3].blendMode = blendMode;

    GLubyte texAtlas = (GLubyte)(textureIndex / 256);
    textureIndex %= 256;

    GLubyte overlayTexAtlas = (GLubyte)(overlayTextureIndex / 256);
    GLubyte overlayTex = (GLubyte)(overlayTextureIndex % 256);

    Verts[vertexIndex].textureWidth = (ubyte)texInfo.base.size.x;
    Verts[vertexIndex].textureHeight = (ubyte)texInfo.base.size.y;
    Verts[vertexIndex + 1].textureWidth = (ubyte)texInfo.base.size.x;
    Verts[vertexIndex + 1].textureHeight = (ubyte)texInfo.base.size.y;
    Verts[vertexIndex + 2].textureWidth = (ubyte)texInfo.base.size.x;
    Verts[vertexIndex + 2].textureHeight = (ubyte)texInfo.base.size.y;
    Verts[vertexIndex + 3].textureWidth = (ubyte)texInfo.base.size.x;
    Verts[vertexIndex + 3].textureHeight = (ubyte)texInfo.base.size.y;

    Verts[vertexIndex].overlayTextureWidth = (ubyte)texInfo.overlay.size.x;
    Verts[vertexIndex].overlayTextureHeight = (ubyte)texInfo.overlay.size.y;
    Verts[vertexIndex + 1].overlayTextureWidth = (ubyte)texInfo.overlay.size.x;
    Verts[vertexIndex + 1].overlayTextureHeight = (ubyte)texInfo.overlay.size.y;
    Verts[vertexIndex + 2].overlayTextureWidth = (ubyte)texInfo.overlay.size.x;
    Verts[vertexIndex + 2].overlayTextureHeight = (ubyte)texInfo.overlay.size.y;
    Verts[vertexIndex + 3].overlayTextureWidth = (ubyte)texInfo.overlay.size.x;
    Verts[vertexIndex + 3].overlayTextureHeight = (ubyte)texInfo.overlay.size.y;

    Verts[vertexIndex].position.x = pos.x + cubeVertices[vertexOffset];
    Verts[vertexIndex].position.y = pos.y + cubeVertices[vertexOffset + 1];
    Verts[vertexIndex].position.z = pos.z + cubeVertices[vertexOffset + 2];
    Verts[vertexIndex + 1].position.x = pos.x + cubeVertices[vertexOffset + 3];
    Verts[vertexIndex + 1].position.y = pos.y + cubeVertices[vertexOffset + 4];
    Verts[vertexIndex + 1].position.z = pos.z + cubeVertices[vertexOffset + 5];
    Verts[vertexIndex + 2].position.x = pos.x + cubeVertices[vertexOffset + 6];
    Verts[vertexIndex + 2].position.y = pos.y + cubeVertices[vertexOffset + 7];
    Verts[vertexIndex + 2].position.z = pos.z + cubeVertices[vertexOffset + 8];
    Verts[vertexIndex + 3].position.x = pos.x + cubeVertices[vertexOffset + 9];
    Verts[vertexIndex + 3].position.y = pos.y + cubeVertices[vertexOffset + 10];
    Verts[vertexIndex + 3].position.z = pos.z + cubeVertices[vertexOffset + 11];

    Verts[vertexIndex].color[0] = (GLubyte)(color[0] * ambientOcclusion[0]);
    Verts[vertexIndex].color[1] = (GLubyte)(color[1] * ambientOcclusion[0]);
    Verts[vertexIndex].color[2] = (GLubyte)(color[2] * ambientOcclusion[0]);
    Verts[vertexIndex + 1].color[0] = (GLubyte)(color[0] * ambientOcclusion[1]);
    Verts[vertexIndex + 1].color[1] = (GLubyte)(color[1] * ambientOcclusion[1]);
    Verts[vertexIndex + 1].color[2] = (GLubyte)(color[2] * ambientOcclusion[1]);
    Verts[vertexIndex + 2].color[0] = (GLubyte)(color[0] * ambientOcclusion[2]);
    Verts[vertexIndex + 2].color[1] = (GLubyte)(color[1] * ambientOcclusion[2]);
    Verts[vertexIndex + 2].color[2] = (GLubyte)(color[2] * ambientOcclusion[2]);
    Verts[vertexIndex + 3].color[0] = (GLubyte)(color[0] * ambientOcclusion[3]);
    Verts[vertexIndex + 3].color[1] = (GLubyte)(color[1] * ambientOcclusion[3]);
    Verts[vertexIndex + 3].color[2] = (GLubyte)(color[2] * ambientOcclusion[3]);

    Verts[vertexIndex].overlayColor[0] = (GLubyte)(overlayColor[0] * ambientOcclusion[0]);
    Verts[vertexIndex].overlayColor[1] = (GLubyte)(overlayColor[1] * ambientOcclusion[0]);
    Verts[vertexIndex].overlayColor[2] = (GLubyte)(overlayColor[2] * ambientOcclusion[0]);
    Verts[vertexIndex + 1].overlayColor[0] = (GLubyte)(overlayColor[0] * ambientOcclusion[1]);
    Verts[vertexIndex + 1].overlayColor[1] = (GLubyte)(overlayColor[1] * ambientOcclusion[1]);
    Verts[vertexIndex + 1].overlayColor[2] = (GLubyte)(overlayColor[2] * ambientOcclusion[1]);
    Verts[vertexIndex + 2].overlayColor[0] = (GLubyte)(overlayColor[0] * ambientOcclusion[2]);
    Verts[vertexIndex + 2].overlayColor[1] = (GLubyte)(overlayColor[1] * ambientOcclusion[2]);
    Verts[vertexIndex + 2].overlayColor[2] = (GLubyte)(overlayColor[2] * ambientOcclusion[2]);
    Verts[vertexIndex + 3].overlayColor[0] = (GLubyte)(overlayColor[0] * ambientOcclusion[3]);
    Verts[vertexIndex + 3].overlayColor[1] = (GLubyte)(overlayColor[1] * ambientOcclusion[3]);
    Verts[vertexIndex + 3].overlayColor[2] = (GLubyte)(overlayColor[2] * ambientOcclusion[3]);

	Verts[vertexIndex].normal[0] = cubeNormals[vertexOffset];
	Verts[vertexIndex].normal[1] = cubeNormals[vertexOffset + 1];
	Verts[vertexIndex].normal[2] = cubeNormals[vertexOffset + 2];
	Verts[vertexIndex + 1].normal[0] = cubeNormals[vertexOffset + 3];
	Verts[vertexIndex + 1].normal[1] = cubeNormals[vertexOffset + 4];
	Verts[vertexIndex + 1].normal[2] = cubeNormals[vertexOffset + 5];
	Verts[vertexIndex + 2].normal[0] = cubeNormals[vertexOffset + 6];
	Verts[vertexIndex + 2].normal[1] = cubeNormals[vertexOffset + 7];
	Verts[vertexIndex + 2].normal[2] = cubeNormals[vertexOffset + 8];
	Verts[vertexIndex + 3].normal[0] = cubeNormals[vertexOffset + 9];
	Verts[vertexIndex + 3].normal[1] = cubeNormals[vertexOffset + 10];
	Verts[vertexIndex + 3].normal[2] = cubeNormals[vertexOffset + 11];

	Verts[vertexIndex].merge = 1;
	Verts[vertexIndex + 1].merge = 1;
	Verts[vertexIndex + 2].merge = 1;
	Verts[vertexIndex + 3].merge = 1;

	if (waveEffect == 2){
		Verts[vertexIndex].color[3] = 255;
		Verts[vertexIndex + 1].color[3] = 255;
		Verts[vertexIndex + 2].color[3] = 255;
		Verts[vertexIndex + 3].color[3] = 255;
	}
	else if (waveEffect == 1){
		Verts[vertexIndex].color[3] = 255;
		Verts[vertexIndex + 1].color[3] = 0;
		Verts[vertexIndex + 2].color[3] = 0;
		Verts[vertexIndex + 3].color[3] = 255;
	}
	else{
		Verts[vertexIndex].color[3] = 0;
		Verts[vertexIndex + 1].color[3] = 0;
		Verts[vertexIndex + 2].color[3] = 0;
		Verts[vertexIndex + 3].color[3] = 0;
	}

    Verts[vertexIndex].tex[0] = 128 + uOffset;
    Verts[vertexIndex].tex[1] = 129 + vOffset;
    Verts[vertexIndex + 1].tex[0] = 128 + uOffset;
    Verts[vertexIndex + 1].tex[1] = 128 + vOffset;
    Verts[vertexIndex + 2].tex[0] = 129 + uOffset;
    Verts[vertexIndex + 2].tex[1] = 128 + vOffset;
    Verts[vertexIndex + 3].tex[0] = 129 + uOffset;
    Verts[vertexIndex + 3].tex[1] = 129 + vOffset;

    // *********** Base Texture
	Verts[vertexIndex].textureIndex = (GLubyte)textureIndex;
	Verts[vertexIndex + 1].textureIndex = (GLubyte)textureIndex;
	Verts[vertexIndex + 2].textureIndex = (GLubyte)textureIndex;
	Verts[vertexIndex + 3].textureIndex = (GLubyte)textureIndex;

    Verts[vertexIndex].textureAtlas = (GLubyte)texAtlas;
    Verts[vertexIndex + 1].textureAtlas = (GLubyte)texAtlas;
    Verts[vertexIndex + 2].textureAtlas = (GLubyte)texAtlas;
    Verts[vertexIndex + 3].textureAtlas = (GLubyte)texAtlas;

    // *********** Overlay texture
    Verts[vertexIndex].overlayTextureIndex = (GLubyte)overlayTex;
    Verts[vertexIndex + 1].overlayTextureIndex = (GLubyte)overlayTex;
    Verts[vertexIndex + 2].overlayTextureIndex = (GLubyte)overlayTex;
    Verts[vertexIndex + 3].overlayTextureIndex = (GLubyte)overlayTex;

    Verts[vertexIndex].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
    Verts[vertexIndex + 1].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
    Verts[vertexIndex + 2].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
    Verts[vertexIndex + 3].overlayTextureAtlas = (GLubyte)overlayTexAtlas;
}

const GLubyte waterUVs[8] = { 0, 7, 0, 0, 7, 0, 7, 7 };

inline void makeLiquidFace(std::vector<LiquidVertex>& verts, i32 index, ui8 uOff, ui8 vOff, ui8 light[2], ui8 color[3], ui8 textureUnit) {

    verts.resize(verts.size() + 4);
    verts[index].tex[0] = waterUVs[0] + uOff;
    verts[index].tex[1] = waterUVs[1] + vOff;
    verts[index + 1].tex[0] = waterUVs[2] + uOff;
    verts[index + 1].tex[1] = waterUVs[3] + vOff;
    verts[index + 2].tex[0] = waterUVs[4] + uOff;
    verts[index + 2].tex[1] = waterUVs[5] + vOff;
    verts[index + 3].tex[0] = waterUVs[6] + uOff;
    verts[index + 3].tex[1] = waterUVs[7] + vOff;

    verts[index].light = light[0];
    verts[index + 1].light = light[0];
    verts[index + 2].light = light[0];
    verts[index + 3].light = light[0];
    verts[index].sunLight = light[1];
    verts[index + 1].sunLight = light[1];
    verts[index + 2].sunLight = light[1];
    verts[index + 3].sunLight = light[1];

    verts[index].color[0] = color[0];
    verts[index].color[1] = color[1];
    verts[index].color[2] = color[2];
    verts[index + 1].color[0] = color[0];
    verts[index + 1].color[1] = color[1];
    verts[index + 1].color[2] = color[2];
    verts[index + 2].color[0] = color[0];
    verts[index + 2].color[1] = color[1];
    verts[index + 2].color[2] = color[2];
    verts[index + 3].color[0] = color[0];
    verts[index + 3].color[1] = color[1];
    verts[index + 3].color[2] = color[2];

    verts[index].textureUnit = (GLubyte)textureUnit;
    verts[index + 1].textureUnit = (GLubyte)textureUnit;
    verts[index + 2].textureUnit = (GLubyte)textureUnit;
    verts[index + 3].textureUnit = (GLubyte)textureUnit;
}

#define CTOP + PADDED_LAYER
#define CBOT - PADDED_LAYER
#define CLEFT - 1
#define CRIGHT + 1
#define CBACK - PADDED_WIDTH
#define CFRONT + PADDED_WIDTH

void GetLeftLightData(int c, GLbyte &l, GLbyte &sl, GLushort *data, GLubyte *lightData[2])
{
	if (Blocks[GETBLOCKTYPE(data[c-1])].occlude){
		l = sl = -1;
	}else{
		l = (lightData[0][c-1]); 
		sl = (lightData[1][c-1]); 
	}
}

inline void GetRightLightData(int c, GLbyte &l, GLbyte &sl, GLushort *data, GLubyte *lightData[2])
{
	
	if (Blocks[GETBLOCKTYPE(data[c + 1])].occlude){
		l = sl = -1;
	}else{
		l = (lightData[0][c+1]); 
		sl = (lightData[1][c+1]); 
	}
}
inline void GetFrontLightData(int c, GLbyte &l, GLbyte &sl, GLushort *data, GLubyte *lightData[2])
{
	if (Blocks[GETBLOCKTYPE(data[c + PADDED_WIDTH])].occlude){
		l = sl = -1;
	}else{
		l = (lightData[0][c+PADDED_WIDTH]); 
		sl = (lightData[1][c+PADDED_WIDTH]); 
	}
}
inline void GetBackLightData(int c, GLbyte &l, GLbyte &sl, GLushort *data, GLubyte *lightData[2])
{
	if (Blocks[GETBLOCKTYPE(data[c - PADDED_WIDTH])].occlude){
		l = sl = -1;
	}else{
		l = (lightData[0][c-PADDED_WIDTH]); 
		sl = (lightData[1][c-PADDED_WIDTH]);
	}
}
inline void GetBottomLightData(int c, GLbyte &l, GLbyte &sl, GLushort *data, GLubyte *lightData[2])
{
	if (Blocks[GETBLOCKTYPE(data[c - PADDED_LAYER])].occlude){
		l = sl = -1;
	}else{
		l = lightData[0][c - PADDED_LAYER];
		sl = lightData[1][c - PADDED_LAYER];
	}
}
inline void GetTopLightData(int c, GLbyte &l, GLbyte &sl, GLushort *data, GLubyte *lightData[2])
{
	if (Blocks[GETBLOCKTYPE(data[c + PADDED_LAYER])].occlude){
		l = sl = -1;
	}else{
		l = (lightData[0][c + PADDED_LAYER]);
		sl = (lightData[1][c + PADDED_LAYER]);
	}
}

//Fills chLightData with the lighting information of the surrounding blocks. This will be used to calculate lighting and ambient occlusion
//It only has to get the light voxels that were not grabbed in checkBlockFaces()
void ChunkMesher::GetLightDataArray(int c, int &x, int &y, int &z, GLbyte lights[], GLbyte sunlights[], GLushort *chData, GLubyte *chLightData[2], bool faces[6])
{
	int c2;
	//bottom 8 voxels
	if (faces[XNEG] || faces[ZNEG] || faces[XPOS] || faces[ZPOS] || faces[YNEG]){
		c2 = c - PADDED_LAYER;
		GetBackLightData(c2, lights[1], sunlights[1], chData, chLightData);
		GetLeftLightData(c2, lights[3], sunlights[3], chData, chLightData);
		GetRightLightData(c2, lights[5], sunlights[5], chData, chLightData);
		GetFrontLightData(c2, lights[7], sunlights[7], chData, chLightData);

		GetLeftLightData(c2 - PADDED_WIDTH, lights[0], sunlights[0], chData, chLightData);
		GetRightLightData(c2 - PADDED_WIDTH, lights[2], sunlights[2], chData, chLightData);
		
		GetLeftLightData(c2 + PADDED_WIDTH, lights[6], sunlights[6], chData, chLightData);
		GetRightLightData(c2 + PADDED_WIDTH, lights[8], sunlights[8], chData, chLightData);
		
	}
    //top 8 voxels
	if (faces[XNEG] || faces[ZNEG] || faces[XPOS] || faces[ZPOS] || faces[YPOS]){
		c2 = c + PADDED_LAYER;
		GetBackLightData(c2, lights[18], sunlights[18], chData, chLightData);
		GetLeftLightData(c2, lights[20], sunlights[20], chData, chLightData);
		GetRightLightData(c2, lights[22], sunlights[22], chData, chLightData);
		GetFrontLightData(c2, lights[24], sunlights[24], chData, chLightData);
			
		GetLeftLightData(c2 - PADDED_WIDTH, lights[17], sunlights[17], chData, chLightData);
		GetRightLightData(c2 - PADDED_WIDTH, lights[19], sunlights[19], chData, chLightData);
		
		GetLeftLightData(c2 + PADDED_WIDTH, lights[23], sunlights[23], chData, chLightData);
		GetRightLightData(c2 + PADDED_WIDTH, lights[25], sunlights[25], chData, chLightData);
	}
    //left 2 voxels
	if (faces[XNEG] || faces[ZNEG] || faces[ZPOS]){
		//left
		c2 = c-1;
		GetBackLightData(c2, lights[9], sunlights[9], chData, chLightData);
		GetFrontLightData(c2, lights[14], sunlights[14], chData, chLightData);
	}
    //right 2 voxels
	if (faces[XPOS] || faces[ZPOS] || faces[ZNEG]){
		c2 = c+1;
		GetBackLightData(c2, lights[11], sunlights[11], chData, chLightData);
		GetFrontLightData(c2, lights[16], sunlights[16], chData, chLightData);
	}
}

//This function checks all surrounding blocks and stores occlusion information in faces[]. It also stores the adjacent light and sunlight for each face.
bool ChunkMesher::checkBlockFaces(bool faces[6], GLbyte lights[26], GLbyte sunlights[26], const RenderTask* task, const bool occlude, const int btype, const int wc)
{
	Block *nblock;
	bool hasFace = false;

	if (faces[XNEG] = ((nblock = &GETBLOCK(task->chData[wc - 1])))->occlude == 0 || ((nblock->occlude == 2 || occlude == 2) && nblock->ID != btype)){
		hasFace = true;
		lights[12] = (task->chLightData[0][wc - 1]);
		sunlights[12] = (task->chLightData[1][wc - 1]);
	}

	if (faces[XPOS] = ((nblock = &GETBLOCK(task->chData[1 + wc]))->occlude == 0 || ((nblock->occlude == 2 || occlude == 2) && nblock->ID != btype))){
        hasFace = true;
		lights[13] = (task->chLightData[0][1 + wc]);
		sunlights[13] = (task->chLightData[1][1 + wc]);
	}

	if (faces[YNEG] = ((nblock = &GETBLOCK(task->chData[wc - PADDED_LAYER]))->occlude == 0 || ((nblock->occlude == 2 || occlude == 2) && nblock->ID != btype))){
        hasFace = true;
		lights[4] = (task->chLightData[0][wc - PADDED_LAYER]);
		sunlights[4] = (task->chLightData[1][wc - PADDED_LAYER]);
	}

	if (faces[YPOS] = ((nblock = &GETBLOCK(task->chData[wc + PADDED_LAYER]))->occlude == 0 || ((nblock->occlude == 2 || occlude == 2) && nblock->ID != btype))){
        hasFace = true;
		lights[21] = (task->chLightData[0][wc + PADDED_LAYER]);
		sunlights[21] = (task->chLightData[1][wc + PADDED_LAYER]);
	}

	if (faces[ZNEG] = ((nblock = &GETBLOCK(task->chData[wc - PADDED_WIDTH]))->occlude == 0 || ((nblock->occlude == 2 || occlude == 2) && nblock->ID != btype))){
        hasFace = true;
		lights[10] = (task->chLightData[0][wc - PADDED_WIDTH]);
		sunlights[10] = (task->chLightData[1][wc - PADDED_WIDTH]);
	}

	if (faces[ZPOS] = ((nblock = &GETBLOCK(task->chData[wc + PADDED_WIDTH]))->occlude == 0 || ((nblock->occlude == 2 || occlude == 2) && nblock->ID != btype))){
        hasFace = true;
		lights[15] = (task->chLightData[0][wc + PADDED_WIDTH]);
		sunlights[15] = (task->chLightData[1][wc + PADDED_WIDTH]);
	}

	return hasFace;
}

//Calculates the smooth lighting based on accumulated light and num adjacent blocks.
//Returns it as a GLubyte for vertex data
GLubyte ChunkMesher::calculateSmoothLighting(int accumulatedLight, int numAdjacentBlocks) {
    return (GLubyte)(255.0f * (LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - ((float)(accumulatedLight) / (4 - numAdjacentBlocks)))));
}

//Gets texture offset according to the texturing method
//Grass method may change color
void ChunkMesher::getTextureIndex(const MeshInfo &mi, const BlockTextureLayer& blockTexture, int& result, int rightDir, int upDir, int frontDir, unsigned int directionIndex, ui8 color[3]) {
    switch (blockTexture.method) {
    case ConnectedTextureMethods::CTM_CONNECTED:
        return getConnectedTextureIndex(mi, result, blockTexture.innerSeams, rightDir, upDir, frontDir, directionIndex);
        break;
    case ConnectedTextureMethods::CTM_RANDOM:
        return getRandomTextureIndex(mi, blockTexture, result);
        break;
    case ConnectedTextureMethods::CTM_GRASS:
        return getGrassTextureIndex(mi, result, rightDir, upDir, frontDir, directionIndex, color);
        break;
    }
}

//Gets overlay texture offset according to the texturing method
//Grass method may change color
//void ChunkMesher::getOverlayTextureIndex(const MeshInfo &mi, const BlockTexture& blockTexture, int& result, int rightDir, int upDir, int frontDir, unsigned int directionIndex, ui8 overlayColor[3]) {
//    switch (blockTexture.overlay.method) {
//    case CTM_CONNECTED:
//        return getConnectedTextureIndex(mi, result, blockTexture.overlay.innerSeams, rightDir, upDir, frontDir, directionIndex);
//        break;
//    case CTM_RANDOM:
//        return getRandomTextureIndex(mi, blockTexture, result);
//        break;
//    case CTM_GRASS:
//        return getGrassTextureIndex(mi, result, rightDir, upDir, frontDir, directionIndex, overlayColor);
//        break;
//    }
//}

//Gets a random offset for use by random textures
void ChunkMesher::getRandomTextureIndex(const MeshInfo &mi, const BlockTextureLayer& blockTexInfo, int& result) {
    //TODO: MurmurHash3
    int seed = getPositionSeed(mi.x + mi.task->position.x, mi.y + mi.task->position.y, mi.z + mi.task->position.z);

    float r = (PseudoRand(seed) + 1.0) * 0.5 * blockTexInfo.totalWeight;
    float totalWeight = 0;
    if (blockTexInfo.weights.length()){
        for (int i = 0; i < blockTexInfo.numTiles; i++) {
            totalWeight += blockTexInfo.weights[i];
            if (r <= totalWeight) {
                result += i;
                return;
            }
        }
    } else {
        for (int i = 0; i < blockTexInfo.numTiles; i++) {
            totalWeight += 1.0f;
            if (r <= totalWeight) {
                result += i;
                return;
            }
        }
    }
}

//Gets a connected texture offset by looking at the surrounding blocks
void ChunkMesher::getConnectedTextureIndex(const MeshInfo &mi, int& result, bool innerSeams, int rightDir, int upDir, int frontDir, unsigned int offset) {

    int connectedOffset = 0;
    int wc = mi.wc;
    int tex = result;
    ui16* chData = mi.task->chData;
    // Top Left
    Block *block = &GETBLOCK(chData[wc + upDir - rightDir]);
    if (*((int *)(&block->pxTex) + offset) != tex) {
        connectedOffset |= 0x80;
    }

    // Top
    block = &GETBLOCK(chData[wc + upDir]);
    if (*((int *)(&block->pxTex) + offset) != tex) {
        connectedOffset |= 0xE0;
    }

    // Top Right
    block = &GETBLOCK(chData[wc + upDir + rightDir]);
    if (*((int *)(&block->pxTex) + offset) != tex) {
        connectedOffset |= 0x20;
    }

    // Right
    block = &GETBLOCK(chData[wc + rightDir]);
    if (*((int *)(&block->pxTex) + offset) != tex) {
        connectedOffset |= 0x38;
    }

    // Bottom Right
    block = &GETBLOCK(chData[wc - upDir + rightDir]);
    if (*((int *)(&block->pxTex) + offset) != tex) {
        connectedOffset |= 0x8;
    }

    // Bottom
    block = &GETBLOCK(chData[wc - upDir]);
    if (*((int *)(&block->pxTex) + offset) != tex) {
        connectedOffset |= 0xE;
    }

    // Bottom Left
    block = &GETBLOCK(chData[wc - upDir - rightDir]);
    if (*((int *)(&block->pxTex) + offset) != tex) {
        connectedOffset |= 0x2;
    }

    // Left
    block = &GETBLOCK(chData[wc - rightDir]);
    if (*((int *)(&block->pxTex) + offset) != tex) {
        connectedOffset |= 0x83;
    }

    if (innerSeams) {
        // Top Front Left
        Block *block = &GETBLOCK(chData[wc + upDir - rightDir + frontDir]);
        if (block->occlude) {
            connectedOffset |= 0x80;
        }

        // Top Front Right
        block = &GETBLOCK(chData[wc + upDir + rightDir + frontDir]);
        if (block->occlude) {
            connectedOffset |= 0x20;
        }

        // Bottom front Right
        block = &GETBLOCK(chData[wc - upDir + rightDir + frontDir]);
        if (block->occlude) {
            connectedOffset |= 0x8;
        }

        //Bottom front
        block = &GETBLOCK(chData[wc - upDir + frontDir]);
        if (block->occlude) {
            connectedOffset |= 0xE;
        }

        // Bottom front Left
        block = &GETBLOCK(chData[wc - upDir - rightDir + frontDir]);
        if (block->occlude) {
            connectedOffset |= 0x2;
        }

        //Left front
        block = &GETBLOCK(chData[wc - rightDir + frontDir]);
        if (block->occlude) {
            connectedOffset |= 0x83;
        }

        //Top front
        block = &GETBLOCK(chData[wc + upDir + frontDir]);
        if (block->occlude) {
            connectedOffset |= 0xE0;
        }

        //Right front
        block = &GETBLOCK(chData[wc + rightDir + frontDir]);
        if (block->occlude) {
            connectedOffset |= 0x38;
        }
    }


    result += connectedTextureOffsets[connectedOffset];
}

//Gets a grass side texture offset by looking at the surrounding blocks
void ChunkMesher::getGrassTextureIndex(const MeshInfo &mi, int& result, int rightDir, int upDir, int frontDir, unsigned int offset, ui8 color[3]) {

    int connectedOffset = 0;
    int wc = mi.wc;

    int tex = result;

    ui16* chData = mi.task->chData;

    // Bottom Front
    Block* block = &GETBLOCK(chData[wc - upDir + frontDir]);
    if (*((int *)(&block->pxTex) + offset) == tex) {
        block = &GETBLOCK(chData[wc]);
        result = block->pyTexInfo.base.textureIndex;
        getTextureIndex(mi, block->pyTexInfo.base, result, 1, -PADDED_WIDTH, PADDED_LAYER, 1, color);
        block->GetBlockColor(color, 0, mi.temperature, mi.rainfall, block->pyTexInfo);
        return;
    }

    // Left
    block = &GETBLOCK(chData[wc - rightDir]);
    if (*((int *)(&block->pxTex) + offset) == tex || block->occlude == 0) {
        connectedOffset |= 0x8;

        if (*((int *)(&block->pxTex) + offset) == tex) {
            // bottom front Left
            block = &GETBLOCK(chData[wc - upDir - rightDir + frontDir]);
            if (*((int *)(&block->pxTex) + offset) == tex) {
                connectedOffset |= 0xC;
            }
        }
    }
    // Front left
    block = &GETBLOCK(chData[wc - rightDir + frontDir]);
    if (*((int *)(&block->pxTex) + offset) == tex) {
        connectedOffset |= 0x8;
    }
 
    // Bottom left
    block = &GETBLOCK(chData[wc - upDir - rightDir]);
    if (*((int *)(&block->pxTex) + offset) == tex) {
        connectedOffset |= 0xC;
    }

    // bottom right
    block = &GETBLOCK(chData[wc - upDir + rightDir]);
    if (*((int *)(&block->pxTex) + offset) == tex) {
        connectedOffset |= 0x3;
    }

    // Right
    block = &GETBLOCK(chData[wc + rightDir]);
    if (*((int *)(&block->pxTex) + offset) == tex || block->occlude == 0) {
        connectedOffset |= 0x1;

        if (*((int *)(&block->pxTex) + offset) == tex) {
            // bottom front Right
            block = &GETBLOCK(chData[wc - upDir + rightDir + frontDir]);
            if (*((int *)(&block->pxTex) + offset) == tex) {
                connectedOffset |= 0x3;
            }
        }
    }

    // Front right
    block = &GETBLOCK(chData[wc + rightDir + frontDir]);
    if (*((int *)(&block->pxTex) + offset) == tex) {
        connectedOffset |= 0x1;
    }


    result += grassTextureOffsets[connectedOffset];
}

void ChunkMesher::addBlockToMesh(MeshInfo& mi)
{
    const Block &block = Blocks[mi.btype];
    const bool occlude = (block.occlude != 0);
    GLbyte lights[26];
    GLbyte sunlights[26];
    GLubyte color[3], overlayColor[3];
    const int wc = mi.wc;
    const int btype = mi.btype;
    int nearBlocks[4];
    int textureIndex, overlayTextureIndex;
    const GLfloat OCCLUSION_FACTOR = 0.2f;

    ui8 light[2] = { mi.task->chLightData[0][mi.wc], mi.task->chLightData[1][mi.wc] };

    GLfloat ambientOcclusion[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    //Lookup the current biome, temperature, and rainfall
    Biome *biome = GameManager::planet->allBiomesLookupVector[mi.task->biomes[mi.z*CHUNK_WIDTH + mi.x]];
    mi.temperature = mi.task->temperatures[mi.z*CHUNK_WIDTH + mi.x];
    mi.rainfall = mi.task->rainfalls[mi.z*CHUNK_WIDTH + mi.x];

    //get bit flags (needs to be changed) -Ben
    GLuint flags = GETFLAGS(mi.task->chData[mi.wc]);

    bool faces[6] = { false, false, false, false, false, false };
    //Check for which faces are occluded by nearby blocks
    if (checkBlockFaces(faces, lights, sunlights, mi.task, occlude, btype, wc) == 0) {
        mi.mergeFront = 0;
        mi.mergeBack = 0;
        mi.mergeBot = 0;
        mi.mergeUp = 0;
        return;
    }

    //Get nearby lighting information
    ui8* lightData[2] = { mi.task->chLightData[0], mi.task->chLightData[1] };
    GetLightDataArray(wc, mi.x, mi.y, mi.z, lights, sunlights, mi.task->chData, lightData, faces);

	if (faces[ZPOS]){ //0 1 2 3
        //Get the color of the block
        Blocks[btype].GetBlockColor(color, overlayColor, flags, mi.temperature, mi.rainfall, block.pzTexInfo);

        //Count the number of solid blocks ad add them to near blocks for ambient occlusion
		nearBlocks[0] = (int)(lights[23] == -1) + (int)(lights[24] == -1) + (int)(lights[14] == -1);
		nearBlocks[1] = (int)(lights[6] == -1) + (int)(lights[7] == -1) + (int)(lights[14] == -1);
		nearBlocks[2] = (int)(lights[7] == -1) + (int)(lights[8] == -1) + (int)(lights[16] == -1);
		nearBlocks[3] = (int)(lights[24] == -1) + (int)(lights[25] == -1) + (int)(lights[16] == -1);
        //Calculate ambient occlusion based on nearby blocks
		ambientOcclusion[0] = 1.0f - nearBlocks[0] * OCCLUSION_FACTOR;
		ambientOcclusion[1] = 1.0f - nearBlocks[1] * OCCLUSION_FACTOR;
		ambientOcclusion[2] = 1.0f - nearBlocks[2] * OCCLUSION_FACTOR;
		ambientOcclusion[3] = 1.0f - nearBlocks[3] * OCCLUSION_FACTOR;

        textureIndex = block.pzTexInfo.base.textureIndex;
        overlayTextureIndex = block.pzTexInfo.overlay.textureIndex;

        //We will offset the texture index based on the texture method
        getTextureIndex(mi, block.pzTexInfo.base, textureIndex, 1, PADDED_LAYER, PADDED_WIDTH, 2, color);
        getTextureIndex(mi, block.pzTexInfo.overlay, overlayTextureIndex, 1, PADDED_LAYER, PADDED_WIDTH, 8, overlayColor);

        if (block.pzTexInfo.base.transparency) {
            _nonBlockVerts.resize(_nonBlockVerts.size() + 4);
            makeFloraFace(&(_nonBlockVerts[0]), &(cubeVertices[0]), &(cubeNormals[0]), 0, block.waveEffect, i32v3(mi.x, mi.y, mi.z), mi.nbIndex, textureIndex, overlayTextureIndex, color, overlayColor, light, block.pzTexInfo);
            mi.nbIndex += 4;

            chunkMeshData->addTransQuad(i8v3(mi.x2 + 1, mi.y2 + 1, mi.z2 + 2));

            mi.mergeFront = false;
        } else {
            //Set up most of the vertex data for a face
            makeCubeFace(_frontVerts, 0, (int)block.waveEffect, glm::ivec3(mi.x, mi.y, mi.z), mi.frontIndex, textureIndex, overlayTextureIndex, color, overlayColor, ambientOcclusion, block.pzTexInfo);

            //Set up the light data using smooth lighting
            _frontVerts[mi.frontIndex].light = calculateSmoothLighting(lights[23] + lights[24] + lights[14] + lights[15] + nearBlocks[0], nearBlocks[0]);
            _frontVerts[mi.frontIndex + 1].light = calculateSmoothLighting(lights[6] + lights[7] + lights[14] + lights[15] + nearBlocks[1], nearBlocks[1]);
            _frontVerts[mi.frontIndex + 2].light = calculateSmoothLighting(lights[7] + lights[8] + lights[15] + lights[16] + nearBlocks[2], nearBlocks[2]);
            _frontVerts[mi.frontIndex + 3].light = calculateSmoothLighting(lights[24] + lights[25] + lights[15] + lights[16] + nearBlocks[3], nearBlocks[3]);
            _frontVerts[mi.frontIndex].sunLight = calculateSmoothLighting(sunlights[23] + sunlights[24] + sunlights[14] + sunlights[15] + nearBlocks[0], nearBlocks[0]);
            _frontVerts[mi.frontIndex + 1].sunLight = calculateSmoothLighting(sunlights[6] + sunlights[7] + sunlights[14] + sunlights[15] + nearBlocks[1], nearBlocks[1]);
            _frontVerts[mi.frontIndex + 2].sunLight = calculateSmoothLighting(sunlights[7] + sunlights[8] + sunlights[15] + sunlights[16] + nearBlocks[2], nearBlocks[2]);
            _frontVerts[mi.frontIndex + 3].sunLight = calculateSmoothLighting(sunlights[24] + sunlights[25] + sunlights[15] + sunlights[16] + nearBlocks[3], nearBlocks[3]);

            //to check for a +x merge, we check that the vertices aligned in the direction of stretch are equal
            if (mi.mergeFront && mi.pbtype == btype &&
                CompareVertices(_frontVerts[mi.pfrontIndex], _frontVerts[mi.pfrontIndex + 3]) && CompareVertices(_frontVerts[mi.pfrontIndex + 3], _frontVerts[mi.frontIndex]) && CompareVertices(_frontVerts[mi.frontIndex], _frontVerts[mi.frontIndex + 3]) &&//-z vertices
                CompareVertices(_frontVerts[mi.pfrontIndex + 1], _frontVerts[mi.pfrontIndex + 2]) && CompareVertices(_frontVerts[mi.pfrontIndex + 2], _frontVerts[mi.frontIndex + 1]) && CompareVertices(_frontVerts[mi.frontIndex + 1], _frontVerts[mi.frontIndex + 2])){ //+z vertices

                _frontVerts[mi.pfrontIndex + 2].position.x += 7;
                _frontVerts[mi.pfrontIndex + 3].position.x += 7;
                _frontVerts[mi.pfrontIndex + 2].tex[0]++;
                _frontVerts[mi.pfrontIndex + 3].tex[0]++;
            } else{
                mi.pfrontIndex = mi.frontIndex;
                mi.frontIndex += 4;
                mi.mergeFront = 1;
            }
        }
	}
	else{
		mi.mergeFront = 0;
	}

    if (faces[ZNEG]) {
        //Get the color of the block
        Blocks[btype].GetBlockColor(color, overlayColor, flags, mi.temperature, mi.rainfall, block.nzTexInfo);

        nearBlocks[0] = (int)(lights[18] == -1) + (int)(lights[19] == -1) + (int)(lights[11] == -1);
        nearBlocks[1] = (int)(lights[1] == -1) + (int)(lights[2] == -1) + (int)(lights[11] == -1);
        nearBlocks[2] = (int)(lights[0] == -1) + (int)(lights[1] == -1) + (int)(lights[9] == -1);
        nearBlocks[3] = (int)(lights[17] == -1) + (int)(lights[18] == -1) + (int)(lights[9] == -1);
        ambientOcclusion[0] = 1.0f - nearBlocks[0] * OCCLUSION_FACTOR;
        ambientOcclusion[1] = 1.0f - nearBlocks[1] * OCCLUSION_FACTOR;
        ambientOcclusion[2] = 1.0f - nearBlocks[2] * OCCLUSION_FACTOR;
        ambientOcclusion[3] = 1.0f - nearBlocks[3] * OCCLUSION_FACTOR;
       
        textureIndex = block.nzTexInfo.base.textureIndex;
        overlayTextureIndex = block.nzTexInfo.overlay.textureIndex;

        //We will offset the texture index based on the texture method
        getTextureIndex(mi, block.nzTexInfo.base, textureIndex, -1, PADDED_LAYER, -PADDED_WIDTH, 5, color);
        getTextureIndex(mi, block.nzTexInfo.overlay, overlayTextureIndex, -1, PADDED_LAYER, -PADDED_WIDTH, 11, overlayColor);

        if (block.nzTexInfo.base.transparency) {
            _nonBlockVerts.resize(_nonBlockVerts.size() + 4);
            makeFloraFace(&(_nonBlockVerts[0]), &(cubeVertices[0]), &(cubeNormals[0]), 60, block.waveEffect, i32v3(mi.x, mi.y, mi.z), mi.nbIndex, textureIndex, overlayTextureIndex, color, overlayColor, light, block.nzTexInfo);
            mi.nbIndex += 4;

            chunkMeshData->addTransQuad(i8v3(mi.x2 + 1, mi.y2 + 1, mi.z2));
            mi.mergeBack = false;
        } else {
            makeCubeFace(_backVerts, 60, (int)block.waveEffect, glm::ivec3(mi.x, mi.y, mi.z), mi.backIndex, textureIndex, overlayTextureIndex, color, overlayColor, ambientOcclusion, block.nzTexInfo);

            _backVerts[mi.backIndex].light = calculateSmoothLighting(lights[18] + lights[19] + lights[11] + lights[10] + nearBlocks[0], nearBlocks[0]);
            _backVerts[mi.backIndex + 1].light = calculateSmoothLighting(lights[1] + lights[2] + lights[11] + lights[10] + nearBlocks[1], nearBlocks[1]);
            _backVerts[mi.backIndex + 2].light = calculateSmoothLighting(lights[0] + lights[1] + lights[9] + lights[10] + nearBlocks[2], nearBlocks[2]);
            _backVerts[mi.backIndex + 3].light = calculateSmoothLighting(lights[17] + lights[18] + lights[9] + lights[10] + nearBlocks[3], nearBlocks[3]);
            _backVerts[mi.backIndex].sunLight = calculateSmoothLighting(sunlights[18] + sunlights[19] + sunlights[11] + sunlights[10] + nearBlocks[0], nearBlocks[0]);
            _backVerts[mi.backIndex + 1].sunLight = calculateSmoothLighting(sunlights[1] + sunlights[2] + sunlights[11] + sunlights[10] + nearBlocks[1], nearBlocks[1]);
            _backVerts[mi.backIndex + 2].sunLight = calculateSmoothLighting(sunlights[0] + sunlights[1] + sunlights[9] + sunlights[10] + nearBlocks[2], nearBlocks[2]);
            _backVerts[mi.backIndex + 3].sunLight = calculateSmoothLighting(sunlights[17] + sunlights[18] + sunlights[9] + sunlights[10] + nearBlocks[3], nearBlocks[3]);

            //to check for a +x merge, we check that the vertices aligned in the direction of stretch are equal
            if (mi.mergeBack && mi.pbtype == btype &&
                CompareVertices(_backVerts[mi.pbackIndex], _backVerts[mi.pbackIndex + 3]) && CompareVertices(_backVerts[mi.pbackIndex + 3], _backVerts[mi.backIndex]) && CompareVertices(_backVerts[mi.backIndex], _backVerts[mi.backIndex + 3]) &&//-z vertices
                CompareVertices(_backVerts[mi.pbackIndex + 1], _backVerts[mi.pbackIndex + 2]) && CompareVertices(_backVerts[mi.pbackIndex + 2], _backVerts[mi.backIndex + 1]) && CompareVertices(_backVerts[mi.backIndex + 1], _backVerts[mi.backIndex + 2])){ //+z vertices

                _backVerts[mi.pbackIndex].position.x += 7;
                _backVerts[mi.pbackIndex + 1].position.x += 7;
                _backVerts[mi.pbackIndex].tex[0]--;
                _backVerts[mi.pbackIndex + 1].tex[0]--;
            } else{
                mi.pbackIndex = mi.backIndex;
                mi.backIndex += 4;
                mi.mergeBack = 1;
            }
        }
    } else {
        mi.mergeBack = 0;
    }

	if (faces[YPOS]){ //0 5 6 1                        
        Blocks[btype].GetBlockColor(color, overlayColor, flags, mi.temperature, mi.rainfall, block.pyTexInfo);

		nearBlocks[0] = (int)(lights[17] == -1) + (int)(lights[18] == -1) + (int)(lights[20] == -1);
		nearBlocks[1] = (int)(lights[20] == -1) + (int)(lights[23] == -1) + (int)(lights[24] == -1);
		nearBlocks[2] = (int)(lights[22] == -1) + (int)(lights[24] == -1) + (int)(lights[25] == -1);
		nearBlocks[3] = (int)(lights[18] == -1) + (int)(lights[19] == -1) + (int)(lights[22] == -1);
		ambientOcclusion[0] = 1.0f - nearBlocks[0] * OCCLUSION_FACTOR;
		ambientOcclusion[1] = 1.0f - nearBlocks[1] * OCCLUSION_FACTOR;
		ambientOcclusion[2] = 1.0f - nearBlocks[2] * OCCLUSION_FACTOR;
		ambientOcclusion[3] = 1.0f - nearBlocks[3] * OCCLUSION_FACTOR;

        textureIndex = block.pyTexInfo.base.textureIndex;
        overlayTextureIndex = block.pyTexInfo.overlay.textureIndex;

        //We will offset the texture index based on the texture method
        getTextureIndex(mi, block.pyTexInfo.base, textureIndex, 1, -PADDED_WIDTH, PADDED_LAYER, 1, color);
        getTextureIndex(mi, block.pyTexInfo.overlay, overlayTextureIndex, 1, -PADDED_WIDTH, PADDED_LAYER, 7, overlayColor);

        if (block.pyTexInfo.base.transparency) {
            _nonBlockVerts.resize(_nonBlockVerts.size() + 4);
            makeFloraFace(&(_nonBlockVerts[0]), &(cubeVertices[0]), &(cubeNormals[0]), 24, block.waveEffect, i32v3(mi.x, mi.y, mi.z), mi.nbIndex, textureIndex, overlayTextureIndex, color, overlayColor, light, block.pyTexInfo);
            mi.nbIndex += 4;

            chunkMeshData->addTransQuad(i8v3(mi.x2 + 1, mi.y2 + 2, mi.z2 + 1));

            mi.mergeUp = false;
        } else {
            makeCubeFace(_topVerts, 24, (int)block.waveEffect, glm::ivec3(mi.x, mi.y, mi.z), mi.topIndex, textureIndex, overlayTextureIndex, color, overlayColor, ambientOcclusion, block.pyTexInfo);

            //Check for +x direction merging
            _topVerts[mi.topIndex].light = calculateSmoothLighting(lights[17] + lights[18] + lights[20] + lights[21] + nearBlocks[0], nearBlocks[0]);
            _topVerts[mi.topIndex + 1].light = calculateSmoothLighting(lights[20] + lights[21] + lights[23] + lights[24] + nearBlocks[1], nearBlocks[1]);
            _topVerts[mi.topIndex + 2].light = calculateSmoothLighting(lights[21] + lights[22] + lights[24] + lights[25] + nearBlocks[2], nearBlocks[2]);
            _topVerts[mi.topIndex + 3].light = calculateSmoothLighting(lights[18] + lights[19] + lights[21] + lights[22] + nearBlocks[3], nearBlocks[3]);
            _topVerts[mi.topIndex].sunLight = calculateSmoothLighting(sunlights[17] + sunlights[18] + sunlights[20] + sunlights[21] + nearBlocks[0], nearBlocks[0]);
            _topVerts[mi.topIndex + 1].sunLight = calculateSmoothLighting(sunlights[20] + sunlights[21] + sunlights[23] + sunlights[24] + nearBlocks[1], nearBlocks[1]);
            _topVerts[mi.topIndex + 2].sunLight = calculateSmoothLighting(sunlights[21] + sunlights[22] + sunlights[24] + sunlights[25] + nearBlocks[2], nearBlocks[2]);
            _topVerts[mi.topIndex + 3].sunLight = calculateSmoothLighting(sunlights[18] + sunlights[19] + sunlights[21] + sunlights[22] + nearBlocks[3], nearBlocks[3]);

            //to check for a +x merge, we check that the vertices aligned in the direction of stretch are equal
            if (mi.mergeUp && mi.pbtype == btype &&
                CompareVertices(_topVerts[mi.pupIndex], _topVerts[mi.pupIndex + 3]) && CompareVertices(_topVerts[mi.pupIndex + 3], _topVerts[mi.topIndex]) && CompareVertices(_topVerts[mi.topIndex], _topVerts[mi.topIndex + 3]) &&//-z vertices
                CompareVertices(_topVerts[mi.pupIndex + 1], _topVerts[mi.pupIndex + 2]) && CompareVertices(_topVerts[mi.pupIndex + 2], _topVerts[mi.topIndex + 1]) && CompareVertices(_topVerts[mi.topIndex + 1], _topVerts[mi.topIndex + 2])){ //+z vertices
                _topVerts[mi.pupIndex + 2].position.x += 7;					//change x
                _topVerts[mi.pupIndex + 3].position.x += 7;
                _topVerts[mi.pupIndex + 2].tex[0]++;
                _topVerts[mi.pupIndex + 3].tex[0]++;
            } else{
                mi.pupIndex = mi.topIndex;
                mi.topIndex += 4;
                mi.mergeUp = true;
            }
        }
	}
	else{
		mi.mergeUp = false;
	}

    if (faces[YNEG]) {
        Blocks[btype].GetBlockColor(color, overlayColor, flags, mi.temperature, mi.rainfall, block.nyTexInfo);

        nearBlocks[0] = (int)(lights[1] == -1) + (int)(lights[2] == -1) + (int)(lights[5] == -1);
        nearBlocks[1] = (int)(lights[5] == -1) + (int)(lights[7] == -1) + (int)(lights[8] == -1);
        nearBlocks[2] = (int)(lights[3] == -1) + (int)(lights[6] == -1) + (int)(lights[7] == -1);
        nearBlocks[3] = (int)(lights[0] == -1) + (int)(lights[1] == -1) + (int)(lights[3] == -1);
        ambientOcclusion[0] = 1.0f - nearBlocks[0] * OCCLUSION_FACTOR;
        ambientOcclusion[1] = 1.0f - nearBlocks[1] * OCCLUSION_FACTOR;
        ambientOcclusion[2] = 1.0f - nearBlocks[2] * OCCLUSION_FACTOR;
        ambientOcclusion[3] = 1.0f - nearBlocks[3] * OCCLUSION_FACTOR;
       
        textureIndex = block.nyTexInfo.base.textureIndex;
        overlayTextureIndex = block.nyTexInfo.overlay.textureIndex;

        //We will offset the texture index based on the texture method
        getTextureIndex(mi, block.nyTexInfo.base, textureIndex, -1, -PADDED_WIDTH, -PADDED_LAYER, 4, color);
        getTextureIndex(mi, block.nyTexInfo.overlay, overlayTextureIndex, -1, -PADDED_WIDTH, -PADDED_LAYER, 10, overlayColor);

        if (block.nyTexInfo.base.transparency) {
            _nonBlockVerts.resize(_nonBlockVerts.size() + 4);
            makeFloraFace(&(_nonBlockVerts[0]), &(cubeVertices[0]), &(cubeNormals[0]), 48, block.waveEffect, i32v3(mi.x, mi.y, mi.z), mi.nbIndex, textureIndex, overlayTextureIndex, color, overlayColor, light, block.nyTexInfo);
            mi.nbIndex += 4;

            chunkMeshData->addTransQuad(i8v3(mi.x2 + 1, mi.y2, mi.z2 + 1));

            mi.mergeBot = false;
        } else {

            makeCubeFace(_bottomVerts, 48, (int)block.waveEffect, glm::ivec3(mi.x, mi.y, mi.z), mi.botIndex, textureIndex, overlayTextureIndex, color, overlayColor, ambientOcclusion, block.nyTexInfo);

            _bottomVerts[mi.botIndex].light = calculateSmoothLighting(lights[1] + lights[2] + lights[4] + lights[5] + nearBlocks[0], nearBlocks[0]);
            _bottomVerts[mi.botIndex + 1].light = calculateSmoothLighting(lights[4] + lights[5] + lights[7] + lights[8] + nearBlocks[1], nearBlocks[1]);
            _bottomVerts[mi.botIndex + 2].light = calculateSmoothLighting(lights[3] + lights[4] + lights[6] + lights[7] + nearBlocks[2], nearBlocks[2]);
            _bottomVerts[mi.botIndex + 3].light = calculateSmoothLighting(lights[0] + lights[1] + lights[3] + lights[4] + nearBlocks[3], nearBlocks[3]);
            _bottomVerts[mi.botIndex].sunLight = calculateSmoothLighting(sunlights[1] + sunlights[2] + sunlights[4] + sunlights[5] + nearBlocks[0], nearBlocks[0]);
            _bottomVerts[mi.botIndex + 1].sunLight = calculateSmoothLighting(sunlights[4] + sunlights[5] + sunlights[7] + sunlights[8] + nearBlocks[1], nearBlocks[1]);
            _bottomVerts[mi.botIndex + 2].sunLight = calculateSmoothLighting(sunlights[3] + sunlights[4] + sunlights[6] + sunlights[7] + nearBlocks[2], nearBlocks[2]);
            _bottomVerts[mi.botIndex + 3].sunLight = calculateSmoothLighting(sunlights[0] + sunlights[1] + sunlights[3] + sunlights[4] + nearBlocks[3], nearBlocks[3]);

            //to check for a +x merge, we check that the vertices aligned in the direction of stretch are equal
            if (mi.mergeBot && mi.pbtype == btype &&
                CompareVertices(_bottomVerts[mi.pbotIndex], _bottomVerts[mi.pbotIndex + 3]) && CompareVertices(_bottomVerts[mi.pbotIndex + 3], _bottomVerts[mi.botIndex]) && CompareVertices(_bottomVerts[mi.botIndex], _bottomVerts[mi.botIndex + 3]) &&//-z vertices
                CompareVertices(_bottomVerts[mi.pbotIndex + 1], _bottomVerts[mi.pbotIndex + 2]) && CompareVertices(_bottomVerts[mi.pbotIndex + 2], _bottomVerts[mi.botIndex + 1]) && CompareVertices(_bottomVerts[mi.botIndex + 1], _bottomVerts[mi.botIndex + 2])){ //+z vertices
                _bottomVerts[mi.pbotIndex].position.x += 7;					//change x
                _bottomVerts[mi.pbotIndex + 1].position.x += 7;
                _bottomVerts[mi.pbotIndex].tex[0]--;
                _bottomVerts[mi.pbotIndex + 1].tex[0]--;
            } else {
                mi.pbotIndex = mi.botIndex;
                mi.botIndex += 4;
                mi.mergeBot = 1;
            }
        }
    } else{
        mi.mergeBot = 0;
    }

    if (faces[XPOS]) {
        Blocks[btype].GetBlockColor(color, overlayColor, flags, mi.temperature, mi.rainfall, block.pxTexInfo);

        nearBlocks[0] = (int)(lights[25] == -1) + (int)(lights[22] == -1) + (int)(lights[16] == -1);
        nearBlocks[1] = (int)(lights[5] == -1) + (int)(lights[8] == -1) + (int)(lights[16] == -1);
        nearBlocks[2] = (int)(lights[2] == -1) + (int)(lights[5] == -1) + (int)(lights[11] == -1);
        nearBlocks[3] = (int)(lights[19] == -1) + (int)(lights[22] == -1) + (int)(lights[11] == -1);
        ambientOcclusion[0] = 1.0f - nearBlocks[0] * OCCLUSION_FACTOR;
        ambientOcclusion[1] = 1.0f - nearBlocks[1] * OCCLUSION_FACTOR;
        ambientOcclusion[2] = 1.0f - nearBlocks[2] * OCCLUSION_FACTOR;
        ambientOcclusion[3] = 1.0f - nearBlocks[3] * OCCLUSION_FACTOR;
      
        textureIndex = block.pxTexInfo.base.textureIndex;
        overlayTextureIndex = block.pxTexInfo.overlay.textureIndex;

        //We will offset the texture index based on the texture method
        getTextureIndex(mi, block.pxTexInfo.base, textureIndex, -PADDED_WIDTH, PADDED_LAYER, 1, 0, color);
        getTextureIndex(mi, block.pxTexInfo.overlay, overlayTextureIndex, -PADDED_WIDTH, PADDED_LAYER, 1, 6, overlayColor);


        if (block.pxTexInfo.base.transparency) {  
            _nonBlockVerts.resize(_nonBlockVerts.size() + 4);
            makeFloraFace(&(_nonBlockVerts[0]), &(cubeVertices[0]), &(cubeNormals[0]), 12, block.waveEffect, i32v3(mi.x, mi.y, mi.z), mi.nbIndex, textureIndex, overlayTextureIndex, color, overlayColor, light, block.pxTexInfo);
            mi.nbIndex += 4;

            chunkMeshData->addTransQuad(i8v3(mi.x2 + 2, mi.y2 + 1, mi.z2 + 1));
        } else {
            makeCubeFace(_rightVerts, 12, (int)block.waveEffect, glm::ivec3(mi.x, mi.y, mi.z), mi.rightIndex, textureIndex, overlayTextureIndex, color, overlayColor, ambientOcclusion, block.pxTexInfo);

            _rightVerts[mi.rightIndex].light = calculateSmoothLighting(lights[25] + lights[22] + lights[13] + lights[16] + nearBlocks[0], nearBlocks[0]);
            _rightVerts[mi.rightIndex + 1].light = calculateSmoothLighting(lights[5] + lights[8] + lights[13] + lights[16] + nearBlocks[1], nearBlocks[1]);
            _rightVerts[mi.rightIndex + 2].light = calculateSmoothLighting(lights[2] + lights[5] + lights[11] + lights[13] + nearBlocks[2], nearBlocks[2]);
            _rightVerts[mi.rightIndex + 3].light = calculateSmoothLighting(lights[19] + lights[22] + lights[11] + lights[13] + nearBlocks[3], nearBlocks[3]);
            _rightVerts[mi.rightIndex].sunLight = calculateSmoothLighting(sunlights[25] + sunlights[22] + sunlights[13] + sunlights[16] + nearBlocks[0], nearBlocks[0]);
            _rightVerts[mi.rightIndex + 1].sunLight = calculateSmoothLighting(sunlights[5] + sunlights[8] + sunlights[13] + sunlights[16] + nearBlocks[1], nearBlocks[1]);
            _rightVerts[mi.rightIndex + 2].sunLight = calculateSmoothLighting(sunlights[2] + sunlights[5] + sunlights[11] + sunlights[13] + nearBlocks[2], nearBlocks[2]);
            _rightVerts[mi.rightIndex + 3].sunLight = calculateSmoothLighting(sunlights[19] + sunlights[22] + sunlights[11] + sunlights[13] + nearBlocks[3], nearBlocks[3]);

            mi.rightIndex += 4;
        }
    }

    if (faces[XNEG]) {
        Blocks[btype].GetBlockColor(color, overlayColor, flags, mi.temperature, mi.rainfall, block.nxTexInfo);

        nearBlocks[0] = (int)(lights[17] == -1) + (int)(lights[20] == -1) + (int)(lights[9] == -1);
        nearBlocks[1] = (int)(lights[0] == -1) + (int)(lights[3] == -1) + (int)(lights[9] == -1);
        nearBlocks[2] = (int)(lights[3] == -1) + (int)(lights[6] == -1) + (int)(lights[14] == -1);
        nearBlocks[3] = (int)(lights[20] == -1) + (int)(lights[23] == -1) + (int)(lights[14] == -1);
        ambientOcclusion[0] = 1.0f - nearBlocks[0] * OCCLUSION_FACTOR;
        ambientOcclusion[1] = 1.0f - nearBlocks[1] * OCCLUSION_FACTOR;
        ambientOcclusion[2] = 1.0f - nearBlocks[2] * OCCLUSION_FACTOR;
        ambientOcclusion[3] = 1.0f - nearBlocks[3] * OCCLUSION_FACTOR;

        textureIndex = block.nxTexInfo.base.textureIndex;
        overlayTextureIndex = block.nxTexInfo.overlay.textureIndex;

        //We will offset the texture index based on the texture method
        getTextureIndex(mi, block.nxTexInfo.base, textureIndex, PADDED_WIDTH, PADDED_LAYER, -1, 3, color);
        getTextureIndex(mi, block.nxTexInfo.overlay, overlayTextureIndex, PADDED_WIDTH, PADDED_LAYER, -1, 9, overlayColor);

        if (block.nxTexInfo.base.transparency) {
            _nonBlockVerts.resize(_nonBlockVerts.size() + 4);
            makeFloraFace(&(_nonBlockVerts[0]), &(cubeVertices[0]), &(cubeNormals[0]), 36, block.waveEffect, i32v3(mi.x, mi.y, mi.z), mi.nbIndex, textureIndex, overlayTextureIndex, color, overlayColor, light, block.nxTexInfo);
            mi.nbIndex += 4;

            chunkMeshData->addTransQuad(i8v3(mi.x2, mi.y2 + 1, mi.z2 + 1));
        } else {
            makeCubeFace(_leftVerts, 36, (int)block.waveEffect, glm::ivec3(mi.x, mi.y, mi.z), mi.leftIndex, textureIndex, overlayTextureIndex, color, overlayColor, ambientOcclusion, block.nxTexInfo);

            _leftVerts[mi.leftIndex].light = calculateSmoothLighting(lights[17] + lights[20] + lights[9] + lights[12] + nearBlocks[0], nearBlocks[0]);
            _leftVerts[mi.leftIndex + 1].light = calculateSmoothLighting(lights[0] + lights[3] + lights[9] + lights[12] + nearBlocks[1], nearBlocks[1]);
            _leftVerts[mi.leftIndex + 2].light = calculateSmoothLighting(lights[3] + lights[6] + lights[14] + lights[12] + nearBlocks[2], nearBlocks[2]);
            _leftVerts[mi.leftIndex + 3].light = calculateSmoothLighting(lights[20] + lights[23] + lights[14] + lights[12] + nearBlocks[3], nearBlocks[3]);
            _leftVerts[mi.leftIndex].sunLight = calculateSmoothLighting(sunlights[17] + sunlights[20] + sunlights[9] + sunlights[12] + nearBlocks[0], nearBlocks[0]);
            _leftVerts[mi.leftIndex + 1].sunLight = calculateSmoothLighting(sunlights[0] + sunlights[3] + sunlights[9] + sunlights[12] + nearBlocks[1], nearBlocks[1]);
            _leftVerts[mi.leftIndex + 2].sunLight = calculateSmoothLighting(sunlights[3] + sunlights[6] + sunlights[14] + sunlights[12] + nearBlocks[2], nearBlocks[2]);
            _leftVerts[mi.leftIndex + 3].sunLight = calculateSmoothLighting(sunlights[20] + sunlights[23] + sunlights[14] + sunlights[12] + nearBlocks[3], nearBlocks[3]);

            mi.leftIndex += 4;
        }
    }

}

//adds a flora mesh
void ChunkMesher::addFloraToMesh(MeshInfo& mi) {

    const Block &block = Blocks[mi.btype];

    GLubyte color[3], overlayColor[3];
    const int wc = mi.wc;
    const int btype = mi.btype;
    
    const float lightMult = 0.95f, lightOff = -0.19f;

    Biome *biome = GameManager::planet->allBiomesLookupVector[mi.task->biomes[mi.z*CHUNK_WIDTH + mi.x]];
    int temperature = mi.task->temperatures[mi.z*CHUNK_WIDTH + mi.x];
    int rainfall = mi.task->rainfalls[mi.z*CHUNK_WIDTH + mi.x];
    GLuint flags = GETFLAGS(mi.task->chData[mi.wc]);

    GLubyte light[2];
    light[0] = mi.task->chLightData[0][wc];
    light[1] = mi.task->chLightData[1][wc];

    light[0] = (GLubyte)(255.0f*(lightOff + pow(lightMult, MAXLIGHT - light[0])));
    light[1] = (GLubyte)(255.0f*(lightOff + pow(lightMult, MAXLIGHT - light[1])));

    Blocks[btype].GetBlockColor(color, overlayColor, flags, temperature, rainfall, block.pxTexInfo);

    i32 textureIndex = block.pxTexInfo.base.textureIndex;
    i32 overlayTextureIndex = block.pxTexInfo.overlay.textureIndex;

    //We will offset the texture index based on the texture method
    getTextureIndex(mi, block.pxTexInfo.base, textureIndex, -PADDED_WIDTH, PADDED_LAYER, 1, 0, color);
    getTextureIndex(mi, block.pxTexInfo.overlay, overlayTextureIndex, -PADDED_WIDTH, PADDED_LAYER, 1, 6, overlayColor);

    int r;

    switch (mi.meshType){
        case MeshType::LEAVES:

            break;
        case MeshType::CROSSFLORA:
            //Generate a random number between 0 and 3 inclusive
            r = std::bind(std::uniform_int_distribution<int>(0, 1), mt19937(getPositionSeed(mi.x, mi.z)))();

            _nonBlockVerts.resize(_nonBlockVerts.size() + 8);
            makeFloraFace(&(_nonBlockVerts[0]), &(crossFloraVertices[r][0]), &(floraNormals[0]), 0, block.waveEffect, i32v3(mi.x, mi.y, mi.z), mi.nbIndex, textureIndex, overlayTextureIndex, color, overlayColor, light, block.pxTexInfo);
            mi.nbIndex += 4;
            makeFloraFace(&(_nonBlockVerts[0]), &(crossFloraVertices[r][0]), &(floraNormals[0]), 12, block.waveEffect, i32v3(mi.x, mi.y, mi.z), mi.nbIndex, textureIndex, overlayTextureIndex, color, overlayColor, light, block.pxTexInfo);
            mi.nbIndex += 4;

            chunkMeshData->addTransQuad(i8v3(mi.x2, mi.y2, mi.z2));
            chunkMeshData->addTransQuad(i8v3(mi.x2, mi.y2, mi.z2));
            break;
        case MeshType::FLORA:
            //Generate a random number between 0 and 3 inclusive
            r = std::bind(std::uniform_int_distribution<int>(0, 3), mt19937(getPositionSeed(mi.x, mi.z)))();

            _nonBlockVerts.resize(_nonBlockVerts.size() + 12);
            makeFloraFace(&(_nonBlockVerts[0]), &(floraVertices[r][0]), &(floraNormals[0]), 0, block.waveEffect, i32v3(mi.x, mi.y, mi.z), mi.nbIndex, textureIndex, overlayTextureIndex, color, overlayColor, light, block.pxTexInfo);
            mi.nbIndex += 4;
            makeFloraFace(&(_nonBlockVerts[0]), &(floraVertices[r][0]), &(floraNormals[0]), 12, block.waveEffect, i32v3(mi.x, mi.y, mi.z), mi.nbIndex, textureIndex, overlayTextureIndex, color, overlayColor, light, block.pxTexInfo);
            mi.nbIndex += 4;
            makeFloraFace(&(_nonBlockVerts[0]), &(floraVertices[r][0]), &(floraNormals[0]), 24, block.waveEffect, i32v3(mi.x, mi.y, mi.z), mi.nbIndex, textureIndex, overlayTextureIndex, color, overlayColor, light, block.pxTexInfo);
            mi.nbIndex += 4;

            chunkMeshData->addTransQuad(i8v3(mi.x2, mi.y2, mi.z2));
            chunkMeshData->addTransQuad(i8v3(mi.x2, mi.y2, mi.z2));
            chunkMeshData->addTransQuad(i8v3(mi.x2, mi.y2, mi.z2));
            break;
    }
}

//Gets the liquid level from a block index
#define LEVEL(i) ((task->chData[i] == 0) ? 0 : (((nextBlock = &GETBLOCK(task->chData[i]))->physicsProperty == block.physicsProperty) ? nextBlock->waterMeshLevel : 0))

#define CALCULATE_LIQUID_VERTEX_HEIGHT(height, heightA, heightB, cornerIndex) \
    div = 0; \
    tot = 0; \
    if (heightA) { \
        tot += heightA; \
        div++; \
    } \
    \
    if (heightB) { \
        tot += heightB; \
        div++; \
    } \
    \
    if (div) { \
        int lvl = LEVEL(cornerIndex); \
        if (lvl) { \
            tot += lvl; \
            div++; \
        } \
        height = (height + (tot / (float)maxLevel)) / (div + 1); \
    } 
    
   
//END CALCULATE_LIQUID_VERTEX_HEIGHT

void ChunkMesher::addLiquidToMesh(MeshInfo& mi) {

    const Block &block = Blocks[mi.btype];
    Block* nextBlock;
    i32 nextBlockID;
    const i32 wc = mi.wc;
    i32 x = mi.x;
    i32 y = mi.y;
    i32 z = mi.z;

    RenderTask* task = mi.task;

    const i32 maxLevel = 100;

    float liquidLevel = block.waterMeshLevel / (float)maxLevel;
    float fallingReduction = 0.0f;

    bool faces[6] = { false, false, false, false, false, false };

    float backLeftHeight = liquidLevel;
    float backRightHeight = liquidLevel;
    float frontRightHeight = liquidLevel;
    float frontLeftHeight = liquidLevel;

    const i32 MIN_ALPHA = 75;
    const i32 MAX_ALPHA = 175;
    const i32 ALPHA_RANGE = MAX_ALPHA - MIN_ALPHA;

    ui8 backRightAlpha, frontRightAlpha, frontLeftAlpha, backLeftAlpha;

    i32 textureUnit = 0;

    i32 div;
    i32 tot;

    i32 left, right, back, front, bottom;

    ui8 uOff = x * 7;
    ui8 vOff = 224 - z * 7;

    ui8 temperature = task->temperatures[x + z*CHUNK_WIDTH];
    ui8 depth = task->depthMap[x + z*CHUNK_WIDTH];

    ui8 color[3];
    color[0] = waterColorMap[depth][temperature][0];
    color[1] = waterColorMap[depth][temperature][1];
    color[2] = waterColorMap[depth][temperature][2];

    ui8 light[2];
    light[0] = (ui8)(255.0f * (LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - (int)(task->chLightData[0][wc]))));
    light[1] = (ui8)(255.0f * (LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - (int)(task->chLightData[1][wc]))));

    nextBlockID = task->chData[wc + PADDED_OFFSETS::BOTTOM];
    nextBlock = &GETBLOCK(nextBlockID);
    //Check if the block is falling
    if (nextBlockID == 0 || nextBlock->waterBreak || (nextBlock->physicsProperty == block.physicsProperty && nextBlock->waterMeshLevel != maxLevel)) {
        memset(faces, 1, 6); //all faces are active
  //      backLeftHeight = backRightHeight = frontLeftHeight = frontRightHeight 
        fallingReduction = 1.0f;
    } else {

        //Get occlusion
        nextBlock = &GETBLOCK(task->chData[wc + PADDED_OFFSETS::LEFT]);
        faces[XNEG] = ((nextBlock->physicsProperty != block.physicsProperty) && !nextBlock->occlude);

        nextBlock = &GETBLOCK(task->chData[wc + PADDED_OFFSETS::BACK]);
        faces[ZNEG] = ((nextBlock->physicsProperty != block.physicsProperty) && !nextBlock->occlude);

        nextBlock = &GETBLOCK(task->chData[wc + PADDED_OFFSETS::RIGHT]);
        faces[XPOS] = ((nextBlock->physicsProperty != block.physicsProperty) && !nextBlock->occlude);

        nextBlock = &GETBLOCK(task->chData[wc + PADDED_OFFSETS::FRONT]);
        faces[ZPOS] = ((nextBlock->physicsProperty != block.physicsProperty) && !nextBlock->occlude);

        nextBlock = &GETBLOCK(task->chData[wc + PADDED_OFFSETS::BOTTOM]);
        faces[YNEG] = ((nextBlock->physicsProperty != block.physicsProperty) && !nextBlock->occlude);
    }

    left = LEVEL(wc + PADDED_OFFSETS::LEFT);
    right = LEVEL(wc + PADDED_OFFSETS::RIGHT);
    back = LEVEL(wc + PADDED_OFFSETS::BACK);
    front = LEVEL(wc + PADDED_OFFSETS::FRONT);
    bottom = LEVEL(wc + PADDED_OFFSETS::BOTTOM);

    //Calculate the liquid levels

    //Back Left Vertex
    CALCULATE_LIQUID_VERTEX_HEIGHT(backLeftHeight, left, back, wc + PADDED_OFFSETS::BACK_LEFT);

    //Back Right Vertex
    CALCULATE_LIQUID_VERTEX_HEIGHT(backRightHeight, right, back, wc + PADDED_OFFSETS::BACK_RIGHT);

    //Front Right Vertex
    CALCULATE_LIQUID_VERTEX_HEIGHT(frontRightHeight, right, front, wc + PADDED_OFFSETS::FRONT_RIGHT);

    //Front Left Vertex
    CALCULATE_LIQUID_VERTEX_HEIGHT(frontLeftHeight, left, front, wc + PADDED_OFFSETS::FRONT_LEFT);

    //only occlude top if we are a full water block and our sides arent down at all
    if (liquidLevel == 1.0f && backRightHeight == 1.0f && backLeftHeight == 1.0f && frontLeftHeight == 1.0f && frontRightHeight == 1.0f) {
        nextBlock = &GETBLOCK(task->chData[wc + PADDED_OFFSETS::TOP]);
        faces[YPOS] = ((nextBlock->physicsProperty != block.physicsProperty) && !nextBlock->occlude);
    } else {
        faces[YPOS] = true;
    }
    
    //Compute alpha
    if (bottom == maxLevel) {
        backRightAlpha = backLeftAlpha = frontRightAlpha = frontLeftAlpha = MAX_ALPHA;
    } else {
        backRightAlpha = backRightHeight * ALPHA_RANGE + MIN_ALPHA;
        backLeftAlpha = backLeftHeight * ALPHA_RANGE + MIN_ALPHA;
        frontRightAlpha = frontRightHeight * ALPHA_RANGE + MIN_ALPHA;
        frontLeftAlpha = frontLeftHeight * ALPHA_RANGE + MIN_ALPHA;
    }

    //Add vertices for the faces
    if (faces[YNEG]){

        makeLiquidFace(_waterVboVerts, mi.liquidIndex, uOff, vOff, light, color, textureUnit);
        
        _waterVboVerts[mi.liquidIndex].position[0] = x + liquidVertices[48];
        _waterVboVerts[mi.liquidIndex].position[1] = y + liquidVertices[49] - fallingReduction;
        _waterVboVerts[mi.liquidIndex].position[2] = z + liquidVertices[50];
        _waterVboVerts[mi.liquidIndex + 1].position[0] = x + liquidVertices[51];
        _waterVboVerts[mi.liquidIndex + 1].position[1] = y + liquidVertices[52] - fallingReduction;
        _waterVboVerts[mi.liquidIndex + 1].position[2] = z + liquidVertices[53];
        _waterVboVerts[mi.liquidIndex + 2].position[0] = x + liquidVertices[54];
        _waterVboVerts[mi.liquidIndex + 2].position[1] = y + liquidVertices[55] - fallingReduction;
        _waterVboVerts[mi.liquidIndex + 2].position[2] = z + liquidVertices[56];
        _waterVboVerts[mi.liquidIndex + 3].position[0] = x + liquidVertices[57];
        _waterVboVerts[mi.liquidIndex + 3].position[1] = y + liquidVertices[58] - fallingReduction;
        _waterVboVerts[mi.liquidIndex + 3].position[2] = z + liquidVertices[59];

        //set alpha
        _waterVboVerts[mi.liquidIndex].color[3] = backLeftAlpha;
        _waterVboVerts[mi.liquidIndex + 1].color[3] = backRightAlpha;
        _waterVboVerts[mi.liquidIndex + 2].color[3] = frontRightAlpha;
        _waterVboVerts[mi.liquidIndex + 3].color[3] = backLeftAlpha;

        mi.liquidIndex += 4;
    }

    if (faces[ZPOS]){

        makeLiquidFace(_waterVboVerts, mi.liquidIndex, uOff, vOff, light, color, textureUnit);

        _waterVboVerts[mi.liquidIndex].position[0] = x + liquidVertices[0];
        _waterVboVerts[mi.liquidIndex].position[1] = y + frontLeftHeight;
        _waterVboVerts[mi.liquidIndex].position[2] = z + liquidVertices[2];
        _waterVboVerts[mi.liquidIndex + 1].position[0] = x + liquidVertices[3];
        _waterVboVerts[mi.liquidIndex + 1].position[1] = y + liquidVertices[4] - fallingReduction;
        _waterVboVerts[mi.liquidIndex + 1].position[2] = z + liquidVertices[5];
        _waterVboVerts[mi.liquidIndex + 2].position[0] = x + liquidVertices[6];
        _waterVboVerts[mi.liquidIndex + 2].position[1] = y + liquidVertices[7] - fallingReduction;
        _waterVboVerts[mi.liquidIndex + 2].position[2] = z + liquidVertices[8];
        _waterVboVerts[mi.liquidIndex + 3].position[0] = x + liquidVertices[9];
        _waterVboVerts[mi.liquidIndex + 3].position[1] = y + frontRightHeight;
        _waterVboVerts[mi.liquidIndex + 3].position[2] = z + liquidVertices[11];

        _waterVboVerts[mi.liquidIndex].color[3] = frontLeftAlpha;
        _waterVboVerts[mi.liquidIndex + 1].color[3] = frontLeftAlpha;
        _waterVboVerts[mi.liquidIndex + 2].color[3] = frontRightAlpha;
        _waterVboVerts[mi.liquidIndex + 3].color[3] = frontRightAlpha;

        mi.liquidIndex += 4;
    }

    if (faces[YPOS]){

        makeLiquidFace(_waterVboVerts, mi.liquidIndex, uOff, vOff, light, color, textureUnit);

        _waterVboVerts.resize(_waterVboVerts.size() + 4);
        _waterVboVerts[mi.liquidIndex].position[0] = x + liquidVertices[24];
        _waterVboVerts[mi.liquidIndex].position[1] = y + backLeftHeight;
        _waterVboVerts[mi.liquidIndex].position[2] = z + liquidVertices[26];
        _waterVboVerts[mi.liquidIndex + 1].position[0] = x + liquidVertices[27];
        _waterVboVerts[mi.liquidIndex + 1].position[1] = y + frontLeftHeight;
        _waterVboVerts[mi.liquidIndex + 1].position[2] = z + liquidVertices[29];
        _waterVboVerts[mi.liquidIndex + 2].position[0] = x + liquidVertices[30];
        _waterVboVerts[mi.liquidIndex + 2].position[1] = y + frontRightHeight;
        _waterVboVerts[mi.liquidIndex + 2].position[2] = z + liquidVertices[32];
        _waterVboVerts[mi.liquidIndex + 3].position[0] = x + liquidVertices[33];
        _waterVboVerts[mi.liquidIndex + 3].position[1] = y + backRightHeight;
        _waterVboVerts[mi.liquidIndex + 3].position[2] = z + liquidVertices[35];

        _waterVboVerts[mi.liquidIndex].color[3] = backLeftAlpha;
        _waterVboVerts[mi.liquidIndex + 1].color[3] = frontLeftAlpha;
        _waterVboVerts[mi.liquidIndex + 2].color[3] = frontRightAlpha;
        _waterVboVerts[mi.liquidIndex + 3].color[3] = backRightAlpha;

        mi.liquidIndex += 4;
    }

    if (faces[ZNEG]){

        makeLiquidFace(_waterVboVerts, mi.liquidIndex, uOff, vOff, light, color, textureUnit);

        _waterVboVerts[mi.liquidIndex].position[0] = x + liquidVertices[60];
        _waterVboVerts[mi.liquidIndex].position[1] = y + backRightHeight;
        _waterVboVerts[mi.liquidIndex].position[2] = z + liquidVertices[62];
        _waterVboVerts[mi.liquidIndex + 1].position[0] = x + liquidVertices[63];
        _waterVboVerts[mi.liquidIndex + 1].position[1] = y + liquidVertices[64] - fallingReduction;
        _waterVboVerts[mi.liquidIndex + 1].position[2] = z + liquidVertices[65];
        _waterVboVerts[mi.liquidIndex + 2].position[0] = x + liquidVertices[66];
        _waterVboVerts[mi.liquidIndex + 2].position[1] = y + liquidVertices[67] - fallingReduction;
        _waterVboVerts[mi.liquidIndex + 2].position[2] = z + liquidVertices[68];
        _waterVboVerts[mi.liquidIndex + 3].position[0] = x + liquidVertices[69];
        _waterVboVerts[mi.liquidIndex + 3].position[1] = y + backLeftHeight;
        _waterVboVerts[mi.liquidIndex + 3].position[2] = z + liquidVertices[71];

        _waterVboVerts[mi.liquidIndex].color[3] = backRightAlpha;
        _waterVboVerts[mi.liquidIndex + 1].color[3] = backRightAlpha;
        _waterVboVerts[mi.liquidIndex + 2].color[3] = backLeftAlpha;
        _waterVboVerts[mi.liquidIndex + 3].color[3] = backLeftAlpha;

        mi.liquidIndex += 4;
    }
    if (faces[XPOS]){

        makeLiquidFace(_waterVboVerts, mi.liquidIndex, uOff, vOff, light, color, textureUnit);

        _waterVboVerts[mi.liquidIndex].position.x = x + liquidVertices[12];
        _waterVboVerts[mi.liquidIndex].position.y = y + frontRightHeight;
        _waterVboVerts[mi.liquidIndex].position.z = z + liquidVertices[14];
        _waterVboVerts[mi.liquidIndex + 1].position.x = x + liquidVertices[15];
        _waterVboVerts[mi.liquidIndex + 1].position.y = y + liquidVertices[16] - fallingReduction;
        _waterVboVerts[mi.liquidIndex + 1].position.z = z + liquidVertices[17];
        _waterVboVerts[mi.liquidIndex + 2].position.x = x + liquidVertices[18];
        _waterVboVerts[mi.liquidIndex + 2].position.y = y + liquidVertices[19] - fallingReduction;
        _waterVboVerts[mi.liquidIndex + 2].position.z = z + liquidVertices[20];
        _waterVboVerts[mi.liquidIndex + 3].position.x = x + liquidVertices[21];
        _waterVboVerts[mi.liquidIndex + 3].position.y = y + backRightHeight;
        _waterVboVerts[mi.liquidIndex + 3].position.z = z + liquidVertices[23];

        _waterVboVerts[mi.liquidIndex].color[3] = frontRightAlpha;
        _waterVboVerts[mi.liquidIndex + 1].color[3] = frontRightAlpha;
        _waterVboVerts[mi.liquidIndex + 2].color[3] = backRightAlpha;
        _waterVboVerts[mi.liquidIndex + 3].color[3] = backRightAlpha;

        mi.liquidIndex += 4;
    }
    if (faces[XNEG]){

        makeLiquidFace(_waterVboVerts, mi.liquidIndex, uOff, vOff, light, color, textureUnit);

        _waterVboVerts[mi.liquidIndex].position[0] = x + liquidVertices[36];
        _waterVboVerts[mi.liquidIndex].position[1] = y + backLeftHeight;
        _waterVboVerts[mi.liquidIndex].position[2] = z + liquidVertices[38];
        _waterVboVerts[mi.liquidIndex + 1].position[0] = x + liquidVertices[39];
        _waterVboVerts[mi.liquidIndex + 1].position[1] = y + liquidVertices[40] - fallingReduction;
        _waterVboVerts[mi.liquidIndex + 1].position[2] = z + liquidVertices[41];
        _waterVboVerts[mi.liquidIndex + 2].position[0] = x + liquidVertices[42];
        _waterVboVerts[mi.liquidIndex + 2].position[1] = y + liquidVertices[43] - fallingReduction;
        _waterVboVerts[mi.liquidIndex + 2].position[2] = z + liquidVertices[44];
        _waterVboVerts[mi.liquidIndex + 3].position[0] = x + liquidVertices[45];
        _waterVboVerts[mi.liquidIndex + 3].position[1] = y + frontLeftHeight;
        _waterVboVerts[mi.liquidIndex + 3].position[2] = z + liquidVertices[47];

        _waterVboVerts[mi.liquidIndex].color[3] = backLeftAlpha;
        _waterVboVerts[mi.liquidIndex + 1].color[3] = backLeftAlpha;
        _waterVboVerts[mi.liquidIndex + 2].color[3] = frontLeftAlpha;
        _waterVboVerts[mi.liquidIndex + 3].color[3] = frontLeftAlpha;

        mi.liquidIndex += 4;
    }
}

void ChunkMesher::mergeTopVerts(MeshInfo &mi)
{
    if (mi.topIndex == 0) return;

    int qi;

    if (_finalTopVerts.size() != 131072) _finalTopVerts.resize(131072);

    for (int i = 0; i < mi.topIndex; i += 4) {
        // Early exit if it cant merge
        if (_topVerts[i].merge == -1) {
            continue;
        } else {
            //Use this quad in the final mesh
            qi = mi.pyVboSize;
            _finalTopVerts[mi.pyVboSize++] = _topVerts[i];
            _finalTopVerts[mi.pyVboSize++] = _topVerts[i + 1];
            _finalTopVerts[mi.pyVboSize++] = _topVerts[i + 2];
            _finalTopVerts[mi.pyVboSize++] = _topVerts[i + 3];
        }
        if (_finalTopVerts[qi].merge != 0 && CompareVerticesLight(_finalTopVerts[qi], _finalTopVerts[qi + 1]) && CompareVerticesLight(_finalTopVerts[qi + 2], _finalTopVerts[qi + 3])) {
            // Search for the next mergeable quad
            for (int j = i + 4; j < mi.topIndex; j += 4) {
                // Skip if the J index is not active for merging
                if (_topVerts[j].merge < 1) continue;

                // Early exit if the J index cant be merged
                if (CompareVerticesLight(_topVerts[j], _topVerts[j + 1]) && CompareVerticesLight(_topVerts[j + 2], _topVerts[j + 3])) {
                    // Early exit if we went too far
                    if (_topVerts[j+1].position.z > _finalTopVerts[qi+1].position.z + 7) break;

                    // If the x locations of the quads line up
                    if (_finalTopVerts[qi].position.x == _topVerts[j].position.x && _finalTopVerts[qi+2].position.x == _topVerts[j+2].position.x) {
                        if (CompareVertices(_finalTopVerts[qi + 1], _topVerts[j]) && CompareVertices(_finalTopVerts[qi + 2], _topVerts[j + 3])) {
                            //they are stretchable, so strech the current quad
                            _finalTopVerts[qi + 1].position.z += 7;
                            _finalTopVerts[qi + 2].position.z += 7;
                            _finalTopVerts[qi + 1].tex[1]--;
                            _finalTopVerts[qi + 2].tex[1]--;
                            //indicate that we want to ignore
                            _topVerts[j].merge = -1;
                        } else{
                            //Else, that was our only shot. Next quad!
                            break;
                        }
                    }
                } else {
                    //Indicate that its not mergeable
                    _topVerts[j].merge = 0;
                }
            }
        }
    }
}

void ChunkMesher::mergeBackVerts(MeshInfo &mi)
{
	//***********************back Y merging
	if (mi.backIndex){
        //Double buffered prev list to avoid a copy
        int *prevQuads = _prevBackQuads[_currPrevBackQuads];
        if (_currPrevBackQuads == 1) {
            _currPrevBackQuads = 0;
        } else {
            _currPrevBackQuads = 1;
        }
        int qj;
        //Make sure we have a buffer allocated (this is stupid)
        if (_finalBackVerts.size() != 131072) _finalBackVerts.resize(131072);

        if (mi.pLayerBackIndex != 0) {
            //Store the size of prev layer and zero the index since we double buffer
            int pLayerBackIndex = mi.pLayerBackIndex;
            mi.pLayerBackIndex = 0;
            //Iterate again for upward merging
            for (int i = 0; i < mi.backIndex; i += 4) {
                //check if its upward mergeable
                if (CompareVerticesLight(_backVerts[i], _backVerts[i + 1]) && CompareVerticesLight(_backVerts[i + 2], _backVerts[i + 3])) {
                    //indicate that it can upward merge
                    _backVerts[i].merge = 1;
                    for (int j = 0; j < pLayerBackIndex; j++){
                        qj = prevQuads[j];
                        if (_finalBackVerts[qj + 2].position.z >= _backVerts[i + 2].position.z) {
                            //Early exit if we went too far
                            if (_finalBackVerts[qj + 2].position.z > _backVerts[i + 2].position.z) break;
                            //Make sure its lined up
                            if (_finalBackVerts[qj + 2].position.x == _backVerts[i + 2].position.x && _finalBackVerts[qj].position.x == _backVerts[i].position.x) {
                                if (CompareVertices(_backVerts[i + 1], _finalBackVerts[qj]) && CompareVertices(_backVerts[i + 2], _finalBackVerts[qj + 3])) {
                                    //They can stretch!
                                    _finalBackVerts[qj].position.y += 7;
                                    _finalBackVerts[qj + 3].position.y += 7;
                                    _finalBackVerts[qj].tex[1]++;
                                    _finalBackVerts[qj + 3].tex[1]++;
                                    _prevBackQuads[_currPrevBackQuads][mi.pLayerBackIndex++] = qj;
                                    _backVerts[i].merge = -1;
                                    break;
                                }
                            }
                        }
                    }
                } else {
                    //signal that it is not upward mergeable
                    _backVerts[i].merge = 0;
                }
                //If the vertex still is active, add to mesh
                if (_backVerts[i].merge != -1) {

                    //if it is upward mergeable, then add to prev back verts
                    if (_backVerts[i].merge > 0) {
                        _prevBackQuads[_currPrevBackQuads][mi.pLayerBackIndex++] = mi.nzVboSize;
                    }

                    _finalBackVerts[mi.nzVboSize++] = _backVerts[i];
                    _finalBackVerts[mi.nzVboSize++] = _backVerts[i + 1];
                    _finalBackVerts[mi.nzVboSize++] = _backVerts[i + 2];
                    _finalBackVerts[mi.nzVboSize++] = _backVerts[i + 3];

                }
            }

        } else { //if there was no previous layer, add verts to final mesh
            //we can assume merge is != -1.
            for (int i = 0; i < mi.backIndex; i+=4) {
              
                //if it is upward mergeable, then add to prev back verts
                if (_backVerts[i].merge > 0 && CompareVertices(_backVerts[i], _backVerts[i + 1]) && CompareVertices(_backVerts[i + 2], _backVerts[i + 3])) {
                    _prevBackQuads[_currPrevBackQuads][mi.pLayerBackIndex++] = mi.nzVboSize;
                }

                _finalBackVerts[mi.nzVboSize++] = _backVerts[i];
                _finalBackVerts[mi.nzVboSize++] = _backVerts[i + 1];
                _finalBackVerts[mi.nzVboSize++] = _backVerts[i + 2];
                _finalBackVerts[mi.nzVboSize++] = _backVerts[i + 3];
            }
        }
	}
	else{
		mi.pLayerBackIndex = 0;
	}
}

void ChunkMesher::mergeFrontVerts(MeshInfo &mi)
{
    //***********************front Y merging
    if (mi.frontIndex){
        //Double buffered prev list to avoid a copy
        int *prevQuads = _prevFrontQuads[_currPrevFrontQuads];
        if (_currPrevFrontQuads == 1) {
            _currPrevFrontQuads = 0;
        } else {
            _currPrevFrontQuads = 1;
        }
        int qj;
        //Make sure we have a buffer allocated (this is stupid)
        if (_finalFrontVerts.size() != 131072) _finalFrontVerts.resize(131072);

        if (mi.pLayerFrontIndex != 0) {
            //Store the size of prev layer and zero the index since we double buffer
            int pLayerFrontIndex = mi.pLayerFrontIndex;
            mi.pLayerFrontIndex = 0;
            //Iterate again for upward merging
            for (int i = 0; i < mi.frontIndex; i += 4) {
                //check if its upward mergeable
                if (CompareVerticesLight(_frontVerts[i], _frontVerts[i + 1]) && CompareVerticesLight(_frontVerts[i + 2], _frontVerts[i + 3])) {
                    //indicate that it can upward merge
                    _frontVerts[i].merge = 1;
                    for (int j = 0; j < pLayerFrontIndex; j++){
                        qj = prevQuads[j];
                        if (_finalFrontVerts[qj + 2].position.z >= _frontVerts[i + 2].position.z) {
                            //Early exit if we went too far
                            if (_finalFrontVerts[qj + 2].position.z > _frontVerts[i + 2].position.z) break;
                            //Make sure its lined up
                            if (_finalFrontVerts[qj + 2].position.x == _frontVerts[i + 2].position.x && _finalFrontVerts[qj].position.x == _frontVerts[i].position.x) {
                                if (CompareVertices(_frontVerts[i + 1], _finalFrontVerts[qj]) && CompareVertices(_frontVerts[i + 2], _finalFrontVerts[qj + 3])) {
                                    //They can stretch!
                                    _finalFrontVerts[qj].position.y += 7;
                                    _finalFrontVerts[qj + 3].position.y += 7;
                                    _finalFrontVerts[qj].tex[1]++;
                                    _finalFrontVerts[qj + 3].tex[1]++;
                                    _prevFrontQuads[_currPrevFrontQuads][mi.pLayerFrontIndex++] = qj;
                                    _frontVerts[i].merge = -1;
                                    break;
                                }
                            }
                        }
                    }
                } else {
                    //signal that it is not upward mergeable
                    _frontVerts[i].merge = 0;
                }
                //If the vertex still is active, add to mesh
                if (_frontVerts[i].merge != -1) {

                    //if it is upward mergeable, then add to prev front verts
                    if (_frontVerts[i].merge > 0) {
                        _prevFrontQuads[_currPrevFrontQuads][mi.pLayerFrontIndex++] = mi.pzVboSize;
                    }

                    _finalFrontVerts[mi.pzVboSize++] = _frontVerts[i];
                    _finalFrontVerts[mi.pzVboSize++] = _frontVerts[i + 1];
                    _finalFrontVerts[mi.pzVboSize++] = _frontVerts[i + 2];
                    _finalFrontVerts[mi.pzVboSize++] = _frontVerts[i + 3];

                }
            }

        } else { //if there was no previous layer, add verts to final mesh
            //we can assume merge is != -1.
            for (int i = 0; i < mi.frontIndex; i += 4) {

                //if it is upward mergeable, then add to prev front verts
                if (_frontVerts[i].merge > 0 && CompareVertices(_frontVerts[i], _frontVerts[i + 1]) && CompareVertices(_frontVerts[i + 2], _frontVerts[i + 3])) {
                    _prevFrontQuads[_currPrevFrontQuads][mi.pLayerFrontIndex++] = mi.pzVboSize;
                }

                _finalFrontVerts[mi.pzVboSize++] = _frontVerts[i];
                _finalFrontVerts[mi.pzVboSize++] = _frontVerts[i + 1];
                _finalFrontVerts[mi.pzVboSize++] = _frontVerts[i + 2];
                _finalFrontVerts[mi.pzVboSize++] = _frontVerts[i + 3];
            }
        }
    } else{
        mi.pLayerFrontIndex = 0;
    }
}

void ChunkMesher::mergeRightVerts(MeshInfo &mi) {

    if (mi.rightIndex == 0){
        mi.pLayerRightIndex = 0;
        return;
    }
    int finalQuadIndex = 0;

// Loop through each quad
    for (int i = 0; i < mi.rightIndex; i+=4) {
        // Early exit if it cant merge
        if (_rightVerts[i].merge == -1) {
            continue;
        } else {
            //Probably use this quad in the final mesh
            _finalQuads[finalQuadIndex++] = i;
        }
        if (_rightVerts[i].merge != 0 && CompareVerticesLight(_rightVerts[i], _rightVerts[i + 3]) && CompareVerticesLight(_rightVerts[i + 1], _rightVerts[i + 2])) {
            // Search for the next mergeable quad
            for (int j = i+4; j < mi.rightIndex; j += 4) {
                // Skip if the J index is not active for merging
                if (_rightVerts[j].merge < 1) continue;
            
                // Early exit if the J index cant be merged
                if (CompareVerticesLight(_rightVerts[j], _rightVerts[j + 3]) && CompareVerticesLight(_rightVerts[j + 1], _rightVerts[j + 2])) {
                    // Early exit if we went too far
                    if (_rightVerts[j].position.z > _rightVerts[i].position.z + 7) break;

                    // If the x locations of the quads line up
                    if (_rightVerts[i].position.x == _rightVerts[j].position.x) {
                        if (CompareVertices(_rightVerts[i], _rightVerts[j + 3]) && CompareVertices(_rightVerts[i + 1], _rightVerts[j + 2])) {
                            //they are stretchable, so strech the current quad
                            _rightVerts[i].position.z += 7;
                            _rightVerts[i + 1].position.z += 7;
                            _rightVerts[i].tex[0]--;
                            _rightVerts[i + 1].tex[0]--;
                            //indicate that we want to ignore
                            _rightVerts[j].merge = -1;
                        } else{
                            //Else, that was our only shot. Next quad!
                            break;
                        }
                    }
                } else {
                    //Indicate that its not mergeable
                    _rightVerts[j].merge = 0;
                }
            }
        }
    }

    int qi;
    int qj;

    //Double buffered prev list to avoid a copy
    int *prevQuads = _prevRightQuads[_currPrevRightQuads];
    if (_currPrevRightQuads == 1) {
        _currPrevRightQuads = 0;
    } else {
        _currPrevRightQuads = 1;
    }

    //Make sure we have a buffer allocated (this is stupid)
    if (_finalRightVerts.size() != 131072) _finalRightVerts.resize(131072);

    if (mi.pLayerRightIndex != 0) {
        //Store the size of prev layer and zero the index since we double buffer
        int pLayerRightIndex = mi.pLayerRightIndex;
        mi.pLayerRightIndex = 0;
        //Iterate again for upward merging
        for (int i = 0; i < finalQuadIndex; i++) {
            qi = _finalQuads[i];
            //check if its upward mergeable
            if (CompareVerticesLight(_rightVerts[qi], _rightVerts[qi + 1]) && CompareVerticesLight(_rightVerts[qi + 2], _rightVerts[qi + 3])) {
                //indicate that it can upward merge
                _rightVerts[qi].merge = 1;
                for (int j = 0; j < pLayerRightIndex; j++){
                    qj = prevQuads[j];
                    if (_finalRightVerts[qj + 2].position.z >= _rightVerts[qi + 2].position.z) {
                        //Early exit if we went too far
                        if (_finalRightVerts[qj + 2].position.z > _rightVerts[qi + 2].position.z) break;
                        //Make sure its lined up
                        if (_finalRightVerts[qj].position.z == _rightVerts[qi].position.z && _finalRightVerts[qj].position.x == _rightVerts[qi].position.x) {
                            if (CompareVertices(_rightVerts[qi + 1], _finalRightVerts[qj]) && CompareVertices(_rightVerts[qi + 2], _finalRightVerts[qj + 3])) {
                                //They can stretch!
                                _finalRightVerts[qj].position.y += 7;
                                _finalRightVerts[qj + 3].position.y += 7;
                                _finalRightVerts[qj].tex[1]++;
                                _finalRightVerts[qj + 3].tex[1]++;
                                _prevRightQuads[_currPrevRightQuads][mi.pLayerRightIndex++] = qj;
                                _rightVerts[qi].merge = -1;
                                break;
                            }
                        }
                    }
                }
            } else {
                //signal that it is not upward mergeable
                _rightVerts[qi].merge = 0;
            }
            //If the vertex still is active, add to mesh
            if (_rightVerts[qi].merge != -1) {

                //if it is upward mergeable, then add to prev right verts
                if (_rightVerts[qi].merge > 0) {
                    _prevRightQuads[_currPrevRightQuads][mi.pLayerRightIndex++] = mi.pxVboSize;
                }

                _finalRightVerts[mi.pxVboSize++] = _rightVerts[qi];
                _finalRightVerts[mi.pxVboSize++] = _rightVerts[qi + 1];
                _finalRightVerts[mi.pxVboSize++] = _rightVerts[qi + 2];
                _finalRightVerts[mi.pxVboSize++] = _rightVerts[qi + 3];
               
            }
        }

    } else { //if there was no previous layer, add verts to final mesh
        //we can assume merge is != -1.
        for (int i = 0; i < finalQuadIndex; i++) {
            qi = _finalQuads[i];

            //if it is upward mergeable, then add to prev right verts
            if (_rightVerts[qi].merge > 0 && CompareVertices(_rightVerts[qi], _rightVerts[qi + 1]) && CompareVertices(_rightVerts[qi + 2], _rightVerts[qi + 3])) {
                _prevRightQuads[_currPrevRightQuads][mi.pLayerRightIndex++] = mi.pxVboSize;
            }

            _finalRightVerts[mi.pxVboSize++] = _rightVerts[qi];
            _finalRightVerts[mi.pxVboSize++] = _rightVerts[qi + 1];
            _finalRightVerts[mi.pxVboSize++] = _rightVerts[qi + 2];
            _finalRightVerts[mi.pxVboSize++] = _rightVerts[qi + 3];
        }
    }
}

void ChunkMesher::mergeLeftVerts(MeshInfo &mi)
{
    if (mi.leftIndex == 0) {
        mi.pLayerLeftIndex = 0;
        return;
    }
    int finalQuadIndex = 0;

    // Loop through each quad
    for (int i = 0; i < mi.leftIndex; i += 4) {
        // Early exit if it cant merge
        if (_leftVerts[i].merge == -1) {
            continue;
        } else {
            //Probably use this quad in the final mesh
            _finalQuads[finalQuadIndex++] = i;
        }
        if (_leftVerts[i].merge != 0 && CompareVerticesLight(_leftVerts[i], _leftVerts[i + 3]) && CompareVerticesLight(_leftVerts[i + 1], _leftVerts[i + 2])) {
            // Search for the next mergeable quad
            for (int j = i + 4; j < mi.leftIndex; j += 4) {
                // Skip if the J index is not active for merging
                if (_leftVerts[j].merge < 1) continue;

                // Early exit if the J index cant be merged
                if (CompareVerticesLight(_leftVerts[j], _leftVerts[j + 3]) && CompareVerticesLight(_leftVerts[j + 1], _leftVerts[j + 2])) {
                    // Early exit if we went too far
                    if (_leftVerts[j + 2].position.z > _leftVerts[i + 2].position.z + 7) break;

                    // If the x locations of the quads line up
                    if (_leftVerts[i].position.x == _leftVerts[j].position.x) {
                        if (CompareVertices(_leftVerts[i + 3], _leftVerts[j]) && CompareVertices(_leftVerts[i + 2], _leftVerts[j + 1])) {
                            //they are stretchable, so strech the current quad
                            _leftVerts[i + 2].position.z += 7;
                            _leftVerts[i + 3].position.z += 7;
                            _leftVerts[i + 2].tex[0]++;
                            _leftVerts[i + 3].tex[0]++;
                            //indicate that we want to ignore
                            _leftVerts[j].merge = -1;
                        } else{
                            //Else, that was our only shot. Next quad!
                            break;
                        }
                    }
                } else {
                    //Indicate that its not mergeable
                    _leftVerts[j].merge = 0;
                }
            }
        }
    }

    int qi;
    int qj;

    //Double buffered prev list to avoid a copy
    int *prevQuads = _prevLeftQuads[_currPrevLeftQuads];
    if (_currPrevLeftQuads == 1) {
        _currPrevLeftQuads = 0;
    } else {
        _currPrevLeftQuads = 1;
    }

    //Make sure we have a buffer allocated (this is stupid)
    if (_finalLeftVerts.size() != 131072) _finalLeftVerts.resize(131072);

    if (mi.pLayerLeftIndex != 0) {
        //Store the size of prev layer and zero the index since we double buffer
        int pLayerLeftIndex = mi.pLayerLeftIndex;
        mi.pLayerLeftIndex = 0;
        //Iterate again for upward merging
        for (int i = 0; i < finalQuadIndex; i++) {
            qi = _finalQuads[i];
            //check if its upward mergeable
            if (CompareVerticesLight(_leftVerts[qi], _leftVerts[qi + 1]) && CompareVerticesLight(_leftVerts[qi + 2], _leftVerts[qi + 3])) {
                //indicate that it can upward merge
                _leftVerts[qi].merge = 1;
                for (int j = 0; j < pLayerLeftIndex; j++){
                    qj = prevQuads[j];
                    if (_finalLeftVerts[qj].position.z >= _leftVerts[qi].position.z) {
                        //Early exit if we went too far
                        if (_finalLeftVerts[qj].position.z > _leftVerts[qi].position.z) break;
                        //Make sure its lined up
                        if (_finalLeftVerts[qj+2].position.z == _leftVerts[qi+2].position.z && _finalLeftVerts[qj].position.x == _leftVerts[qi].position.x) {
                            if (CompareVertices(_leftVerts[qi + 1], _finalLeftVerts[qj]) && CompareVertices(_leftVerts[qi + 2], _finalLeftVerts[qj + 3])) {
                                //They can stretch!
                                _finalLeftVerts[qj].position.y += 7;
                                _finalLeftVerts[qj + 3].position.y += 7;
                                _finalLeftVerts[qj].tex[1]++;
                                _finalLeftVerts[qj + 3].tex[1]++;
                                _prevLeftQuads[_currPrevLeftQuads][mi.pLayerLeftIndex++] = qj;
                                _leftVerts[qi].merge = -1;
                                break;
                            }
                        }
                    }
                }
            } else {
                //signal that it is not upward mergeable
                _leftVerts[qi].merge = 0;
            }
            //If the vertex still is active, add to mesh
            if (_leftVerts[qi].merge != -1) {

                //if it is upward mergeable, then add to prev left verts
                if (_leftVerts[qi].merge > 0) {
                    _prevLeftQuads[_currPrevLeftQuads][mi.pLayerLeftIndex++] = mi.nxVboSize;
                }

                _finalLeftVerts[mi.nxVboSize++] = _leftVerts[qi];
                _finalLeftVerts[mi.nxVboSize++] = _leftVerts[qi + 1];
                _finalLeftVerts[mi.nxVboSize++] = _leftVerts[qi + 2];
                _finalLeftVerts[mi.nxVboSize++] = _leftVerts[qi + 3];

            }
        }

    } else { //if there was no previous layer, add verts to final mesh
        //we can assume merge is != -1.
        for (int i = 0; i < finalQuadIndex; i++) {
            qi = _finalQuads[i];

            //if it is upward mergeable, then add to prev left verts
            if (_leftVerts[qi].merge > 0 && CompareVertices(_leftVerts[qi], _leftVerts[qi + 1]) && CompareVertices(_leftVerts[qi + 2], _leftVerts[qi + 3])) {
                _prevLeftQuads[_currPrevLeftQuads][mi.pLayerLeftIndex++] = mi.nxVboSize;
            }

            _finalLeftVerts[mi.nxVboSize++] = _leftVerts[qi];
            _finalLeftVerts[mi.nxVboSize++] = _leftVerts[qi + 1];
            _finalLeftVerts[mi.nxVboSize++] = _leftVerts[qi + 2];
            _finalLeftVerts[mi.nxVboSize++] = _leftVerts[qi + 3];
        }
    }
}

void ChunkMesher::mergeBottomVerts(MeshInfo &mi)
{
    if (mi.botIndex == 0) return;

    int qi;

    if (_finalBottomVerts.size() != 131072) _finalBottomVerts.resize(131072);

    for (int i = 0; i < mi.botIndex; i += 4) {
        // Early exit if it cant merge
        if (_bottomVerts[i].merge == -1) {
            continue;
        } else {
            //Use this quad in the final mesh
            qi = mi.nyVboSize;
            _finalBottomVerts[mi.nyVboSize++] = _bottomVerts[i];
            _finalBottomVerts[mi.nyVboSize++] = _bottomVerts[i + 1];
            _finalBottomVerts[mi.nyVboSize++] = _bottomVerts[i + 2];
            _finalBottomVerts[mi.nyVboSize++] = _bottomVerts[i + 3];
        }
        if (_finalBottomVerts[qi].merge != 0 && CompareVerticesLight(_finalBottomVerts[qi], _finalBottomVerts[qi + 1]) && CompareVerticesLight(_finalBottomVerts[qi + 2], _finalBottomVerts[qi + 3])) {
            // Search for the next mergeable quad
            for (int j = i + 4; j < mi.botIndex; j += 4) {
                // Skip if the J index is not active for merging
                if (_bottomVerts[j].merge < 1) continue;

                // Early exit if the J index cant be merged
                if (CompareVerticesLight(_bottomVerts[j], _bottomVerts[j + 1]) && CompareVerticesLight(_bottomVerts[j + 2], _bottomVerts[j + 3])) {
                    // Early exit if we went too far
                    if (_bottomVerts[j + 1].position.z > _finalBottomVerts[qi + 1].position.z + 7) break;

                    // If the x locations of the quads line up
                    if (_finalBottomVerts[qi].position.x == _bottomVerts[j].position.x && _finalBottomVerts[qi + 2].position.x == _bottomVerts[j + 2].position.x) {
                        if (CompareVertices(_finalBottomVerts[qi + 1], _bottomVerts[j]) && CompareVertices(_finalBottomVerts[qi + 2], _bottomVerts[j + 3])) {
                            //they are stretchable, so strech the current quad
                            _finalBottomVerts[qi + 1].position.z += 7;
                            _finalBottomVerts[qi + 2].position.z += 7;
                            _finalBottomVerts[qi + 1].tex[1]--;
                            _finalBottomVerts[qi + 2].tex[1]--;
                            //indicate that we want to ignore
                            _bottomVerts[j].merge = -1;
                        } else{
                            //Else, that was our only shot. Next quad!
                            break;
                        }
                    }
                } else {
                    //Indicate that its not mergeable
                    _bottomVerts[j].merge = 0;
                }
            }
        }
    }
}

bool ChunkMesher::createChunkMesh(RenderTask *renderTask)
{
    //Stores the number of blocks iterated for z and y
	int zc, yc;

	int waveEffect;
	Block *block;

    //Stores the information about the current mesh job
    MeshInfo mi = {};

    _waterVboVerts.clear();
    _nonBlockVerts.clear();

	//store the render task so we can pass it to functions
    mi.task = renderTask;

    //Used in merging
    _currPrevRightQuads = 0;
    _currPrevLeftQuads = 0;
    _currPrevBackQuads = 0;
    _currPrevFrontQuads = 0;


	//create a new chunk mesh data container
	if (chunkMeshData != NULL){
		ERROR("Tried to create mesh with in use chunkMeshData!");
		return 0;
	}
    
    //Stores the data for a chunk mesh
	chunkMeshData = new ChunkMeshData(renderTask->chunk);

	for (mi.y = 0; mi.y < CHUNK_WIDTH; mi.y++) {

        mi.y2 = mi.y * 2;
		yc = mi.y * CHUNK_LAYER;
        
        //reset all indexes to zero for each layer
		mi.topIndex = 0;
		mi.leftIndex = 0;
		mi.rightIndex = 0;
		mi.frontIndex = 0;
		mi.backIndex = 0;
		mi.botIndex = 0;
		for (mi.z = 0; mi.z < CHUNK_WIDTH; mi.z++){
            
            mi.z2 = mi.z * 2;
			zc = yc + mi.z * CHUNK_WIDTH;

			for (mi.x = 0; mi.x < CHUNK_WIDTH; mi.x++){

                mi.x2 = mi.x * 2;
				mi.c = zc + mi.x;
                //We use wc instead of c, because the array is sentinalized at all edges so we dont have to access neighbor chunks with mutexes
				mi.wc = (mi.y + 1)*(PADDED_LAYER)+(mi.z + 1)*(PADDED_WIDTH)+(mi.x + 1); //get the expanded c for our sentinelized array
				//get the block properties
                mi.btype = GETBLOCKTYPE(renderTask->chData[mi.wc]);
				block = &(Blocks[mi.btype]);

              
				waveEffect = block->waveEffect;
				//water is currently hardcoded 
				if (mi.btype >= LOWWATER){
                    addLiquidToMesh(mi);
				}
				else 
				{
                    //Check the mesh type
                    mi.meshType = Blocks[mi.btype].meshType;

                    switch (mi.meshType){
                        case MeshType::BLOCK:
						    addBlockToMesh(mi);
						    break;
                        case MeshType::LEAVES:
                        case MeshType::CROSSFLORA:
                        case MeshType::FLORA:
                            addFloraToMesh(mi);
						    break;
					    default:
					        //No mesh, do nothing
						    break;
					}

				}
                //Store the current block for use next frame as the previous block
				mi.pbtype = mi.btype;
			}
            //We reset +x merging for up, front, back, bottom each z iteration
			mi.mergeUp = 0;
			mi.mergeFront = 0;
			mi.mergeBack = 0;
			mi.mergeBot = 0;
		}
        //second pass of merging for all directions
		mergeTopVerts(mi);
		mergeFrontVerts(mi);
        mergeBackVerts(mi);
        mergeRightVerts(mi);
        mergeLeftVerts(mi);
        mergeBottomVerts(mi);
	}

	int highestY = 0, lowestY = 256, highestX = 0, lowestX = 256, highestZ = 0, lowestZ = 256;
	int pyVboOff, pxVboOff, pzVboOff, nyVboOff, nxVboOff, nzVboOff;
	int index = 0;
	pyVboOff = 0;
   
	chunkMeshData->vertices.resize(mi.pyVboSize + mi.nxVboSize + mi.pxVboSize + mi.nzVboSize + mi.pzVboSize + mi.nyVboSize);
	for (int i = 0; i < mi.pyVboSize; i++){ 
		if (_finalTopVerts[i].position.y < lowestY) lowestY = _finalTopVerts[i].position.y;
		chunkMeshData->vertices[index++] = _finalTopVerts[i];
	}
	nxVboOff = index;
	for (int i = 0; i < mi.nxVboSize; i++){
		if (_finalLeftVerts[i].position.x > highestX) highestX = _finalLeftVerts[i].position.x;
		chunkMeshData->vertices[index++] = _finalLeftVerts[i];
	}
	pxVboOff = index;
	for (int i = 0; i < mi.pxVboSize; i++){
		if (_finalRightVerts[i].position.x < lowestX) lowestX = _finalRightVerts[i].position.x;
		chunkMeshData->vertices[index++] = _finalRightVerts[i];
	}
	nzVboOff = index;
	for (int i = 0; i < mi.nzVboSize; i++){
		if (_finalBackVerts[i].position.z > highestZ) highestZ = _finalBackVerts[i].position.z;
		chunkMeshData->vertices[index++] = _finalBackVerts[i];
	}
	pzVboOff = index;
	for (int i = 0; i < mi.pzVboSize; i++){
		if (_finalFrontVerts[i].position.z < lowestZ) lowestZ = _finalFrontVerts[i].position.z;
		chunkMeshData->vertices[index++] = _finalFrontVerts[i];
	}
	nyVboOff = index;
	for (int i = 0; i < mi.nyVboSize; i++){
		if (_finalBottomVerts[i].position.y > highestY) highestY = _finalBottomVerts[i].position.y;
		chunkMeshData->vertices[index++] = _finalBottomVerts[i];
	}

    if (mi.nbIndex) {
        chunkMeshData->transVertices.swap(_nonBlockVerts);
    }

    highestY /= 7;
    lowestY /= 7;
    highestX /= 7;
    lowestX /= 7;
    highestZ /= 7;
    lowestZ /= 7;

	int indice = (index / 4) * 6;

	chunkMeshData->bAction = 0;
	chunkMeshData->wAction = 0;

	//add all vertices to the vbo
    if (chunkMeshData->vertices.size() || chunkMeshData->transVertices.size()){
		chunkMeshData->indexSize = (chunkMeshData->vertices.size() * 6) / 4;

		//now let the sizes represent indice sizes
		chunkMeshData->pyVboOff = pyVboOff;
		chunkMeshData->pyVboSize = (mi.pyVboSize / 4) * 6;
		chunkMeshData->nyVboOff = nyVboOff;
		chunkMeshData->nyVboSize = (mi.nyVboSize / 4) * 6;
		chunkMeshData->pxVboOff = pxVboOff;
		chunkMeshData->pxVboSize = (mi.pxVboSize / 4) * 6;
		chunkMeshData->nxVboOff = nxVboOff;
		chunkMeshData->nxVboSize = (mi.nxVboSize / 4) * 6;
		chunkMeshData->pzVboOff = pzVboOff;
		chunkMeshData->pzVboSize = (mi.pzVboSize / 4) * 6;
		chunkMeshData->nzVboOff = nzVboOff;
		chunkMeshData->nzVboSize = (mi.nzVboSize / 4) * 6;

        chunkMeshData->transVboSize = (mi.nbIndex / 4) * 6;
      
		chunkMeshData->highestY = highestY;
		chunkMeshData->lowestY = lowestY;
		chunkMeshData->highestX = highestX;
		chunkMeshData->lowestX = lowestX;
		chunkMeshData->highestZ = highestZ;
		chunkMeshData->lowestZ = lowestZ;

		chunkMeshData->bAction = 1;
	}
	else{
		chunkMeshData->indexSize = 0;
		chunkMeshData->bAction = 2;
	}

	if (mi.liquidIndex){
        chunkMeshData->waterIndexSize = (mi.liquidIndex * 6) / 4;
		chunkMeshData->waterVertices.swap(_waterVboVerts);
		chunkMeshData->wAction = 1;
	}
	else{
		chunkMeshData->waterIndexSize = 0;
		chunkMeshData->wAction = 2;
	}

	return 0;
}

bool ChunkMesher::createOnlyWaterMesh(RenderTask *renderTask)
{
	if (chunkMeshData != NULL){
		ERROR("Tried to create mesh with in use chunkMeshData!");
		return 0;
	}
    chunkMeshData = new ChunkMeshData(renderTask->chunk);

    _waterVboVerts.clear();
    MeshInfo mi = {};

    mi.task = renderTask;

    for (int i = 0; i < renderTask->wSize; i++) {
        mi.wc = renderTask->wvec[i];
        mi.btype = GETBLOCKTYPE(renderTask->chData[mi.wc]);
        mi.x = (mi.wc % PADDED_CHUNK_WIDTH) - 1;
        mi.y = (mi.wc / PADDED_CHUNK_LAYER) - 1;
        mi.z = ((mi.wc % PADDED_CHUNK_LAYER) / PADDED_CHUNK_WIDTH) - 1;
      
        addLiquidToMesh(mi);
    }

	chunkMeshData->bAction = 0;

	if (mi.liquidIndex){
        chunkMeshData->waterIndexSize = (mi.liquidIndex * 6) / 4;
		chunkMeshData->waterVertices.swap(_waterVboVerts);
		chunkMeshData->wAction = 1;
	}
	else{
		chunkMeshData->waterIndexSize = 0;
		chunkMeshData->wAction = 2;
	}
    
	return 0;
}

void ChunkMesher::freeBuffers()
{
	//free memory
	vector <BlockVertex>().swap(_vboVerts);

    //These dont get too big so it might be ok to not free them?
	/*vector <Vertex>().swap(waterVboVerts);
	vector<Vertex>().swap(finalTopVerts);
	vector<Vertex>().swap(finalLeftVerts);
	vector<Vertex>().swap(finalRightVerts);
	vector<Vertex>().swap(finalFrontVerts);
	vector<Vertex>().swap(finalBackVerts);
	vector<Vertex>().swap(finalBottomVerts);
	vector<Vertex>().swap(finalNbVerts);*/
}