/// 
///  IRenderStage.h
///  Vorb Engine
///
///  Created by Ben Arnold on 28 Oct 2014
///  Copyright 2014 Regrowth Studios
///  All Rights Reserved
///  
///  This file provides an abstract interface for a render 
///  stage.
///

#pragma once

#ifndef IRenderStage_h_
#define IRenderStage_h_

class Camera;

namespace vorb {
    namespace core {
        namespace graphics {

            class IRenderStage
            {
            public:
                IRenderStage(const Camera* camera = nullptr);
                virtual ~IRenderStage();

                /// Renders the stage
                virtual void draw() = 0;

                /// Check if the stage is visible
                virtual bool isVisible() const { return _isVisible; }

                /// Sets the visibility of the stage
                virtual void setIsVisible(bool isVisible) { _isVisible = isVisible; }

                /// Sets the camera
                virtual void setCamera(const Camera* camera) { _camera = camera; }
            protected:
                const Camera* _camera; ///< Optional Camera, not needed for post processing stages
                bool _isVisible; ///< Determines if the stage should be rendered
            };

        }
    }
}

namespace vg = vorb::core::graphics;

#endif // IRenderStage_h_