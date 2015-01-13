///
/// TerrainGenTextures.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 18 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// The output textures for terrain generation
///

#pragma once

#ifndef TerrainGenTextures_h__
#define TerrainGenTextures_h__

#include <Vorb/graphics/gtypes.h>
#include <Vorb/graphics/glenums.h>

#define TERRAINGEN_INTERNAL_FORMAT vg::TextureInternalFormat::RGBA32F

class TerrainGenTextures {
public:
    struct TextureIDs {
    public:
        VGTexture height_temp_hum;
    };

    ~TerrainGenTextures();

    void init(const ui32v2& dims);

    /// @return OpenGL texture IDs
    const TerrainGenTextures::TextureIDs& getTextureIDs() const {
        return m_tex;
    }

    void use();
    
    static void unuse();

    void destroy();
private:
    void initTarget(const ui32v2& size, const ui32& texID, const vg::TextureInternalFormat& format, const ui32& attachment);
   
    union {
        TextureIDs m_tex; ///< Named texture targets
        VGTexture m_textures[1]; ///< All 1 textures
    };

    VGFramebuffer m_fbo = 0;
    ui32v2 m_dims = ui32v2(0);
};

#endif // TerrainGenTextures_h__