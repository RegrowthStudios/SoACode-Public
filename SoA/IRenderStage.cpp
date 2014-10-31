#include "stdafx.h"
#include "IRenderStage.h"


vg::IRenderStage::IRenderStage(Camera* camera /* = nullptr */) :
    _inputFbo(nullptr),
    _camera(camera) {
    // Empty
}


vg::IRenderStage::~IRenderStage() {
    // Empty
}

