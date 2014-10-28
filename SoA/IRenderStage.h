/// 
///  IRenderStage.h
///  Vorb Engine
///
///  Created by Ben Arnold on 28 Oct 2014
///  Copyright 2014 Regrowth Studios
///  All Rights Reserved
///  
///  This file provides an interface for a render pipeline
///  stage.
///

#pragma once

#ifndef IRenderStage_h_
#define IRenderStage_h_

namespace vorb {
namespace graphics {

class FrameBuffer;

class IRenderStage
{
public:
    IRenderStage();
    virtual ~IRenderStage();

    /// Renders the stage
    virtual void render() = 0;
protected:
    FrameBuffer* _renderTarget;
};

}
}

namespace vg = vorb::graphics;

#endif // IRenderStage_h_