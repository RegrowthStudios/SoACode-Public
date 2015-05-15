#include "stdafx.h"
#include "LenseFlareRenderer.h"

#include "ModPathResolver.h"
#include "ShaderLoader.h"
#include "Errors.h"

#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/ImageIO.h>
#include <Vorb/graphics/SamplerState.h>
#include <Vorb/graphics/ShaderManager.h>
#include <Vorb/io/IOManager.h>
#include <Vorb/io/Keg.h>

struct FlareVertex {
    f32v2 position;
    f32v2 uv;
    f32 offset;
};

struct FlareSprite {
    bool rotate = false;
    f32 offset;
    f32 size;
    ui32 textureIndex;
};
KEG_TYPE_DEF_SAME_NAME(FlareSprite, kt) {
    KEG_TYPE_INIT_ADD_MEMBER(kt, FlareSprite, rotate, BOOL);
    KEG_TYPE_INIT_ADD_MEMBER(kt, FlareSprite, offset, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, FlareSprite, size, F32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, FlareSprite, textureIndex, UI32);
}

struct FlareKegProperties {
    ui32 spritesPerRow = 1;
    f32 intensity = 1.0f;
    Array<FlareSprite> sprites;
};
KEG_TYPE_DEF_SAME_NAME(FlareKegProperties, kt) {
    KEG_TYPE_INIT_ADD_MEMBER(kt, FlareKegProperties, spritesPerRow, UI32);
    KEG_TYPE_INIT_ADD_MEMBER(kt, FlareKegProperties, intensity, F32);
    kt.addValue("sprites", keg::Value::array(offsetof(FlareKegProperties, sprites), keg::Value::custom(0, "FlareSprite", false)));
}

const int VERTS_PER_QUAD = 4;
const int INDICES_PER_QUAD = 6;

LenseFlareRenderer::LenseFlareRenderer(const ModPathResolver* textureResolver) :
m_textureResolver(textureResolver) {
    // Empty
}

LenseFlareRenderer::~LenseFlareRenderer() {
    dispose();
}

void LenseFlareRenderer::render(const f32m4& VP, const f64v3& relCamPos,
                                const f32v3& color,
                                float aspectRatio,
                                f32 size,
                                f32 intensity) {
    if (size <= 0.0f || intensity <= 0.0f) return;
    if (!m_program) {
        lazyInit();
    }

    m_program->use();

    f32v2 dims(size, size * aspectRatio);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    // Upload uniforms
    f32v3 center(-relCamPos);
    glUniform1f(m_program->getUniform("unIntensity"), intensity * m_intensity);
    glUniform3fv(m_program->getUniform("unCenter"), 1, &center[0]);
    glUniform3fv(m_program->getUniform("unColor"), 1, &color[0]);
    glUniform2fv(m_program->getUniform("unDims"), 1, &dims[0]);
    glUniformMatrix4fv(m_program->getUniform("unVP"), 1, GL_FALSE, &VP[0][0]);

    glBindVertexArray(m_vao);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDrawElements(GL_TRIANGLES, INDICES_PER_QUAD * m_numSprites, GL_UNSIGNED_SHORT, 0);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glBindVertexArray(0);

    m_program->unuse();

}

void LenseFlareRenderer::dispose() {
    if (m_program) {
        vg::ShaderManager::destroyProgram(&m_program);
    }
    if (m_texture) {
        vg::GpuMemory::freeTexture(m_texture);
    }
    if (m_vbo) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_ibo) {
        glDeleteBuffers(1, &m_ibo);
        m_vbo = 0;
    }
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
}

void LenseFlareRenderer::lazyInit() {
   
    { // Load the shader
        m_program = ShaderLoader::createProgramFromFile("Shaders/LensFlare/flare.vert",
                                                        "Shaders/LensFlare/flare.frag");
        m_unColor = m_program->getUniform("unColor");
        // Set constant uniforms
        m_program->use();
        glUniform1i(m_program->getUniform("unTexture"), 0);
        m_program->unuse();
    }
    
    { // Load the texture
        vio::Path path;
        m_textureResolver->resolvePath("Effects/lens_flares.png", path);
        vg::ScopedBitmapResource res = vg::ImageIO().load(path);
        if (!res.data) {
            fprintf(stderr, "ERROR: Failed to load Effects/lens_flares.png\n");
        }
        m_texWidth = res.width;
        m_texHeight = res.height;
        m_texture = vg::GpuMemory::uploadTexture(res.bytesUI8, res.width, res.height, &vg::SamplerState::LINEAR_CLAMP_MIPMAP);
    }

    initMesh();
}

