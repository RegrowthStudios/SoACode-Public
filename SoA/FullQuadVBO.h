///
/// FullQuadVBO.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 6 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// A mesh for a Fullscreen quad
///

#pragma once

#ifndef FullQuadVBO_h__
#define FullQuadVBO_h__

namespace vorb {
    namespace core {
        namespace graphics {
            class FullQuadVBO {
            public:
                void init();
                void dispose();

                void draw();
            private:
                union {
                    struct {
                        ui32 _vb;
                        ui32 _ib;
                    };
                    ui32 _buffers[2];
                };
                ui32 _vao;
            };
        }
    }
}
namespace vg = vorb::core::graphics;

#endif // FullQuadVBO_h__