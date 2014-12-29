#include "stdafx.h"
#include "TestDeferredScreen.h"

#include <glm/gtc/matrix_transform.hpp>
#include <Vorb/colors.h>
#include <Vorb/GLStates.h>
#include <Vorb/GLRenderTarget.h>
#include <Vorb/ImageLoader.h>
#include <Vorb/InputDispatcher.h>
#include <Vorb/IOManager.h>

// Number cells per row/column in a single grid
const ui32 CELLS = 20;

DepthState dsLightPoint(true, DepthFunction::GREATER_EQUAL, false);
DepthState dsLightDirectional(true, DepthFunction::NOT_EQUAL, false);

struct DefVertex {
public:
    f32v3 position;
    f32v3 normal;
};

TestDeferredScreen::TestDeferredScreen() :
m_sb(true, false) {
    // Empty
}

i32 TestDeferredScreen::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}
i32 TestDeferredScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void TestDeferredScreen::build() {
    // Empty
}
void TestDeferredScreen::destroy(const GameTime& gameTime) {
    // Empty
}

void TestDeferredScreen::onEntry(const GameTime& gameTime) {
    buildGeometry();
    buildLightMaps();

    m_gbuffer = vg::GBuffer(_game->getWindow().getWidth(), _game->getWindow().getHeight());
    m_gbuffer.init().initDepthStencil();

    m_sb.init();

    IOManager iom;
    const cString src;

    m_deferredPrograms.clear.init();
    src = iom.readFileToString("Shaders/Deferred/Clear.vert");
    m_deferredPrograms.clear.addShader(vg::ShaderType::VERTEX_SHADER, src);
    delete[] src;
    src = iom.readFileToString("Shaders/Deferred/Clear.frag");
    m_deferredPrograms.clear.addShader(vg::ShaderType::FRAGMENT_SHADER, src);
    delete[] src;
    m_deferredPrograms.clear.setAttribute("vPosition", 0);
    m_deferredPrograms.clear.link();
    m_deferredPrograms.clear.initAttributes();
    m_deferredPrograms.clear.initUniforms();

    m_deferredPrograms.composition.init();
    src = iom.readFileToString("Shaders/Deferred/Composition.vert");
    m_deferredPrograms.composition.addShader(vg::ShaderType::VERTEX_SHADER, src);
    delete[] src;
    src = iom.readFileToString("Shaders/Deferred/Composition.frag");
    m_deferredPrograms.composition.addShader(vg::ShaderType::FRAGMENT_SHADER, src);
    delete[] src;
    m_deferredPrograms.composition.setAttribute("vPosition", 0);
    m_deferredPrograms.composition.link();
    m_deferredPrograms.composition.initAttributes();
    m_deferredPrograms.composition.initUniforms();

    {
        m_deferredPrograms.geometry["Basic"] = vg::GLProgram(false);
        vg::GLProgram& p = m_deferredPrograms.geometry["Basic"];
        p.init();
        src = iom.readFileToString("Shaders/Deferred/Geometry.vert");
        p.addShader(vg::ShaderType::VERTEX_SHADER, src);
        delete[] src;
        src = iom.readFileToString("Shaders/Deferred/Geometry.frag");
        p.addShader(vg::ShaderType::FRAGMENT_SHADER, src);
        delete[] src;
        p.link();
        p.initAttributes();
        p.initUniforms();
    }
    {
        m_deferredPrograms.light["Directional"] = vg::GLProgram(false);
        vg::GLProgram& p = m_deferredPrograms.light["Directional"];
        p.init();
        src = iom.readFileToString("Shaders/Deferred/LightDirectional.vert");
        p.addShader(vg::ShaderType::VERTEX_SHADER, src);
        delete[] src;
        vg::ShaderSource srcFrag;
        srcFrag.stage = vg::ShaderType::FRAGMENT_SHADER;
        srcFrag.sources.push_back(iom.readFileToString("Shaders/Deferred/LightModels.glsl"));
        srcFrag.sources.push_back(iom.readFileToString("Shaders/Deferred/LightDirectional.frag"));
        p.addShader(srcFrag);
        delete[] srcFrag.sources[0];
        delete[] srcFrag.sources[1];
        p.link();
        p.initAttributes();
        p.initUniforms();
    }
    m_quad.init(0);

    m_hooks.addAutoHook(&vui::InputDispatcher::key.onKeyDown, [&] (void* s, const vui::KeyEvent& e) {
        switch (e.keyCode) {
        case VKEY_I: if(m_roughness < 0.96) m_roughness += 0.05f; break;
        case VKEY_J: if (m_roughness > 0.04) m_roughness -= 0.05f; break;
        case VKEY_O: if (m_reflectance < 0.91) m_reflectance += 0.1f; break;
        case VKEY_K: if (m_reflectance > 0.09) m_reflectance -= 0.1f; break;
        case VKEY_P: if (m_metalness < 0.5) m_metalness = 1; break;
        case VKEY_L: if (m_metalness > 0.5) m_metalness = 0; break;
        default:
            break;
        }
    });
    m_reflectance = 0.0f;
    m_roughness = 1.0f;
    m_metalness = 1.0f;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0);
}
void TestDeferredScreen::onExit(const GameTime& gameTime) {
    glDeleteBuffers(1, &m_verts);
    glDeleteBuffers(1, &m_inds);
    m_deferredPrograms.dispose();
    m_sb.dispose();
    m_quad.dispose();
    m_gbuffer.dispose();
}

