#include "stdafx.h"
#include "TerrainPatch.h"

#include "BlockData.h"
#include "Camera.h"
#include "Chunk.h"
#include "FloraGenerator.h"
#include "GameManager.h"
#include "MessageManager.h"
#include "Options.h"
#include "WorldStructs.h"

#include "utils.h"

int lodDetailLevels[DetailLevels+3] = {8/scale, 16/scale, 32/scale, 64/scale, 128/scale, 256/scale, 512/scale, 1024/scale, 2048/scale, 4096/scale, 8192/scale, 16384/scale, 32768/scale, 65536/scale, 131072/scale, 262144/scale, 524228/scale};
int lodDistanceLevels[DetailLevels] = {500/scale, 1000/scale, 2000/scale, 4000/scale, 8000/scale, 16000/scale, 32000/scale, 64000/scale, 128000/scale, 256000/scale, 512000/scale, 1024000/scale, 4096000/scale, INT_MAX};

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

