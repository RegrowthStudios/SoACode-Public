#include "stdafx.h"
#include "SpriteFont.h"

#include <boost\filesystem.hpp>

#include "ImageSaver.h"
#include "SpriteBatch.h"

i32 closestPow2(i32 i) {
    i--;
    i32 pi = 1;
    while (i > 0) {
        i >>= 1;
        pi <<= 1;
    }
    return pi;
}

SpriteFont::SpriteFont(const cString font, int size, char cs, char ce) {
    TTF_Font* f = TTF_OpenFont(font, size);
    _fontHeight = TTF_FontHeight(f);
    _regStart = cs;
    _regLength = ce - cs + 1;
    i32 padding = size / 8;

    // First Measure All The Regions
    i32v4* glyphRects = new i32v4[_regLength];
    i32 i = 0, advance;
    for (char c = cs; c <= ce; c++) {
        TTF_GlyphMetrics(f, c, &glyphRects[i].x, &glyphRects[i].z, &glyphRects[i].y, &glyphRects[i].w, &advance);
        glyphRects[i].z -= glyphRects[i].x;
        glyphRects[i].x = 0;
        glyphRects[i].w -= glyphRects[i].y;
        glyphRects[i].y = 0;
        i++;
    }

    // Find Best Partitioning Of Glyphs
    i32 rows = 1, w, h, bestWidth = 0, bestHeight = 0, area = 4096 * 4096, bestRows = 0;
    std::vector<int>* bestPartition = nullptr;
    while (rows <= _regLength) {
        h = rows * (padding + _fontHeight) + padding;
        auto gr = createRows(glyphRects, _regLength, rows, padding, w);

        // Desire A Power Of 2 Texture
        w = closestPow2(w);
        h = closestPow2(h);

        // A Texture Must Be Feasible
        if (w > 4096 || h > 4096) {
            rows++;
            delete[] gr;
            continue;
        }

        // Check For Minimal Area
        if (area >= w * h) {
            if (bestPartition) delete[] bestPartition;
            bestPartition = gr;
            bestWidth = w;
            bestHeight = h;
            bestRows = rows;
            area = bestWidth * bestHeight;
            rows++;
        } else {
            delete[] gr;
            break;
        }
    }

    // Can A Bitmap Font Be Made?
    if (!bestPartition) return;

    // Create The Texture
    glGenTextures(1, &_texID);
    glBindTexture(GL_TEXTURE_2D, _texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bestWidth, bestHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Now Draw All The Glyphs
    SDL_Color fg = { 255, 255, 255, 255 };
    i32 ly = padding;
    for (i32 ri = 0; ri < bestRows; ri++) {
        i32 lx = padding;
        for (i32 ci = 0; ci < bestPartition[ri].size(); ci++) {
            i32 gi = bestPartition[ri][ci];

            SDL_Surface* glyphSurface = TTF_RenderGlyph_Blended(f, (char)(cs + gi), fg);

            // Pre-multiplication Occurs Here
            ubyte* sp = (ubyte*)glyphSurface->pixels;
            i32 cp = glyphSurface->w * glyphSurface->h * 4;
            for (i32 i = 0; i < cp; i += 4) {
                f32 a = sp[i + 3] / 255.0f;
                sp[i] *= a;
                sp[i + 1] = sp[i];
                sp[i + 2] = sp[i];
            }

            // Save Glyph Image And Update Coordinates
            glTexSubImage2D(GL_TEXTURE_2D, 0, lx, ly, glyphSurface->w, glyphSurface->h, GL_BGRA, GL_UNSIGNED_BYTE, glyphSurface->pixels);
            glyphRects[gi].x = lx;
            glyphRects[gi].y = ly;
            glyphRects[gi].z = glyphSurface->w;
            glyphRects[gi].w = glyphSurface->h;

            SDL_FreeSurface(glyphSurface);
            glyphSurface = nullptr;

            lx += glyphRects[gi].z + padding;
        }
        ly += _fontHeight + padding;
    }

    // Draw The Unsupported Glyph
    i32 rs = padding - 1;
    i32* pureWhiteSquare = new i32[rs * rs];
    memset(pureWhiteSquare, 0xffffffff, rs * rs * sizeof(i32));
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, rs, rs, GL_RGBA, GL_UNSIGNED_BYTE, pureWhiteSquare);
    delete[] pureWhiteSquare;
    pureWhiteSquare = nullptr;

    // Create SpriteBatch Glyphs
    _glyphs = new CharGlyph[_regLength + 1];
    for (i = 0; i < _regLength; i++) {
        _glyphs[i].character = (char)(cs + i);
        _glyphs[i].size = f32v2(glyphRects[i].z, glyphRects[i].w);
        _glyphs[i].uvRect = f32v4(
            (float)glyphRects[i].x / (float)bestWidth,
            (float)glyphRects[i].y / (float)bestHeight,
            (float)glyphRects[i].z / (float)bestWidth,
            (float)glyphRects[i].w / (float)bestHeight
            );
    }
    _glyphs[_regLength].character = ' ';
    _glyphs[_regLength].size = _glyphs[0].size;
    _glyphs[_regLength].uvRect = f32v4(0, 0, (float)rs / (float)bestWidth, (float)rs / (float)bestHeight);

#ifdef DEBUG
    // Save An Image
    std::vector<ui8> pixels;
    pixels.resize(bestWidth * bestHeight * 4);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
    char buffer[512];
    sprintf(buffer, "SFont_%s_%s_%d.png", TTF_FontFaceFamilyName(f), TTF_FontFaceStyleName(f), size);
    vg::ImageSaver::savePng(buffer, bestWidth, bestHeight, pixels);
#endif // DEBUG

    glBindTexture(GL_TEXTURE_2D, 0);
    delete[] glyphRects;
    delete[] bestPartition;
    TTF_CloseFont(f);
}