void TestDeferredScreen::onEvent(const SDL_Event& e) {
    // Empty
}
void TestDeferredScreen::update(const GameTime& gameTime) {
    // Empty
}
void TestDeferredScreen::draw(const GameTime& gameTime) {
    /************************************************************************/
    /* Deferred pass                                                        */
    /************************************************************************/
    m_gbuffer.useGeometry();

    // Clear the GBuffer
    glBlendFunc(GL_ONE, GL_ZERO);
    DepthState::WRITE.set();
    m_deferredPrograms.clear.use();
    m_quad.draw();
    
    f32v3 eyePos(0, 0, 4);
    f32m4 mVP = glm::perspectiveFov(90.0f, 800.0f, 600.0f, 0.1f, 1000.0f) * glm::lookAt(eyePos, f32v3(0, 0, 0), f32v3(0, 1, 0));
    f32m4 mVPInv = glm::inverse(mVP);

    DepthState::FULL.set();
    RasterizerState::CULL_CLOCKWISE.set();

    vg::GLProgram& progGeo = m_deferredPrograms.geometry["Basic"];
    progGeo.use();
    progGeo.enableVertexAttribArrays();

    glUniformMatrix4fv(progGeo.getUniform("unVP"), 1, false, (f32*)&mVP[0][0]);

    glBindBuffer(GL_ARRAY_BUFFER, m_verts);
    glVertexAttribPointer(progGeo.getAttribute("vPosition"), 3, GL_FLOAT, false, sizeof(DefVertex), offsetptr(DefVertex, position));
    glVertexAttribPointer(progGeo.getAttribute("vNormal"), 3, GL_FLOAT, false, sizeof(DefVertex), offsetptr(DefVertex, normal));
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_inds);

    f32m4 mW = glm::translate(f32m4(1.0f), f32v3(-1.3f, -1, 0));
    f32m3 mWIT = f32m3(glm::transpose(glm::inverse(mW)));
    glUniformMatrix4fv(progGeo.getUniform("unWorld"), 1, false, (f32*)&mW[0][0]);
    glUniformMatrix3fv(progGeo.getUniform("unWorldIT"), 1, false, (f32*)&mWIT[0][0]);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);

    mW = glm::translate(f32m4(1.0f), f32v3(1.3f, 1, 0));
    mWIT = f32m3(glm::transpose(glm::inverse(mW)));
    glUniformMatrix4fv(progGeo.getUniform("unWorld"), 1, false, (f32*)&mW[0][0]);
    glUniformMatrix3fv(progGeo.getUniform("unWorldIT"), 1, false, (f32*)&mWIT[0][0]);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);

    mW = glm::translate(f32m4(1.0f), f32v3(0, 0, -2));
    mWIT = f32m3(glm::transpose(glm::inverse(mW)));
    glUniformMatrix4fv(progGeo.getUniform("unWorld"), 1, false, (f32*)&mW[0][0]);
    glUniformMatrix3fv(progGeo.getUniform("unWorldIT"), 1, false, (f32*)&mWIT[0][0]);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    progGeo.disableVertexAttribArrays();

    /************************************************************************/
    /* Lighting pass                                                        */
    /************************************************************************/
    m_gbuffer.useLight();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBlendFunc(GL_ONE, GL_ONE);

    {
        dsLightDirectional.set();

        vg::GLProgram& progLight = m_deferredPrograms.light["Directional"];
        progLight.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_gbuffer.getTextureIDs().normal);
        glUniform1i(progLight.getUniform("unTexNormal"), 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_gbuffer.getTextureIDs().depth);
        glUniform1i(progLight.getUniform("unTexDepth"), 1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_envMap);
        glUniform1i(progLight.getUniform("unTexEnvironment"), 2);
        glUniformMatrix4fv(progLight.getUniform("unVPInv"), 1, false, (f32*)&mVPInv[0][0]);
        glUniform3f(progLight.getUniform("unEyePosition"), eyePos.x, eyePos.y, eyePos.z);
        glUniform1f(progLight.getUniform("unRoughness"), m_roughness);
        glUniform1f(progLight.getUniform("unReflectance"), m_reflectance * 0.96f + 0.04f);
        glUniform1f(progLight.getUniform("unMetalness"), m_metalness);

        //const size_t NUM_LIGHTS = 3;
        //f32v3 lightDirs[NUM_LIGHTS] = {
        //    f32v3(0, 0, -1),
        //    f32v3(2, -1, 0),
        //    f32v3(-1, 1, -1)
        //    //f32v3(-4, -3, 0),
        //    //f32v3(6, 3, 2)
        //};
        //f32v3 lightColors[NUM_LIGHTS] = {
        //    f32v3(0.6, 0.6, 0.3),
        //    f32v3(1.0, 0.0, 0.0),
        //    f32v3(0.0, 1.0, 0.0)
        //    //f32v3(0.0, 1.0, 1.0),
        //    //f32v3(1.0, 0.0, 1.0)
        //};
        //for (size_t i = 0; i < NUM_LIGHTS; i++) {
        //    f32v3 lightDir = glm::normalize(lightDirs[i]);
        //    glUniform3f(progLight.getUniform("unLightDirection"), lightDir.x, lightDir.y, lightDir.z);
        //    f32v3 lightColor = lightColors[i];
        //    glUniform3f(progLight.getUniform("unLightIntensity"), lightColor.x, lightColor.y, lightColor.z);
        //}
        m_quad.draw();
    }
    {
        dsLightPoint.set();

        /*vg::GLProgram& progLight = m_deferredPrograms.light["Point"];
        progLight.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_gbuffer.getTextureIDs().normal);
        glUniform1i(progLight.getUniform("unTexNormal"), 0);

        const size_t NUM_LIGHTS = 5;
        f32v3 lightDirs[NUM_LIGHTS] = {
        f32v3(0, -2, -1),
        f32v3(2, -1, 0),
        f32v3(-1, 1, -1),
        f32v3(-4, -3, 0),
        f32v3(6, 3, 2)
        };
        f32v3 lightColors[NUM_LIGHTS] = {
        f32v3(0.6, 0.6, 0.3),
        f32v3(1.0, 0.0, 0.0),
        f32v3(0.0, 1.0, 0.0),
        f32v3(0.0, 1.0, 1.0),
        f32v3(1.0, 0.0, 1.0)
        };
        for (size_t i = 0; i < NUM_LIGHTS; i++) {
        f32v3 lightDir = glm::normalize(lightDirs[i]);
        glUniform3f(progLight.getUniform("unLightDirection"), lightDir.x, lightDir.y, lightDir.z);
        f32v3 lightColor = lightColors[i];
        glUniform3f(progLight.getUniform("unLightColor"), lightColor.x, lightColor.y, lightColor.z);
        m_quad.draw();
        }*/
    }



    /************************************************************************/
    /* Compose deferred and lighting                                        */
    /************************************************************************/
    vg::GLRenderTarget::unuse(_game->getWindow().getWidth(), _game->getWindow().getHeight());
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBlendFunc(GL_ONE, GL_ZERO);
    DepthState::WRITE.set();
    m_deferredPrograms.composition.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_gbuffer.getTextureIDs().color);
    glUniform1i(m_deferredPrograms.composition.getUniform("unTexColor"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_gbuffer.getTextureIDs().depth);
    glUniform1i(m_deferredPrograms.composition.getUniform("unTexDepth"), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_gbuffer.getTextureIDs().light);
    glUniform1i(m_deferredPrograms.composition.getUniform("unTexLight"), 2);
    m_quad.draw();

    /************************************************************************/
    /* Debug                                                                */
    /************************************************************************/
    m_sb.begin();
    m_sb.draw(m_gbuffer.getTextureIDs().color, f32v2(0, 0), f32v2(200, 150), color::White);
    m_sb.draw(m_gbuffer.getTextureIDs().normal, f32v2(200, 0), f32v2(200, 150), color::White);
    m_sb.draw(m_gbuffer.getTextureIDs().depth, f32v2(400, 0), f32v2(200, 150), color::White);
    m_sb.draw(m_gbuffer.getTextureIDs().light, f32v2(600, 0), f32v2(200, 150), color::White);
    m_sb.end();
    m_sb.renderBatch(f32v2(_game->getWindow().getWidth(), _game->getWindow().getHeight()));
}

