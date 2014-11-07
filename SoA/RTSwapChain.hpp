///
/// RTSwapChain.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 6 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// A Series Of Render Targets (FBOs) That Are Swapped
///

#pragma once

#ifndef RTSwapChain_h__
#define RTSwapChain_h__

#include "GLRenderTarget.h"

namespace vorb {
    namespace core {
        namespace graphics {

            template<int N>
            class RTSwapChain {
            public:
                RTSwapChain(const ui32& w, const ui32& h) {
                    for (i32 i = 0; i < N; i++) {
                        _fbos[i].setSize(w, h);
                    }
                }

                void init(TextureInternalFormat format = TextureInternalFormat::RGBA8) {
                    for (i32 i = 0; i < N; i++) {
                        _fbos[i].init(format, TextureInternalFormat::NONE, 0);
                    }
                }
                void dispose() {
                    for (i32 i = 0; i < N; i++) {
                        _fbos[i].dispose();
                    }
                }

                void reset(ui32 textureUnit, GLRenderTarget* rt, bool isMSAA, bool shouldClear = true) {
                    const GLRenderTarget* prev = rt;

                    _current = 0;
                    if (isMSAA) {
                        ui32 w = getCurrent().getWidth();
                        ui32 h = getCurrent().getHeight();
                        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, getCurrent().getID());
                        glBindFramebuffer(GL_READ_FRAMEBUFFER, rt->getID());
                        glDrawBuffer(GL_COLOR_ATTACHMENT0);
                        glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);

                        swap();
                        prev = &getPrevious();
                    }

                    getCurrent().use();
                    if (shouldClear) glClear(GL_COLOR_BUFFER_BIT);
                    glActiveTexture(GL_TEXTURE0 + textureUnit);
                    prev->bindTexture();
                }
                void swap() {
                    _current++;
                    _current %= N;
                }
                void use(ui32 textureUnit, bool shouldClear = true) {
                    getCurrent().use();
                    if (shouldClear) glClear(GL_COLOR_BUFFER_BIT);

                    glActiveTexture(GL_TEXTURE0 + textureUnit);
                    getPrevious().bindTexture();
                }

                const GLRenderTarget& getCurrent() {
                    return _fbos[_current];
                }
                const GLRenderTarget& getPrevious() {
                    return _fbos[(_current + N - 1) % N];
                }
            private:
                GLRenderTarget _fbos[N];
                i32 _current = 0;
            };
        }
    }
}
namespace vg = vorb::core::graphics;

#endif // RTSwapChain_h__