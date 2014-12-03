#include "stdafx.h"
#include "SpriteBatch.h"

#include <tchar.h>

#include "DepthState.h"
#include "GLEnums.h"
#include "GLProgram.h"
#include "RasterizerState.h"
#include "SamplerState.h"
#include "SpriteFont.h"

#pragma region Shader Source
const cString VS_SRC = R"(
uniform mat4 World;
uniform mat4 VP;

in vec4 vPosition;
in vec2 vUV;
in vec4 vUVRect;
in vec4 vTint;

out vec2 fUV;
flat out vec4 fUVRect;
out vec4 fTint;

void main() {
    fTint = vTint;
    fUV = vUV;
    fUVRect = vUVRect;
    vec4 worldPos = World * vPosition;
    gl_Position = VP * worldPos;
}
)";
const cString FS_SRC = R"(
uniform sampler2D SBTex;

in vec2 fUV;
flat in vec4 fUVRect;
in vec4 fTint;

out vec4 fColor;

void main() {
    fColor = texture(SBTex, fract(fUV.xy) * fUVRect.zw + fUVRect.xy) * fTint;
}
)";
#pragma endregion

VertexSpriteBatch::VertexSpriteBatch(const f32v3& pos, const f32v2& uv, const f32v4& uvr, const ColorRGBA8& color) :
position(pos),
uv(uv),
uvRect(uvr),
color(color) {

}
VertexSpriteBatch::VertexSpriteBatch() :
position(0, 0, 0),
uv(0, 0),
uvRect(0, 0, 0, 0),
color(0, 0, 0, 0) {}

SpriteGlyph::SpriteGlyph() :
SpriteGlyph(0, 0) {}
SpriteGlyph::SpriteGlyph(ui32 texID, f32 d) :
textureID(texID), depth(d) {}

SpriteBatch::SpriteBatch(bool isDynamic /*= true*/, bool doInit /*= false*/) :
    _bufUsage(isDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW),
    _glyphCapacity(0),
    _vao(0), _vbo(0) {
    if (doInit) init();
}
SpriteBatch::~SpriteBatch() {
    _batchRecycler.freeAll();
    _glyphRecycler.freeAll();
}

void SpriteBatch::init() {
    createProgram();
    createVertexArray();
    createPixelTexture();
}
void SpriteBatch::dispose() {
    if (_vbo != 0) {
        glDeleteBuffers(1, &_vbo);
        _vbo = 0;
    }
    if (_vao != 0) {
        glDeleteVertexArrays(1, &_vao);
        _vao = 0;
    }

    if (_texPixel != 0) {
        glDeleteTextures(1, &_texPixel);
        _texPixel = 0;
    }
}

void SpriteBatch::begin() {
    if (_glyphs.size() > 0) {
        // Why Would This Ever Happen?
        for (auto glyph : _glyphs) _glyphRecycler.recycle(glyph);
        _glyphs.swap(std::vector<SpriteGlyph*>());
    }
    for (auto batch : _batches) _batchRecycler.recycle(batch);
    _batches.swap(std::vector<SpriteBatchCall*>());
}

