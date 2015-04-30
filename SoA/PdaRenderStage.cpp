#include "stdafx.h"
#include "PdaRenderStage.h"

#include "PDA.h"


PdaRenderStage::PdaRenderStage(const PDA* pda) :
    _pda(pda) {
    // Empty
}

void PdaRenderStage::render() {
    if (_pda->isOpen()) {
        _pda->draw();
    }
}
