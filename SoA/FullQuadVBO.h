///
/// FullQuadVBO.h
/// Vorb Engine
///
/// Created by Cristian Zaloj on 6 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// A mesh for a fullscreen quad
///

#pragma once

#ifndef FullQuadVBO_h__
#define FullQuadVBO_h__

namespace vorb {
    namespace core {
        namespace graphics {
            /// Wrapper over common functionality to draw a quad across the entire screen
            class FullQuadVBO {
            public:
                /// Initialize all OpenGL resources
                /// @param attrLocation: Position attribute location for VAO
                void init(i32 attrLocation = 0);
                /// Dispose all OpenGL resources
                void dispose();

                /// Binds vertex array, index buffer, and issues a draw command
                void draw();
            private:
                union {
                    struct {
                        ui32 _vb; ///< Vertex buffer ID
                        ui32 _ib; ///< Index buffer ID
                    };
                    ui32 _buffers[2]; ///< Storage for both buffers used by this mesh
                };
                ui32 _vao; ///< VAO with vertex attribute pointing to 0
            };
        }
    }
}
namespace vg = vorb::core::graphics;

#endif // FullQuadVBO_h__