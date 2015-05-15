///
/// soaUtils.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 17 Feb 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Soa specific generic utilities
///

#pragma once

#ifndef soaUtils_h__
#define soaUtils_h__

#include <Vorb/graphics/ImageIO.h>
#include <Vorb/io/IOManager.h>
#include <Vorb/types.h>
#include <chrono>
#include <cstdio>
#include <ctime> 
#include <iomanip>
#include <sstream>
#include <string> 

/************************************************************************/
/* Debugging Utilities                                                  */
/************************************************************************/
#define PRINT_VEC_TYPE(TYPE, SYM) \
inline void printVec(const char* desc, const TYPE##v2& vec) { \
    printf("%s <%"#SYM", %"#SYM">\n", desc, vec.x, vec.y); \
} \
inline void printVec(const char* desc, const TYPE##v3& vec) { \
    printf("%s <%"#SYM", %"#SYM", %"#SYM">\n", desc, vec.x, vec.y, vec.z); \
} \
inline void printVec(const char* desc, const TYPE##v4& vec) { \
    printf("%s <%"#SYM", %"#SYM", %"#SYM", %"#SYM">\n", desc, vec.x, vec.y, vec.z, vec.w); \
}
PRINT_VEC_TYPE(f32, f)
PRINT_VEC_TYPE(f64, lf)
PRINT_VEC_TYPE(i16, hd)
PRINT_VEC_TYPE(i32, d)
PRINT_VEC_TYPE(i64, lld)
PRINT_VEC_TYPE(ui16, hu)
PRINT_VEC_TYPE(ui32, u)
PRINT_VEC_TYPE(ui64, llu)
#undef PRINT_VEC_TYPE

/************************************************************************/
/* Miscellaneous Utilities                                              */
/************************************************************************/
/// Gets the closest point on the AABB to the position
/// @param pos: Position to query nearest point in relation to
/// @param aabbPos: Position of the -x,-y,-z corner of the aabb
/// @param aabbDims: Dimensions of the aabb
/// @return the position of the closest point on the aabb
inline f32v3 getClosestPointOnAABB(const f32v3& pos, const f32v3& aabbPos,
                                   const f32v3& aabbDims) {
    return f32v3((pos.x <= aabbPos.x) ? aabbPos.x : ((pos.x > aabbPos.x + aabbDims.x) ?
           (aabbPos.x + aabbDims.x) : pos.x),
           (pos.y <= aabbPos.y) ? aabbPos.y : ((pos.y > aabbPos.y + aabbDims.y) ?
           (aabbPos.y + aabbDims.y) : pos.y),
           (pos.z <= aabbPos.z) ? aabbPos.z : ((pos.z > aabbPos.z + aabbDims.z) ?
           (aabbPos.z + aabbDims.z) : pos.z));
}
inline f64v3 getClosestPointOnAABB(const f64v3& pos, const f64v3& aabbPos,
                                   const f64v3& aabbDims) {
    return f64v3((pos.x <= aabbPos.x) ? aabbPos.x : ((pos.x > aabbPos.x + aabbDims.x) ?
           (aabbPos.x + aabbDims.x) : pos.x),
           (pos.y <= aabbPos.y) ? aabbPos.y : ((pos.y > aabbPos.y + aabbDims.y) ?
           (aabbPos.y + aabbDims.y) : pos.y),
           (pos.z <= aabbPos.z) ? aabbPos.z : ((pos.z > aabbPos.z + aabbDims.z) ?
           (aabbPos.z + aabbDims.z) : pos.z));
}

/// Moves val towards target in increments of step
template <typename T>
inline void stepTowards(T& val, const T& target, const T& step) {
    if (val < target) {
        val += step;
        if (val > target) val = target;
    } else if (val > target) {
        val -= step;
        if (val < target) val = target;
    }
}

/// Gets dot product with self, cheaper than glm::dot because less copy
inline f32 selfDot(const f32v3& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}
inline f32 selfDot(const f32v4& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}
inline f64 selfDot(const f64v3& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}
inline f64 selfDot(const f64v4& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}
inline i32 selfDot(const i32v3& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}
inline i32 selfDot(const i32v4& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}
inline ui32 selfDot(const ui32v3& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}
inline ui32 selfDot(const ui32v4& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}

inline bool dumpFramebufferImage(const nString& rootDir, const ui32v4& viewport) {
    std::vector<ui8v4> pixels;
    int width = (viewport.z - viewport.x);
    int height = (viewport.w - viewport.y);
    pixels.resize(width * height);
    // Read pixels from framebuffer
    glReadPixels(viewport.x, viewport.y, viewport.z, viewport.w, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    // Use time to get file path
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d-%X");
    nString path = rootDir + "img-" + ss.str() + ".png";
    std::replace(path.begin(), path.end(), ':', '-');
    // Save screenshot
    if (!vg::ImageIO().save(path, pixels.data(), width, height, vg::ImageIOFormat::RGBA_UI8)) return false;

    printf("Made screenshot %s\n", path.c_str());
    return true;
}

#endif // soaUtils_h__
