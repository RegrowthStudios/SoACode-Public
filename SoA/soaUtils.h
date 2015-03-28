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

#include <cstdio>
#include <Vorb/types.h>

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

#endif // soaUtils_h__