void SpriteFont::dispose() {
    if (_texID != 0) {
        glDeleteTextures(1, &_texID);
        _texID = 0;
    }
    if (_glyphs) {
        _glyphs = nullptr;
        delete[] _glyphs;
    }
}

void SpriteFont::getInstalledFonts(std::map<nString, nString>& fontFileDictionary) {
#ifdef DEBUG
    ui32 startTime = SDL_GetTicks(), searchCount = 0;
#endif // DEBUG
    boost::filesystem::path fontDirectory(getenv("SystemRoot"));
    fontDirectory /= "Fonts";
    boost::filesystem::directory_iterator dirIter(fontDirectory);
    nString fontExtTTF = ".ttf";
    nString fontExtFON = ".fon";
    while (true) {
        i32* m_impCheck = (i32*)&dirIter;
        if (*m_impCheck == 0) break;

        auto dirFile = dirIter->path();
#ifdef DEBUG
        searchCount++;
#endif // DEBUG
        if (!boost::filesystem::is_directory(dirFile)) {
            nString fExt = dirFile.extension().string();
            if (fExt == fontExtTTF || fExt == fontExtFON) {
                nString fontFile = dirFile.string();
                TTF_Font* font = TTF_OpenFont(fontFile.c_str(), 12);
                if (font) {
                    nString fontName = TTF_FontFaceFamilyName(font);
                    fontName += " - " + nString(TTF_FontFaceStyleName(font));
                    fontFileDictionary[fontName] = fontFile;

                    TTF_CloseFont(font);
                    font = nullptr;
                }
            }
        }
        dirIter++;
    }
#ifdef DEBUG
    printf("Found %d System Fonts Out Of %d Files\nTime Spent: %d ms\n", fontFileDictionary.size(), searchCount, SDL_GetTicks() - startTime);
#endif // DEBUG
    return;
}

std::vector<i32>* SpriteFont::createRows(i32v4* rects, i32 rectsLength, i32 r, i32 padding, i32& w) {
    // Blank Initialize
    std::vector<i32>* l = new std::vector<i32>[r]();
    i32* cw = new i32[r]();
    for (int i = 0; i < r; i++) {
        cw[i] = padding;
    }

    // Loop Through All Glyphs
    for (i32 i = 0; i < rectsLength; i++) {
        // Find Row For Placement
        i32 ri = 0;
        for (i32 rii = 1; rii < r; rii++)
        if (cw[rii] < cw[ri]) ri = rii;

        // Add Width To That Row
        cw[ri] += rects[i].z + padding;

        // Add Glyph To The Row List
        l[ri].push_back(i);
    }

    // Find The Max Width
    w = 0;
    for (i32 i = 0; i < r; i++) {
        if (cw[i] > w) w = cw[i];
    }

    return l;
}

f32v2 SpriteFont::measure(const cString s) {
    f32v2 size(0, _fontHeight);
    float cw = 0;
    for (int si = 0; s[si] != 0; si++) {
        char c = s[si];
        if (s[si] == '\n') {
            size.y += _fontHeight;
            if (size.x < cw)
                size.x = cw;
            cw = 0;
        } else {
            // Check For Correct Glyph
            int gi = c - _regStart;
            if (gi < 0 || gi >= _regLength)
                gi = _regLength;
            cw += _glyphs[gi].size.x;
        }
    }
    if (size.x < cw)
        size.x = cw;
    return size;
}

void SpriteFont::draw(SpriteBatch* batch, const cString s, f32v2 position, f32v2 scaling, ColorRGBA8 tint, f32 depth) {
    f32v2 tp = position;
    for (int si = 0; s[si] != 0; si++) {
        char c = s[si];
        if (s[si] == '\n') {
            tp.y += _fontHeight * scaling.y;
            tp.x = position.x;
        } else {
            // Check For Correct Glyph
            int gi = c - _regStart;
            if (gi < 0 || gi >= _regLength)
                gi = _regLength;
            batch->draw(_texID, &_glyphs[gi].uvRect, tp, _glyphs[gi].size * scaling, tint, depth);
            tp.x += _glyphs[gi].size.x * scaling.x;
        }
    }
}
