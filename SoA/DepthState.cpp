#include "stdafx.h"
#include "DepthState.h"

DepthState::DepthState(bool read, DepthFunction depthFunction, bool write) :
shouldRead(read),
depthFunc(depthFunction),
shouldWrite(write) {}

void DepthState::set() const {
    if (shouldRead || shouldWrite) {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(shouldWrite);
        glDepthFunc(static_cast<GLenum>(depthFunc));
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}

const DepthState DepthState::FULL(true, DepthFunction::LESS_THAN, true);
const DepthState DepthState::WRITE(false, DepthFunction::ALWAYS, true);
const DepthState DepthState::READ(true, DepthFunction::LESS_THAN, false);
const DepthState DepthState::NONE(false, DepthFunction::ALWAYS, false);