void SpriteBatch::draw(ui32 t, f32v4* uvRect, f32v2* uvTiling, f32v2 position, f32v2 offset, f32v2 size, f32 rotation, const ColorRGBA8& tint, f32 depth /*= 0.0f*/) {
    f32v4 uvr = uvRect != nullptr ? *uvRect : f32v4(0, 0, 1, 1);
    f32v2 uvt = uvTiling != nullptr ? *uvTiling : f32v2(1, 1);
    SpriteGlyph* g = _glyphRecycler.create();
    g->textureID = t == 0 ? _texPixel : t;
    g->depth = depth;

    f32 rxx = (f32)cos(-rotation);
    f32 rxy = (f32)sin(-rotation);
    f32 cl = size.x * (-offset.x);
    f32 cr = size.x * (1 - offset.x);
    f32 ct = size.y * (-offset.y);
    f32 cb = size.y * (1 - offset.y);

    g->vtl.position.x = (cl * rxx) + (ct * rxy) + position.x;
    g->vtl.position.y = (cl * -rxy) + (ct * rxx) + position.y;
    g->vtl.position.z = depth;
    g->vtl.uv.x = 0;
    g->vtl.uv.y = 0;
    g->vtl.uvRect = uvr;
    g->vtl.color = tint;

    g->vtr.position.x = (cr * rxx) + (ct * rxy) + position.x;
    g->vtr.position.y = (cr * -rxy) + (ct * rxx) + position.y;
    g->vtr.position.z = depth;
    g->vtr.uv.x = uvt.x;
    g->vtr.uv.y = 0;
    g->vtr.uvRect = uvr;
    g->vtr.color = tint;

    g->vbl.position.x = (cl * rxx) + (cb * rxy) + position.x;
    g->vbl.position.y = (cl * -rxy) + (cb * rxx) + position.y;
    g->vbl.position.z = depth;
    g->vbl.uv.x = 0;
    g->vbl.uv.y = uvt.y;
    g->vbl.uvRect = uvr;
    g->vbl.color = tint;

    g->vbr.position.x = (cr * rxx) + (cb * rxy) + position.x;
    g->vbr.position.y = (cr * -rxy) + (cb * rxx) + position.y;
    g->vbr.position.z = depth;
    g->vbr.uv.x = uvt.x;
    g->vbr.uv.y = uvt.y;
    g->vbr.uvRect = uvr;
    g->vbr.color = tint;

    _glyphs.push_back(g);
}
void SpriteBatch::draw(ui32 t, f32v4* uvRect, f32v2* uvTiling, f32v2 position, f32v2 offset, f32v2 size, const ColorRGBA8& tint, f32 depth /*= 0.0f*/) {
    f32v4 uvr = uvRect != nullptr ? *uvRect : f32v4(0, 0, 1, 1);
    f32v2 uvt = uvTiling != nullptr ? *uvTiling : f32v2(1, 1);
    SpriteGlyph* g = _glyphRecycler.create();
    g->textureID = t == 0 ? _texPixel : t;
    g->depth = depth;

    f32 cl = size.x * (-offset.x);
    f32 cr = size.x * (1 - offset.x);
    f32 ct = size.y * (-offset.y);
    f32 cb = size.y * (1 - offset.y);

    g->vtl.position.x = cl + position.x;
    g->vtl.position.y = ct + position.y;
    g->vtl.position.z = depth;
    g->vtl.uv.x = 0;
    g->vtl.uv.y = 0;
    g->vtl.uvRect = uvr;
    g->vtl.color = tint;

    g->vtr.position.x = cr + position.x;
    g->vtr.position.y = ct + position.y;
    g->vtr.position.z = depth;
    g->vtr.uv.x = uvt.x;
    g->vtr.uv.y = 0;
    g->vtr.uvRect = uvr;
    g->vtr.color = tint;

    g->vbl.position.x = cl + position.x;
    g->vbl.position.y = cb + position.y;
    g->vbl.position.z = depth;
    g->vbl.uv.x = 0;
    g->vbl.uv.y = uvt.y;
    g->vbl.uvRect = uvr;
    g->vbl.color = tint;

    g->vbr.position.x = cr + position.x;
    g->vbr.position.y = cb + position.y;
    g->vbr.position.z = depth;
    g->vbr.uv.x = uvt.x;
    g->vbr.uv.y = uvt.y;
    g->vbr.uvRect = uvr;
    g->vbr.color = tint;

    _glyphs.push_back(g);
}
void SpriteBatch::draw(ui32 t, f32v4* uvRect, f32v2* uvTiling, f32v2 position, f32v2 size, const ColorRGBA8& tint, f32 depth /*= 0.0f*/) {
    f32v4 uvr = uvRect != nullptr ? *uvRect : f32v4(0, 0, 1, 1);
    f32v2 uvt = uvTiling != nullptr ? *uvTiling : f32v2(1, 1);
    SpriteGlyph* g = _glyphRecycler.create();
    g->textureID = t == 0 ? _texPixel : t;
    g->depth = depth;

    g->vtl.position.x = position.x;
    g->vtl.position.y = position.y;
    g->vtl.position.z = depth;
    g->vtl.uv.x = 0;
    g->vtl.uv.y = 0;
    g->vtl.uvRect = uvr;
    g->vtl.color = tint;

    g->vtr.position.x = size.x + position.x;
    g->vtr.position.y = position.y;
    g->vtr.position.z = depth;
    g->vtr.uv.x = uvt.x;
    g->vtr.uv.y = 0;
    g->vtr.uvRect = uvr;
    g->vtr.color = tint;

    g->vbl.position.x = position.x;
    g->vbl.position.y = size.y + position.y;
    g->vbl.position.z = depth;
    g->vbl.uv.x = 0;
    g->vbl.uv.y = uvt.y;
    g->vbl.uvRect = uvr;
    g->vbl.color = tint;

    g->vbr.position.x = size.x + position.x;
    g->vbr.position.y = size.y + position.y;
    g->vbr.position.z = depth;
    g->vbr.uv.x = uvt.x;
    g->vbr.uv.y = uvt.y;
    g->vbr.uvRect = uvr;
    g->vbr.color = tint;

    _glyphs.push_back(g);
}
void SpriteBatch::draw(ui32 t, f32v4* uvRect, f32v2 position, f32v2 size, const ColorRGBA8& tint, f32 depth /*= 0.0f*/) {
    f32v4 uvr = uvRect != nullptr ? *uvRect : f32v4(0, 0, 1, 1);
    SpriteGlyph* g = _glyphRecycler.create();
    g->textureID = t == 0 ? _texPixel : t;
    g->depth = depth;

    g->vtl.position.x = position.x;
    g->vtl.position.y = position.y;
    g->vtl.position.z = depth;
    g->vtl.uv.x = 0;
    g->vtl.uv.y = 0;
    g->vtl.uvRect = uvr;
    g->vtl.color = tint;

    g->vtr.position.x = size.x + position.x;
    g->vtr.position.y = position.y;
    g->vtr.position.z = depth;
    g->vtr.uv.x = 1;
    g->vtr.uv.y = 0;
    g->vtr.uvRect = uvr;
    g->vtr.color = tint;

    g->vbl.position.x = position.x;
    g->vbl.position.y = size.y + position.y;
    g->vbl.position.z = depth;
    g->vbl.uv.x = 0;
    g->vbl.uv.y = 1;
    g->vbl.uvRect = uvr;
    g->vbl.color = tint;

    g->vbr.position.x = size.x + position.x;
    g->vbr.position.y = size.y + position.y;
    g->vbr.position.z = depth;
    g->vbr.uv.x = 1;
    g->vbr.uv.y = 1;
    g->vbr.uvRect = uvr;
    g->vbr.color = tint;

    _glyphs.push_back(g);
}
void SpriteBatch::draw(ui32 t, f32v2 position, f32v2 size, const ColorRGBA8& tint, f32 depth /*= 0.0f*/) {
    SpriteGlyph* g = _glyphRecycler.create();
    g->textureID = t == 0 ? _texPixel : t;
    g->depth = depth;

    g->vtl.position.x = position.x;
    g->vtl.position.y = position.y;
    g->vtl.position.z = depth;
    g->vtl.uv.x = 0;
    g->vtl.uv.y = 0;
    g->vtl.uvRect = f32v4(0, 0, 1, 1);
    g->vtl.color = tint;

    g->vtr.position.x = size.x + position.x;
    g->vtr.position.y = position.y;
    g->vtr.position.z = depth;
    g->vtr.uv.x = 1;
    g->vtr.uv.y = 0;
    g->vtr.uvRect = f32v4(0, 0, 1, 1);
    g->vtr.color = tint;

    g->vbl.position.x = position.x;
    g->vbl.position.y = size.y + position.y;
    g->vbl.position.z = depth;
    g->vbl.uv.x = 0;
    g->vbl.uv.y = 1;
    g->vbl.uvRect = f32v4(0, 0, 1, 1);
    g->vbl.color = tint;

    g->vbr.position.x = size.x + position.x;
    g->vbr.position.y = size.y + position.y;
    g->vbr.position.z = depth;
    g->vbr.uv.x = 1;
    g->vbr.uv.y = 1;
    g->vbr.uvRect = f32v4(0, 0, 1, 1);
    g->vbr.color = tint;

    _glyphs.push_back(g);
}
void SpriteBatch::drawString(SpriteFont* font, const cString s, f32v2 position, f32v2 scaling, const ColorRGBA8& tint, f32 depth /*= 0.0f*/) {
    if (s == nullptr) s = "";
    font->draw(this, s, position, scaling, tint, depth);
}
void SpriteBatch::drawString(SpriteFont* font, const cString s, f32v2 position, f32 desiredHeight, f32 scaleX, const ColorRGBA8& tint, f32 depth /*= 0.0f*/) {
    if (s == nullptr) s = "";
    f32v2 scaling(desiredHeight / (f32)font->getFontHeight());
    scaling.x *= scaleX;
    font->draw(this, s, position, scaling, tint, depth);
}

