#include "stdafx.h"
#include "RasterizerState.h"

RasterizerState::RasterizerState(bool use, CullFaceMode cullFaceMode, FrontFaceDirection frontFaceDirection) :
useCulling(use),
cullMode(cullFaceMode),
faceOrientation(frontFaceDirection) {}

void RasterizerState::set() const {
    if (useCulling) {
        glEnable(GL_CULL_FACE);
        glFrontFace(static_cast<GLenum>(faceOrientation));
        glCullFace(static_cast<GLenum>(cullMode));
    } else {
        glDisable(GL_CULL_FACE);
    }
}

const RasterizerState RasterizerState::CULL_NONE(false, CullFaceMode::BACK, FrontFaceDirection::CCW);
const RasterizerState RasterizerState::CULL_CLOCKWISE(true, CullFaceMode::BACK, FrontFaceDirection::CCW);
const RasterizerState RasterizerState::CULL_COUNTER_CLOCKWISE(true, CullFaceMode::BACK, FrontFaceDirection::CW);