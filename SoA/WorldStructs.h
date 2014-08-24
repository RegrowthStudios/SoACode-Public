#pragma once
#include <queue>

#include "Constants.h"
#include "OpenGLStructs.h"
#include "Texture2d.h"

extern string saveFilePath;
extern class Item *ObjectList[OBJECT_LIST_SIZE];

struct FixedSizeBillboardVertex{
	glm::vec3 pos;
	GLubyte uv[2];
};

class Marker{
public:
	glm::dvec3 pos;
	Color color;
	int num;
	double dist;
	string name;

	class Texture2D distText;
	Texture2D nameTex;

	Marker(const glm::dvec3 &Pos, string Name, const glm::vec3 Color);
	void Draw(glm::mat4 &VP, const glm::dvec3 &playerPos);
};

struct NoiseInfo
{
	NoiseInfo(){
		memset(this, 0, sizeof(NoiseInfo));
		modifier = NULL;
		name = "UNNAMED";
	}
	~NoiseInfo(){
		if (modifier) delete modifier;
		modifier = NULL;
	}
	double persistence;
	double frequency;
	double lowBound;
	double upBound;
	double scale;
	GLint octaves;
	GLint composition;
	NoiseInfo *modifier;
	string name;
	GLint type;
};

struct BiomeTree
{
	BiomeTree(GLfloat prob, GLint index) : probability(prob), 
										   treeIndex(index)
	{
	}
	GLfloat probability;
	GLint treeIndex;
};

struct BiomeFlora
{
	BiomeFlora(GLfloat prob, GLint index) : probability(prob), 
										   floraIndex(index)
	{
	}
	GLfloat probability;
	GLint floraIndex;
};

struct Biome
{
	Biome();

	GLint vecIndex;
	GLint hasAltColor;
	GLint looseSoilDepth; //TO ADD
	GLint isBase; //ISBASE MUST BE LAST GLINT
	GLubyte r, g, b, padding;
	GLushort surfaceBlock;
	GLushort underwaterBlock;
	GLushort beachBlock;
	GLushort padding2; //PADDING2 MUST BE LAST GLushort
	GLfloat treeChance;
	GLfloat applyBiomeAt;
	GLfloat lowTemp, highTemp, tempSlopeLength;
	GLfloat lowRain, highRain, rainSlopeLength;
	GLfloat maxHeight, maxHeightSlopeLength;
	GLfloat minTerrainMult; //MINTERRAINMULT MUST BE LAST GLfloat
	string name;
	string filename;

	GLushort surfaceLayers[SURFACE_DEPTH]; //stores the top 50 layers corresponding to this biome
	NoiseInfo distributionNoise;
	vector <NoiseInfo> terrainNoiseList;
	vector <Biome*> childBiomes;
	vector <BiomeTree> possibleTrees;
	vector <BiomeFlora> possibleFlora;
};

//flags
const int PLATEAU = 0x1;
const int VOLCANO = 0x2;
const int TOOSTEEP = 0x4;

struct ChunkMeshData
{
	ChunkMeshData(class Chunk *ch);

    void addTransQuad(const i8v3& pos);

	//***** This 88 byte block gets memcpyd to ChunkMesh *****
	GLint pxVboOff, pxVboSize, nxVboOff, nxVboSize, pzVboOff, pzVboSize, nzVboOff, nzVboSize;
	GLint pyVboOff, pyVboSize, nyVboOff, nyVboSize, transVboSize, cutoutVboSize;
	GLint highestY, lowestY, highestX, lowestX, highestZ, lowestZ;
	GLuint indexSize;
	GLuint waterIndexSize;
	//*****  End Block *****

	vector <BlockVertex> vertices;
    vector <BlockVertex> transVertices;
    vector <BlockVertex> cutoutVertices;
    vector <LiquidVertex> waterVertices;
	Chunk *chunk;
	struct ChunkMesh *chunkMesh;
	int bAction, wAction;
	int debugCode;

    //*** Transparency info for sorting ***
    ui32 transVertIndex;
    vector <i8v3> transQuadPositions;
    vector <ui32> transQuadIndices;
};

struct LoadData
{
	LoadData()
	{
	}
	LoadData(struct HeightData *hmap, int x, int z, class TerrainGenerator *gen)
	{
		heightMap = hmap;
		wx = x;
		wz = z;
		generator = gen;
	}
	
	inline void init(HeightData *hmap, int x, int z, TerrainGenerator *gen)
	{
		heightMap = hmap;
		wx = x;
		wz = z;
		generator = gen;
	}

	HeightData *heightMap;
	int wx;
	int wz;
	TerrainGenerator *generator;
};

struct HeightData
{
	GLint height;
	GLint temperature;
	GLint rainfall;
	GLint snowDepth;
	GLint sandDepth;
	GLint flags;
	Biome *biome;
	GLushort surfaceBlock;
};

struct FaceData
{
	void Set(int Face, int Ipos, int Jpos, int Rot){
		face = Face;
		ipos = Ipos;
		jpos = Jpos;
		rotation = Rot;
	}
	int face;
	int ipos;
	int jpos;
	int rotation;
};

struct MineralData
{
	MineralData(GLint btype, GLint startheight, float startchance, GLint centerheight, float centerchance, GLint endheight, float endchance, GLint minsize, GLint maxsize)
	{
		blockType = btype;
		startHeight = startheight;
		startChance = startchance;
		endHeight = endheight;
		endChance = endchance;
		centerHeight = centerheight;
		centerChance = centerchance;
		minSize = minsize;
		maxSize = maxsize;
	}
	GLint blockType, startHeight, endHeight, centerHeight, minSize, maxSize;
	GLfloat startChance, centerChance, endChance;
};

struct BillboardVertex
{
	glm::vec3 pos;
	glm::vec2 uvMult;
	GLubyte texUnit;
	GLubyte texID;
	GLubyte light[2];
	GLubyte color[4];
	GLubyte size;
	GLubyte xMod;
	GLubyte padding[2]; //needs to be 4 byte aligned
};

struct PhysicsBlockPosLight
{
	GLfloat pos[3]; //12
    GLubyte color[3]; //15
    GLubyte pad1; //16
    GLubyte overlayColor[3]; //19
    GLubyte pad2; //20
	GLubyte light[2]; //22
    GLubyte pad3[2]; //24
};

struct TreeVertex
{
	glm::vec2 pos; //8
	glm::vec3 center; //20
	GLubyte lr, lg, lb, size; //24
	GLubyte tr, tg, tb, ltex; //28
};

//No idea how this works. Something to do with prime numbers, but returns # between -1 and 1
inline double PseudoRand(int x, int z)
{
	 int n= (x & 0xFFFF) + ((z & 0x7FFF) << 16);
	 n=(n<<13)^n;
	 int nn=(n*(n*n*60493+z*19990303)+x*1376312589)&0x7fffffff;
	 return 1.0-((double)nn/1073741824.0);
}


inline double PseudoRand(int n)
{
    n = (n << 13) ^ n;
    int nn = (n*(n*n * 60493 + n * 19990303) + n * 1376312589) & 0x7fffffff;
    return 1.0 - ((double)nn / 1073741824.0);
}

