#pragma once

extern inline void setModelMatrixTranslation(const glm::vec3 &translation, const glm::vec3& playerPos);
extern inline void setModelMatrixTranslation(const float x, const float y, const float z, const float playerX, const float playerY, const float playerZ);
extern inline void setModelMatrixScale(const glm::vec3 &scale);
extern inline void setModelMatrixScale(const float scaleX, const float scaleY, const float scaleZ);
