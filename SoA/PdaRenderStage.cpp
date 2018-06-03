#include "stdafx.h"
#include "PdaRenderStage.h"

#include "PDA.h"


PdaRenderStage::PdaRenderStage() {
    // Empty
}

void PdaRenderStage::hook(const PDA* pda) {
    _pda = pda;
}

void PdaRenderStage::render(const Camera* camera) {
    if (_pda->isOpen()) {
        _pda->draw();
    }
}
