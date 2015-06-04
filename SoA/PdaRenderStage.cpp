#include "stdafx.h"
#include "PdaRenderStage.h"

#include "PDA.h"


PdaRenderStage::PdaRenderStage() {
    // Empty
}

void PdaRenderStage::init(const PDA* pda) {
    _pda = pda;
}

void PdaRenderStage::render() {
    if (_pda->isOpen()) {
        _pda->draw();
    }
}
