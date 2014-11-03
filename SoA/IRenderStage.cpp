#include "stdafx.h"
#include "IRenderStage.h"


vg::IRenderStage::IRenderStage(const Camera* camera /* = nullptr */) :
    _inputFbo(nullptr),
    _camera(camera),
    _isVisible(true) {
    // Empty
}


vg::IRenderStage::~IRenderStage() {
    // Empty
}