void SpriteBatch::end(SpriteSortMode ssm /*= SpriteSortMode::Texture*/) {
    sortGlyphs(ssm);
    generateBatches();
}

void SpriteBatch::renderBatch(f32m4 mWorld, f32m4 mCamera, /*const BlendState* bs = nullptr,*/ const SamplerState* ss /*= nullptr*/, const DepthState* ds /*= nullptr*/, const RasterizerState* rs /*= nullptr*/, vg::GLProgram* shader /*= nullptr*/) {
    //if (bs == nullptr) bs = BlendState::PremultipliedAlphaBlend;
    if (ds == nullptr) ds = &DepthState::NONE;
    if (rs == nullptr) rs = &RasterizerState::CULL_NONE;
    if (ss == nullptr) ss = &SamplerState::LINEAR_WRAP;
    if (shader == nullptr) shader = _program;

    // Setup The Shader
    //bs->set();
    ds->set();
    rs->set();

    shader->use();

    glUniformMatrix4fv(shader->getUniform("World"), 1, false, (f32*)&mWorld);
    glUniformMatrix4fv(shader->getUniform("VP"), 1, false, (f32*)&mCamera);

    glBindVertexArray(_vao);

    // Draw All The Batches
    i32 bc = _batches.size();
    for (int i = 0; i < bc; i++) {
        SpriteBatchCall* batch = _batches[i];

        glActiveTexture(GL_TEXTURE0);
        glUniform1i(shader->getUniform("SBTex"), 0);
        glBindTexture(GL_TEXTURE_2D, batch->textureID);
        ss->setObject(0);

        glDrawArrays(GL_TRIANGLES, batch->indexOffset, batch->indices);
    }

    glBindVertexArray(0);
    glBindSampler(0, 0);

    shader->unuse();
}
void SpriteBatch::renderBatch(f32m4 mWorld, const f32v2& screenSize, /*const BlendState* bs = nullptr,*/ const SamplerState* ss /*= nullptr*/, const DepthState* ds /*= nullptr*/, const RasterizerState* rs /*= nullptr*/, vg::GLProgram* shader /*= nullptr*/) {
    f32m4 mCamera(
        2.0f / screenSize.x, 0, 0, 0,
        0, -2.0f / screenSize.y, 0, 0,
        0, 0, 1, 0,
        -1, 1, 0, 1
        );
    renderBatch(mWorld, mCamera, /*bs, */ ss, ds, rs, shader);
}
void SpriteBatch::renderBatch(const f32v2& screenSize, /*const BlendState* bs = nullptr,*/ const SamplerState* ss /*= nullptr*/, const DepthState* ds /*= nullptr*/, const RasterizerState* rs /*= nullptr*/, vg::GLProgram* shader /*= nullptr*/) {
    f32m4 mIdentity(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
        );
    renderBatch(mIdentity, screenSize, /*bs, */ ss, ds, rs, shader);
}

