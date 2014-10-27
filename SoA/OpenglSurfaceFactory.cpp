#include "stdafx.h"

#include "OpenglSurfaceFactory.h"

namespace Awesomium {

    OpenglSurface::OpenglSurface(int width, int height) : _width(width), _height(height) {
        glGenTextures(1, &_textureID);
        glBindTexture(GL_TEXTURE_2D, _textureID);

        if (glewIsSupported("glTexStorage2D")) {
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, _width, _height);
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        }

        _rowSpan = _width * 4;

        _pixels = new ui8[_width * _height * 4];

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    OpenglSurface::~OpenglSurface() {
        delete[] _pixels;
    }

    void OpenglSurface::Paint(unsigned char *src_buffer, int src_row_span, const Awesomium::Rect &src_rect, const Awesomium::Rect &dest_rect) {
        glBindTexture(GL_TEXTURE_2D, _textureID);

        //Copy the pixel data to the _pixels buffer
        for (int y = 0; y < src_rect.height; y++) {
            memcpy(_pixels + y * (src_rect.width * 4), src_buffer + (y + src_rect.y) * src_row_span + src_rect.x * 4, src_rect.width * 4);
        }

        //Add it do the GL texture
        glTexSubImage2D(GL_TEXTURE_2D, 0, dest_rect.x, dest_rect.y, dest_rect.width, dest_rect.height, GL_BGRA, GL_UNSIGNED_BYTE, _pixels);
    }

    //TODO: Implement this!
    void OpenglSurface::Scroll(int dx, int dy, const Awesomium::Rect &clip_rect) {
        std::cout << "ERROR: SCROLLING NOT IMPLEMENTED!!!\n";
    }

    Awesomium::Surface *OpenglSurfaceFactory::CreateSurface(Awesomium::WebView *view, int width, int height) {
        OpenglSurface* newSurface = new OpenglSurface(width, height);

        return newSurface;
    }

    void OpenglSurfaceFactory::DestroySurface(Awesomium::Surface *surface) {
        delete surface;
    }
}