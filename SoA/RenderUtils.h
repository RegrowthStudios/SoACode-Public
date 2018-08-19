///
/// RenderUtils.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 3 Dec 2014
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Some simple utils for rendering
///

#pragma once

#ifndef RenderUtils_h__
#define RenderUtils_h__

#include "stdafx.h"

/// Sets translation for a matrix relative to relativePos. Will overwrite existing translation
inline void setMatrixTranslation(f32m4& matrix, const f64v3 &position, const f64v3& relativePos) {
    matrix[3][0] = (f32)(position.x - relativePos.x);
    matrix[3][1] = (f32)(position.y - relativePos.y);
    matrix[3][2] = (f32)(position.z - relativePos.z);
}

/// Sets translation for a matrix relative to relativePos. Will overwrite existing translation
inline void setMatrixTranslation(f32m4& matrix, const f32v3 &position, const f32v3& relativePos) {
    matrix[3][0] = (position.x - relativePos.x);
    matrix[3][1] = (position.y - relativePos.y);
    matrix[3][2] = (position.z - relativePos.z);
}

/// Sets translation for a matrix relative to relativePos. Will overwrite existing translation
inline void setMatrixTranslation(f32m4& matrix, const f32v3 &position, const f64v3& relativePos) {
    matrix[3][0] = (f32)((f64)position.x - relativePos.x);
    matrix[3][1] = (f32)((f64)position.y - relativePos.y);
    matrix[3][2] = (f32)((f64)position.z - relativePos.z);
}

/// Sets translation for a matrix. Will overwrite existing translation
inline void setMatrixTranslation(f32m4& matrix, const f32 x, const f32 y, const f32 z) {
    matrix[3][0] = x;
    matrix[3][1] = y;
    matrix[3][2] = z;
}

/// Sets translation for a matrix. Will overwrite existing translation
inline void setMatrixTranslation(f32m4& matrix, const double x, const double y, const double z) {
    matrix[3][0] = (f32)x;
    matrix[3][1] = (f32)y;
    matrix[3][2] = (f32)z;
}

/// Sets translation for a matrix. Will overwrite existing translation
inline void setMatrixTranslation(f32m4& matrix, const f32v3& trans) {
    matrix[3][0] = trans.x;
    matrix[3][1] = trans.y;
    matrix[3][2] = trans.z;
}

/// Sets translation for a matrix. Will overwrite existing translation
inline void setMatrixTranslation(f32m4& matrix, const f64v3& trans) {
    matrix[3][0] = (f32)trans.x;
    matrix[3][1] = (f32)trans.y;
    matrix[3][2] = (f32)trans.z;
}

/// Sets scale for a matrix. Will overwrite existing scale
inline void setMatrixScale(f32m4& matrix, const f32v3 &scale) {
    matrix[0][0] = scale.x;
    matrix[1][1] = scale.y;
    matrix[2][2] = scale.z;
}

/// Sets scale for a matrix. Will overwrite existing scale
inline void setMatrixScale(f32m4& matrix, const f32 scaleX, const f32 scaleY, const f32 scaleZ) {
    matrix[0][0] = scaleX;
    matrix[1][1] = scaleY;
    matrix[2][2] = scaleZ;
}

#endif // RenderUtils_h__