void SpriteBatch::sortGlyphs(SpriteSortMode ssm) {
    if (_glyphs.size() < 1) return;

    switch (ssm) {
    case SpriteSortMode::TEXTURE:
        std::stable_sort(_glyphs.begin(), _glyphs.end(), SSMTexture);
        break;
    case SpriteSortMode::FRONT_TO_BACK:
        std::stable_sort(_glyphs.begin(), _glyphs.end(), SSMFrontToBack);
        break;
    case SpriteSortMode::BACK_TO_FRONT:
        std::stable_sort(_glyphs.begin(), _glyphs.end(), SSMBackToFront);
        break;
    default:
        break;
    }
}
void SpriteBatch::generateBatches() {
    if (_glyphs.size() < 1) return;

    // Create Arrays
    VertexSpriteBatch* verts = new VertexSpriteBatch[6 * _glyphs.size()];
    int vi = 0;

    SpriteBatchCall* call = _batchRecycler.create();
    call->textureID = _glyphs[0]->textureID;
    call->indexOffset = 0;
    call->indices = 6;
    _batches.push_back(call);
    verts[vi++] = _glyphs[0]->vtl;
    verts[vi++] = _glyphs[0]->vbl;
    verts[vi++] = _glyphs[0]->vbr;
    verts[vi++] = _glyphs[0]->vbr;
    verts[vi++] = _glyphs[0]->vtr;
    verts[vi++] = _glyphs[0]->vtl;
    _glyphRecycler.recycle(_glyphs[0]);

    int gc = _glyphs.size();
    for (int i = 1; i < gc; i++) {
        SpriteGlyph* glyph = _glyphs[i];
        call = call->append(glyph, _batches, &_batchRecycler);
        verts[vi++] = glyph->vtl;
        verts[vi++] = glyph->vbl;
        verts[vi++] = glyph->vbr;
        verts[vi++] = glyph->vbr;
        verts[vi++] = glyph->vtr;
        verts[vi++] = glyph->vtl;
        _glyphRecycler.recycle(_glyphs[i]);
    }
    _glyphs.swap(std::vector<SpriteGlyph*>());

    // Set The Buffer Data
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    if (gc > _glyphCapacity) {
        _glyphCapacity = gc * 2;
        glBufferData(GL_ARRAY_BUFFER, _glyphCapacity * 6 * sizeof(VertexSpriteBatch), nullptr, _bufUsage);
    }
    glBufferSubData(GL_ARRAY_BUFFER, 0, gc * 6 * sizeof(VertexSpriteBatch), verts);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    delete[] verts;
}

