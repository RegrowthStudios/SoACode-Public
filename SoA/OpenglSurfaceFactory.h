#pragma once

#include <Awesomium/BitmapSurface.h>
#include <Awesomium/DataPak.h>
#include <Awesomium/DataSource.h>
#include <Awesomium/STLHelpers.h>
#include <Awesomium/WebCore.h>

namespace Awesomium {
    class OpenglSurface : public Awesomium::Surface {
    public:
        OpenglSurface(int width, int height);
        ~OpenglSurface();

        void Paint(unsigned char *src_buffer, int src_row_span, const Awesomium::Rect &src_rect, const Awesomium::Rect &dest_rect);
        void Scroll(int dx, int dy, const Awesomium::Rect &clip_rect);

        bool getIsDirty() const { return _isDirty; }
        void setIsDirty(bool isDirty) { _isDirty = isDirty; }

        GLuint getTextureID() const { return _textureID; }
    private:
        ui8* _pixels;
        int _width;
        int _height;
        int _rowSpan;
        bool _isDirty;

        GLuint _textureID;
    };


    class OpenglSurfaceFactory : public Awesomium::SurfaceFactory {
    public:
        Awesomium::Surface *CreateSurface(Awesomium::WebView *view, int width, int height);
        void DestroySurface(Awesomium::Surface *surface);
    };
}