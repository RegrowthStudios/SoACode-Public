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

class Camera;

namespace vorb {
    namespace core {
        namespace graphics {

            class FrameBuffer;

            class IRenderStage
            {
            public:
                IRenderStage(Camera* camera = nullptr);
                virtual ~IRenderStage();

                virtual void setState(FrameBuffer* inputFbo = nullptr) = 0;

                /// Renders the stage
                virtual void draw() = 0;

                /// Check if the stage is visible
                virtual bool isVisible() = 0;

                /// Sets the camera
                virtual void setCamera(Camera* camera) { _camera = camera; }

                /// Sets the render target
                virtual void setInputFbo(FrameBuffer* inputFbo) { _inputFbo = inputFbo; }
            protected:
                FrameBuffer* _inputFbo; ///< Optional Render Target
                Camera* _camera; ///< Optional Camera, not needed for post processing stages
            };

        }
    }
}

namespace vg = vorb::core::graphics;

#endif // IRenderStage_h_