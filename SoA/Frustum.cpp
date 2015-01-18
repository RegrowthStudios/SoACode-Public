#include "stdafx.h"
#include "Frustum.h"

void Frustum::update(const f32m4& projectionMatrix, const f32m4& viewMatrix) {
    f32m4 model = projectionMatrix * viewMatrix;

    f32 clip[16];
    f32 t;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            clip[i * 4 + j] = model[i][j];
        }
    }

    /* Extract the numbers for the RIGHT plane */
    _frustum[0][0] = clip[3] - clip[0];
    _frustum[0][1] = clip[7] - clip[4];
    _frustum[0][2] = clip[11] - clip[8];
    _frustum[0][3] = clip[15] - clip[12];

    /* Normalize the result */
    t = sqrt(_frustum[0][0] * _frustum[0][0] + _frustum[0][1] * _frustum[0][1] + _frustum[0][2] * _frustum[0][2]);
    _frustum[0][0] /= t;
    _frustum[0][1] /= t;
    _frustum[0][2] /= t;
    _frustum[0][3] /= t;

    /* Extract the numbers for the LEFT plane */
    _frustum[1][0] = clip[3] + clip[0];
    _frustum[1][1] = clip[7] + clip[4];
    _frustum[1][2] = clip[11] + clip[8];
    _frustum[1][3] = clip[15] + clip[12];

    /* Normalize the result */
    t = sqrt(_frustum[1][0] * _frustum[1][0] + _frustum[1][1] * _frustum[1][1] + _frustum[1][2] * _frustum[1][2]);
    _frustum[1][0] /= t;
    _frustum[1][1] /= t;
    _frustum[1][2] /= t;
    _frustum[1][3] /= t;

    /* Extract the BOTTOM plane */
    _frustum[2][0] = clip[3] + clip[1];
    _frustum[2][1] = clip[7] + clip[5];
    _frustum[2][2] = clip[11] + clip[9];
    _frustum[2][3] = clip[15] + clip[13];

    /* Normalize the result */
    t = sqrt(_frustum[2][0] * _frustum[2][0] + _frustum[2][1] * _frustum[2][1] + _frustum[2][2] * _frustum[2][2]);
    _frustum[2][0] /= t;
    _frustum[2][1] /= t;
    _frustum[2][2] /= t;
    _frustum[2][3] /= t;

    /* Extract the TOP plane */
    _frustum[3][0] = clip[3] - clip[1];
    _frustum[3][1] = clip[7] - clip[5];
    _frustum[3][2] = clip[11] - clip[9];
    _frustum[3][3] = clip[15] - clip[13];

    /* Normalize the result */
    t = sqrt(_frustum[3][0] * _frustum[3][0] + _frustum[3][1] * _frustum[3][1] + _frustum[3][2] * _frustum[3][2]);
    _frustum[3][0] /= t;
    _frustum[3][1] /= t;
    _frustum[3][2] /= t;
    _frustum[3][3] /= t;

    /* Extract the FAR plane */
    _frustum[4][0] = clip[3] - clip[2];
    _frustum[4][1] = clip[7] - clip[6];
    _frustum[4][2] = clip[11] - clip[10];
    _frustum[4][3] = clip[15] - clip[14];

    /* Normalize the result */
    t = sqrt(_frustum[4][0] * _frustum[4][0] + _frustum[4][1] * _frustum[4][1] + _frustum[4][2] * _frustum[4][2]);
    _frustum[4][0] /= t;
    _frustum[4][1] /= t;
    _frustum[4][2] /= t;
    _frustum[4][3] /= t;

    /* Extract the NEAR plane */
    _frustum[5][0] = clip[3] + clip[2];
    _frustum[5][1] = clip[7] + clip[6];
    _frustum[5][2] = clip[11] + clip[10];
    _frustum[5][3] = clip[15] + clip[14];

    /* Normalize the result */
    t = sqrt(_frustum[5][0] * _frustum[5][0] + _frustum[5][1] * _frustum[5][1] + _frustum[5][2] * _frustum[5][2]);
    _frustum[5][0] /= t;
    _frustum[5][1] /= t;
    _frustum[5][2] /= t;
    _frustum[5][3] /= t;
}

bool Frustum::pointInFrustum(const f32v3& pos) const {
    for (int p = 0; p < 4; p++) {
        if (_frustum[p][0] * pos.x + _frustum[p][1] * pos.y + _frustum[p][2] * pos.z + _frustum[p][3] <= 0) return false;
    }
    return true;
}

bool Frustum::sphereInFrustum(const f32v3& pos, float radius) const {
    for (int p = 0; p < 4; p++) { //*************************************** IGNORING FAR AND NEAR CLIPPING PLANE *****************************************************
        if (_frustum[p][0] * pos.x + _frustum[p][1] * pos.y + _frustum[p][2] * pos.z + _frustum[p][3] <= -radius) return false;
    }
    return true;
}
