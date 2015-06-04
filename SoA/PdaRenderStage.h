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

#include <Vorb/graphics/IRenderStage.h>

class PDA;

class PdaRenderStage : public vg::IRenderStage {
public:
    PdaRenderStage();

    void init(const PDA* pda);

    /// Draws the render stage
    virtual void render() override;
private:
    const PDA* _pda = nullptr; ///< Handle to the PDA
};

#endif // PdaRenderStage_h__