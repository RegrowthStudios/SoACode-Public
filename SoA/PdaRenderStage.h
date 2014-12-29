/// 
///  PdaRenderStage.h
///  Seed of Andromeda
///
///  Created by Benjamin Arnold on 2 Nov 2014
///  Copyright 2014 Regrowth Studios
///  All Rights Reserved
///  
///  This file implements the render stage for PDA rendering
///

#pragma once

#ifndef PdaRenderStage_h__
#define PdaRenderStage_h__

#include <Vorb/IRenderStage.h>

class PDA;

class PdaRenderStage : public vg::IRenderStage {
public:
    PdaRenderStage(const PDA* pda);

    /// Draws the render stage
    virtual void draw() override;
private:
    const PDA* _pda; ///< Handle to the PDA
};

#endif // PdaRenderStage_h__