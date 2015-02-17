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

#endif // soaUtils_h__
