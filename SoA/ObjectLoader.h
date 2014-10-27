#pragma once
#include "OpenGLStructs.h"

#ifndef GL_UNSIGNED_SHORT_5_6_5
#define GL_UNSIGNED_SHORT_5_6_5 0x8363
#endif
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

typedef f64 coordinate3lf[3];

ui32 loadDDS(const cString imagepath);

class ObjectLoader {
public:
    ObjectLoader();

    i32 load(const cString filename, std::vector<ColorVertex>& vertices, std::vector<ui16>& indices);
private:
    bool _isMaterial, _hasNormals, _hasTexture, _hasVertexNormals;
};