void TestDeferredScreen::buildGeometry() {
    // Make vertices
    ui32 cellPoints = CELLS + 1;
    ui32 gridPoints = cellPoints * cellPoints;
    DefVertex* verts = new DefVertex[gridPoints * 2];
    size_t i = 0;
    for (size_t y = 0; y < cellPoints; y++) {
        for (size_t x = 0; x < cellPoints; x++) {
            f32v3 pos((f32)x / (f32)CELLS, 0, (f32)y / (f32)CELLS);
            pos.x -= 0.5f;
            pos.x *= 2.0f;
            pos.z -= 0.5f;
            pos.z *= 2.0f;
            pos.x = glm::sign(pos.x) * (sin(abs(pos.x) * 1.57079f));
            pos.z = glm::sign(pos.z) * (sin(abs(pos.z) * 1.57079f));

            f32 lp = glm::length(pos);
            if (lp < 0.00001) {
                pos.y = 1.0f;
            } else {
                f32 d = std::max(abs(pos.x), abs(pos.z));
                pos /= lp;
                pos *= d;
                pos.y = sin(acos(d));
            }

            verts[i].position = pos;
            verts[i].normal = glm::normalize(verts[i].position);
            pos.y = -pos.y;
            verts[gridPoints + i].position = pos;
            verts[gridPoints + i].normal = glm::normalize(verts[gridPoints + i].position);
            i++;
        }
    }

    // Make indices
    ui32 gridInds = 6 * CELLS * CELLS;
    m_indexCount = gridInds * 2;
    ui32* inds = new ui32[m_indexCount];
    i = 0;
    for (size_t y = 0; y < CELLS; y++) {
        for (size_t x = 0; x < CELLS; x++) {
            ui32 vi = y * cellPoints + x;
            inds[i] = vi;
            inds[i + 1] = vi + cellPoints;
            inds[i + 2] = vi + 1;
            inds[i + 3] = vi + 1;
            inds[i + 4] = vi + cellPoints;
            inds[i + 5] = vi + cellPoints + 1;

            vi += gridPoints;
            inds[gridInds + i] = vi;
            inds[gridInds + i + 1] = vi + 1;
            inds[gridInds + i + 2] = vi + cellPoints;
            inds[gridInds + i + 3] = vi + cellPoints;
            inds[gridInds + i + 4] = vi + 1;
            inds[gridInds + i + 5] = vi + cellPoints + 1;

            i += 6;
        }
    }

    // Fill buffers
    glGenBuffers(1, &m_verts);
    glBindBuffer(GL_ARRAY_BUFFER, m_verts);
    glBufferData(GL_ARRAY_BUFFER, gridPoints * 2 * sizeof(DefVertex), verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    delete[] verts;
    glGenBuffers(1, &m_inds);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_inds);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexCount * sizeof(ui32), inds, GL_STATIC_DRAW);
    delete[] inds;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void TestDeferredScreen::buildLightMaps() {
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glGenTextures(1, &m_envMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_envMap);

    std::vector<ui8> pixels;
    ui32v2 size;
    
    vg::ImageLoader::loadPng("Textures/Test/nx.png", pixels, size.x, size.y, false);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB16F, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    pixels.swap(std::vector<ui8>());
    vg::ImageLoader::loadPng("Textures/Test/px.png", pixels, size.x, size.y, false);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB16F, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    pixels.swap(std::vector<ui8>());
    vg::ImageLoader::loadPng("Textures/Test/ny.png", pixels, size.x, size.y, false);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB16F, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    pixels.swap(std::vector<ui8>());
    vg::ImageLoader::loadPng("Textures/Test/py.png", pixels, size.x, size.y, false);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB16F, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    pixels.swap(std::vector<ui8>());
    vg::ImageLoader::loadPng("Textures/Test/nz.png", pixels, size.x, size.y, false);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB16F, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    pixels.swap(std::vector<ui8>());
    vg::ImageLoader::loadPng("Textures/Test/pz.png", pixels, size.x, size.y, false);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB16F, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    pixels.swap(std::vector<ui8>());

    SamplerState ss(
        (VGEnum)vg::TextureMinFilter::LINEAR_MIPMAP_LINEAR,
        (VGEnum)vg::TextureMagFilter::LINEAR,
        (VGEnum)vg::TextureWrapMode::REPEAT,
        (VGEnum)vg::TextureWrapMode::REPEAT,
        (VGEnum)vg::TextureWrapMode::REPEAT
        );
    ss.set(GL_TEXTURE_CUBE_MAP);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}