void SpriteBatch::createProgram() {
    // Only need to create it if it isn't cached
    if (!_program) {

        // Allocate the program
        _program = new vg::GLProgram(true);

        // Create the vertex shader
        _program->addShader(vg::ShaderType::VERTEX_SHADER, VS_SRC);

        // Create the fragment shader
        _program->addShader(vg::ShaderType::FRAGMENT_SHADER, FS_SRC);

        // Set the attributes
        std::vector <nString> attributes;
        attributes.push_back("vPosition");
        attributes.push_back("vTint");
        attributes.push_back("vUV");
        attributes.push_back("vUVRect");
        _program->setAttributes(attributes);
        
        // Link the program
        _program->link();

        // Init the uniforms
        _program->initUniforms();

        // Init the attributes
        _program->initAttributes();
    }
}

void SpriteBatch::createVertexArray() {
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    _glyphCapacity = _INITIAL_GLYPH_CAPACITY;
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, _glyphCapacity * 6 * sizeof(VertexSpriteBatch), nullptr, _bufUsage);

    _program->enableVertexAttribArrays();

    glVertexAttribPointer(_program->getAttribute("vPosition"), 3, GL_FLOAT, false, sizeof(VertexSpriteBatch), (void*)offsetof(VertexSpriteBatch, position));
    glVertexAttribPointer(_program->getAttribute("vTint"), 4, GL_UNSIGNED_BYTE, true, sizeof(VertexSpriteBatch), (void*)offsetof(VertexSpriteBatch, color));
    glVertexAttribPointer(_program->getAttribute("vUV"), 2, GL_FLOAT, false, sizeof(VertexSpriteBatch), (void*)offsetof(VertexSpriteBatch, uv));
    glVertexAttribPointer(_program->getAttribute("vUVRect"), 4, GL_FLOAT, false, sizeof(VertexSpriteBatch), (void*)offsetof(VertexSpriteBatch, uvRect));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void SpriteBatch::createPixelTexture() {
    glGenTextures(1, &_texPixel);
    glBindTexture(GL_TEXTURE_2D, _texPixel);
    ui32 pix = 0xffffffffu;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pix);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void SpriteBatch::disposeProgram() {
    if (_program) {
        _program->destroy();
        _program = nullptr;
    }
}

vg::GLProgram* SpriteBatch::_program = nullptr;

void SpriteBatch::SpriteBatchCall::set(i32 iOff, ui32 texID, std::vector<SpriteBatchCall*>& calls) {
    textureID = texID;
    indices = 6;
    indexOffset = iOff;
    calls.push_back(this);
}
SpriteBatch::SpriteBatchCall* SpriteBatch::SpriteBatchCall::append(SpriteGlyph* g, std::vector<SpriteBatchCall*>& calls, PtrRecycler<SpriteBatchCall>* recycler) {
    if (g->textureID != textureID) {
        SpriteBatchCall* sbc = recycler->create();
        sbc->set(indexOffset + indices, g->textureID, calls);
        return sbc;
    } else {
        indices += 6;
    }
    return this;
}

