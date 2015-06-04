#include "stdafx.h"
#include "TestDeferredScreen.h"

#include "ShaderLoader.h"
#include <Vorb/colors.h>
#include <Vorb/graphics/GLStates.h>
#include <Vorb/graphics/GLRenderTarget.h>
#include <Vorb/graphics/ImageIO.h>
#include <Vorb/ui/InputDispatcher.h>
#include <Vorb/io/IOManager.h>

// Number cells per row/column in a single grid
const ui32 CELLS = 20;

vg::DepthState dsLightPoint(true, vg::DepthFunction::GREATER, false);
vg::DepthState dsLightDirectional(true, vg::DepthFunction::NOTEQUAL, false);

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
void TestDeferredScreen::destroy(const vui::GameTime& gameTime) {
    // Empty
}

void TestDeferredScreen::onEntry(const vui::GameTime& gameTime) {
    m_eyePos = glm::vec3(0, 0, 4);
    buildGeometry();
    buildLightMaps();

    m_gbuffer = vg::GBuffer(m_game->getWindow().getWidth(), m_game->getWindow().getHeight());
    Array<vg::GBufferAttachment> ga;
    ga.setData(4);
    ga[0].format = vg::TextureInternalFormat::RGBA16F;
    ga[0].pixelFormat = vg::TextureFormat::RGBA;
    ga[0].pixelType = vg::TexturePixelType::HALF_FLOAT;
    ga[0].number = 0;
    ga[1] = ga[0];
    ga[1].number = 1;
    ga[2].format = vg::TextureInternalFormat::RG32F;
    ga[2].pixelFormat = vg::TextureFormat::RG;
    ga[2].pixelType = vg::TexturePixelType::FLOAT;
    ga[2].number = 2;
    m_gbuffer.init(ga, vg::TextureInternalFormat::RGB16F).initDepthStencil();

    m_sb.init();

    { // Init Shaders
        m_deferredPrograms.clear = ShaderLoader::createProgramFromFile("Shaders/Deferred/Clear.vert",
                                                                       "Shaders/Deferred/Clear.frag");

        m_deferredPrograms.composition = ShaderLoader::createProgramFromFile("Shaders/Deferred/Composition.vert",
                                                                             "Shaders/Deferred/Composition.frag");

        m_deferredPrograms.geometry["Basic"] = ShaderLoader::createProgramFromFile("Shaders/Deferred/Geometry.vert",
                                                                                   "Shaders/Deferred/Geometry.frag");

        m_deferredPrograms.light["Directional"] = ShaderLoader::createProgramFromFile("Shaders/Deferred/LightDirectional.vert",
                                                                                      "Shaders/Deferred/LightDirectional.frag");
    }

    m_quad.init(0);

    m_hooks.addAutoHook(vui::InputDispatcher::key.onKeyDown, [&] (Sender s, const vui::KeyEvent& e) {
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
void TestDeferredScreen::onExit(const vui::GameTime& gameTime) {
    glDeleteBuffers(1, &m_verts);
    glDeleteBuffers(1, &m_inds);
    m_deferredPrograms.dispose();
    m_sb.dispose();
    m_quad.dispose();
    m_gbuffer.dispose();
}

void TestDeferredScreen::update(const vui::GameTime& gameTime) {
    // Empty
    glm::vec4 rotated = glm::vec4(m_eyePos, 1) * glm::rotate(glm::mat4(), (float)(10 * gameTime.elapsed), glm::vec3(0, 1, 0));
    m_eyePos.x = rotated.x;
    m_eyePos.y = rotated.y;
    m_eyePos.z = rotated.z;
}

void TestDeferredScreen::draw(const vui::GameTime& gameTime) {
    /************************************************************************/
    /* Deferred pass                                                        */
    /************************************************************************/
    m_gbuffer.useGeometry();

    // Clear the GBuffer
    glBlendFunc(GL_ONE, GL_ZERO);
    vg::DepthState::WRITE.set();
    m_deferredPrograms.clear.use();
    m_quad.draw();
    
    f32m4 mVP = glm::perspectiveFov(90.0f, 800.0f, 600.0f, 0.1f, 1000.0f) * glm::lookAt(m_eyePos, f32v3(0, 0, 0), f32v3(0, 1, 0));
    f32m4 mVPInv = glm::inverse(mVP);

    vg::DepthState::FULL.set();
    vg::RasterizerState::CULL_CLOCKWISE.set();

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
        glBindTexture(GL_TEXTURE_2D, m_gbuffer.getGeometryTexture(1));
        glUniform1i(progLight.getUniform("unTexNormal"), 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_gbuffer.getGeometryTexture(2));
        glUniform1i(progLight.getUniform("unTexDepth"), 1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_envMap);
        glUniform1i(progLight.getUniform("unTexEnvironment"), 2);
        glUniformMatrix4fv(progLight.getUniform("unVPInv"), 1, false, (f32*)&mVPInv[0][0]);
        glUniform3f(progLight.getUniform("unEyePosition"), m_eyePos.x, m_eyePos.y, m_eyePos.z);
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
        glBindTexture(GL_TEXTURE_2D, m_gbuffer.getGeometryTexture(1));
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
    vg::GLRenderTarget::unuse(m_game->getWindow().getWidth(), m_game->getWindow().getHeight());
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBlendFunc(GL_ONE, GL_ZERO);
    vg::DepthState::WRITE.set();
    m_deferredPrograms.composition.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_gbuffer.getGeometryTexture(0));
    glUniform1i(m_deferredPrograms.composition.getUniform("unTexColor"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_gbuffer.getGeometryTexture(2));
    glUniform1i(m_deferredPrograms.composition.getUniform("unTexDepth"), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_gbuffer.getLightTexture());
    glUniform1i(m_deferredPrograms.composition.getUniform("unTexLight"), 2);
    m_quad.draw();

    /************************************************************************/
    /* Debug                                                                */
    /************************************************************************/
    m_sb.begin();
    m_sb.draw(m_gbuffer.getGeometryTexture(0), f32v2(0, 0), f32v2(200, 150), color::White);
    m_sb.draw(m_gbuffer.getGeometryTexture(1), f32v2(200, 0), f32v2(200, 150), color::White);
    m_sb.draw(m_gbuffer.getGeometryTexture(2), f32v2(400, 0), f32v2(200, 150), color::White);
    m_sb.draw(m_gbuffer.getLightTexture(), f32v2(600, 0), f32v2(200, 150), color::White);
    m_sb.end();
    m_sb.render(f32v2(m_game->getWindow().getWidth(), m_game->getWindow().getHeight()));
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
    
    vg::ImageIO imageLoader;
    vg::ScopedBitmapResource rs0 = imageLoader.load("Textures/Test/nx.png");
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB16F, rs0.width, rs0.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rs0.data);
    vg::ScopedBitmapResource rs1 = imageLoader.load("Textures/Test/px.png");
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB16F, rs1.width, rs1.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rs1.data);
    vg::ScopedBitmapResource rs2 = imageLoader.load("Textures/Test/ny.png");
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB16F, rs2.width, rs2.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rs2.data);
    vg::ScopedBitmapResource rs3 = imageLoader.load("Textures/Test/py.png");
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB16F, rs3.width, rs3.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rs3.data);
    vg::ScopedBitmapResource rs4 = imageLoader.load("Textures/Test/nz.png");
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB16F, rs4.width, rs4.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rs4.data);
    vg::ScopedBitmapResource rs5 = imageLoader.load("Textures/Test/pz.png");
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB16F, rs5.width, rs5.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rs5.data);

    vg::SamplerState ss(
        (VGEnum)vg::TextureMinFilter::LINEAR_MIPMAP_LINEAR,
        (VGEnum)vg::TextureMagFilter::LINEAR,
        (VGEnum)vg::TextureWrapMode::REPEAT,
        (VGEnum)vg::TextureWrapMode::REPEAT,
        (VGEnum)vg::TextureWrapMode::REPEAT
        );
    ss.set(GL_TEXTURE_CUBE_MAP);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}