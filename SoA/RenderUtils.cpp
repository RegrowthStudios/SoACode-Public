#include "stdafx.h"
#include "RenderUtils.h"

#include "global.h"

inline void setModelMatrixTranslation(const glm::vec3 &translation, const glm::vec3& playerPos) {
    GlobalModelMatrix[3][0] = (float)((double)translation.x - playerPos.x);
    GlobalModelMatrix[3][1] = (float)((double)translation.y - playerPos.y);
    GlobalModelMatrix[3][2] = (float)((double)translation.z - playerPos.z);
}

inline void setModelMatrixTranslation(const float x, const float y, const float z, const float playerX, const float playerY, const float playerZ) {
    GlobalModelMatrix[3][0] = (float)((double)x - playerX);
    GlobalModelMatrix[3][1] = (float)((double)y - playerY);
    GlobalModelMatrix[3][2] = (float)((double)z - playerZ);
}

inline void setModelMatrixScale(const glm::vec3 &scale) {
    GlobalModelMatrix[0][0] = scale.x;
    GlobalModelMatrix[1][1] = scale.y;
    GlobalModelMatrix[2][2] = scale.z;
}

inline void setModelMatrixScale(const float scaleX, const float scaleY, const float scaleZ) {
    GlobalModelMatrix[0][0] = scaleX;
    GlobalModelMatrix[1][1] = scaleY;
    GlobalModelMatrix[2][2] = scaleZ;
}