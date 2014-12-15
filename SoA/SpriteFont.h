///
/// SpriteFont.h
/// Vorb Engine
///
/// Created by Cristian Zaloj on 30 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// 
///

#pragma once

#ifndef SpriteFont_h__
#define SpriteFont_h__

#include <TTF\SDL_ttf.h>

#include "gtypes.h"

class SpriteBatch;

struct CharGlyph {
public:
    char character;
    f32v4 uvRect;
    f32v2 size;
};

#define FIRST_PRINTABLE_CHAR ((char)32)
#define LAST_PRINTABLE_CHAR ((char)126)

class SpriteFont {
    friend class SpriteBatch;
public:
    SpriteFont(const cString font, i32 size, char cs, char ce);
    SpriteFont(const cString font, i32 size) :
        SpriteFont(font, size, FIRST_PRINTABLE_CHAR, LAST_PRINTABLE_CHAR) {}
    void dispose();

    const i32& getFontHeight() const {
        return _fontHeight;
    }

    static void getInstalledFonts(std::map<nString, nString>& fontFileDictionary);

    f32v2 measure(const cString s);
private:
    void draw(SpriteBatch* batch, const cString s, f32v2 position, f32v2 scaling, color4 tint, f32 depth);

    static std::vector<i32>* createRows(i32v4* rects, i32 rectsLength, i32 r, i32 padding, i32& w);

    i32 _regStart, _regLength;
    CharGlyph* _glyphs;
    i32 _fontHeight;

    VGTexture _texID;
};

#endif // SpriteFont_h__