void LenseFlareRenderer::loadSprites(FlareKegProperties& kegProps) {
    nString data;
    vio::IOManager iom;
    if (!iom.readFileToString("Data/LensFlares/SoA_defaultFlares.yml", data)) {
        pError("Couldn't find Data/LensFlares/SoA_defaultFlares.yml");
    }

    keg::ReadContext context;
    context.env = keg::getGlobalEnvironment();
    context.reader.init(data.c_str());
    keg::Node node = context.reader.getFirst();

    keg::Error err = keg::parse((ui8*)&kegProps, node, context, &KEG_GLOBAL_TYPE(FlareKegProperties));
    if (err != keg::Error::NONE) {
        fprintf(stderr, "Failed to parse Data/LensFlares/SoA_defaultFlares.yml");
    }
}

void LenseFlareRenderer::initMesh() {

    FlareKegProperties properties;
    loadSprites(properties);
    Array<FlareSprite>& sprites = properties.sprites;
    m_numSprites = sprites.size();
    m_intensity = properties.intensity;

    const f32v2 positions[4] = {
        f32v2(-1.0f, 1.0f),
        f32v2(-1.0f, -1.0f),
        f32v2(1.0f, -1.0f),
        f32v2(1.0f, 1.0f)
    };
    const f32v2 uvs[4] = {
        f32v2(0.0f, 1.0f),
        f32v2(0.0f, 0.0f),
        f32v2(1.0f, 0.0f),
        f32v2(1.0f, 1.0f)
    };
    
    ui32 xSprites = properties.spritesPerRow;
    ui32 spriteDims = m_texWidth / xSprites;
    if (spriteDims == 0) pError("Lens flare has sprite dims of 0!");
    ui32 ySprites = m_texHeight / spriteDims;
    f32v2 uvSprite(spriteDims / (f32)m_texWidth,
                   spriteDims / (f32)m_texHeight);

    const ui16 quadIndices[INDICES_PER_QUAD] = { 0, 1, 2, 2, 3, 0 };

    std::vector<FlareVertex> vertices(m_numSprites * VERTS_PER_QUAD);
    std::vector<ui16> indices(m_numSprites * INDICES_PER_QUAD);
    int index = 0;

    for (int i = 0; i < m_numSprites; i++) {
        FlareSprite& s = sprites[i];
        f32v2 uvOffset = f32v2(s.textureIndex % xSprites,
                               s.textureIndex / xSprites) * uvSprite;

        vertices[index].position = positions[0] * s.size;
        vertices[index + 1].position = positions[1] * s.size;
        vertices[index + 2].position = positions[2] * s.size;
        vertices[index + 3].position = positions[3] * s.size;
        vertices[index].uv = uvs[0] * uvSprite + uvOffset;
        vertices[index + 1].uv = uvs[1] * uvSprite + uvOffset;
        vertices[index + 2].uv = uvs[2] * uvSprite + uvOffset;
        vertices[index + 3].uv = uvs[3] * uvSprite + uvOffset;
        vertices[index].offset = s.offset;
        vertices[index + 1].offset = s.offset;
        vertices[index + 2].offset = s.offset;
        vertices[index + 3].offset = s.offset;
        index += 4;
    }
    // Set indices
    for (int i = 0; i < m_numSprites; i++) {
        for (int j = 0; j < INDICES_PER_QUAD; j++) {
            indices[i * INDICES_PER_QUAD + j] = quadIndices[j] + i * VERTS_PER_QUAD;
        }
    }

    if (m_vbo == 0) glGenBuffers(1, &m_vbo);
    if (m_ibo == 0) glGenBuffers(1, &m_ibo);
    if (m_vao == 0) glGenVertexArrays(1, &m_vao);

    // Upload data and make VAO
    glBindVertexArray(m_vao);
    vg::GpuMemory::bindBuffer(m_vbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::bindBuffer(m_ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);

    vg::GpuMemory::uploadBufferData(m_vbo, vg::BufferTarget::ARRAY_BUFFER, sizeof(FlareVertex) * vertices.size(), vertices.data());
    vg::GpuMemory::uploadBufferData(m_ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER, sizeof(ui16) * indices.size(), indices.data());

    m_program->enableVertexAttribArrays();
    glVertexAttribPointer(m_program->getAttribute("vPosition"), 2, GL_FLOAT, GL_FALSE, sizeof(FlareVertex), offsetptr(FlareVertex, position));
    glVertexAttribPointer(m_program->getAttribute("vUV"), 2, GL_FLOAT, GL_FALSE, sizeof(FlareVertex), offsetptr(FlareVertex, uv));
    glVertexAttribPointer(m_program->getAttribute("vOffset"), 1, GL_FLOAT, GL_FALSE, sizeof(FlareVertex), offsetptr(FlareVertex, offset));

    glBindVertexArray(0);
}