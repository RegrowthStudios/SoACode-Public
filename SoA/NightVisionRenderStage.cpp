#include "stdafx.h"
#include "NightVisionRenderStage.h"

#include <ctime>

#include <Vorb/colors.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/Random.h>
#include <Vorb/graphics/SamplerState.h>
#include "ShaderLoader.h"

#include "SoaOptions.h"

KEG_TYPE_DEF_SAME_NAME(NightVisionRenderParams, kt) {
    using namespace keg;
    kt.addValue("Color", Value::basic(offsetof(NightVisionRenderParams, color), BasicType::F32_V3));
    kt.addValue("Contrast", Value::basic(offsetof(NightVisionRenderParams, luminanceExponent), BasicType::F32));
    kt.addValue("Filter", Value::basic(offsetof(NightVisionRenderParams, luminanceTare), BasicType::F32));
    kt.addValue("Brightness", Value::basic(offsetof(NightVisionRenderParams, colorAmplification), BasicType::F32));
    kt.addValue("Noise", Value::basic(offsetof(NightVisionRenderParams, noisePower), BasicType::F32));
    kt.addValue("ColorNoise", Value::basic(offsetof(NightVisionRenderParams, noiseColor), BasicType::F32));
}

NightVisionRenderParams NightVisionRenderParams::createDefault() {
    NightVisionRenderParams v = {};
    v.colorAmplification = NIGHT_VISION_DEFAULT_COLOR_AMPLIFICATION;
    v.luminanceExponent = NIGHT_VISION_DEFAULT_LUMINANCE_EXPONENT;
    v.luminanceTare = NIGHT_VISION_DEFAULT_LUMINANCE_TARE;
    v.noisePower = NIGHT_VISION_DEFAULT_NOISE_POWER;
    v.noiseColor = NIGHT_VISION_DEFAULT_NOISE_COLOR;
    v.color = NIGHT_VISION_DEFAULT_VISION_COLOR;
    return v;
}

NightVisionRenderStage::NightVisionRenderStage() {
    // Empty
}

void NightVisionRenderStage::init(vg::FullQuadVBO* quad) {
    m_quad = quad;
    m_texNoise.width = NIGHT_VISION_NOISE_QUALITY;
    m_texNoise.height = NIGHT_VISION_NOISE_QUALITY;

    // Generate random data
    i32 pixCount = m_texNoise.width * m_texNoise.height;
    ui8* data = new ui8[pixCount];
    Random r(clock());
    for (i32 i = 0; i < pixCount; i++) {
        data[i] = (ui8)(r.genMT() * 255.0f);
    }

    // Build noise texture
    glGenTextures(1, &m_texNoise.id);
    glBindTexture(GL_TEXTURE_2D, m_texNoise.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_texNoise.width, m_texNoise.height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    vg::SamplerState::POINT_WRAP.set(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    delete[] data;
}

void NightVisionRenderStage::setParams(NightVisionRenderParams& params) {
    m_program.use();
    glUniform1f(m_program.getUniform("unLuminanceExponent"), params.luminanceExponent);
    glUniform1f(m_program.getUniform("unLuminanceTare"), params.luminanceTare);
    glUniform1f(m_program.getUniform("unNoisePower"), params.noisePower);
    glUniform1f(m_program.getUniform("unNoiseColor"), params.noiseColor);
    glUniform1f(m_program.getUniform("unColorAmplification"), params.colorAmplification);
    glUniform3f(m_program.getUniform("unVisionColor"), params.color.r, params.color.g, params.color.b);
}

void NightVisionRenderStage::reloadShader() {
    IRenderStage::reloadShader();
    if (m_texNoise.id) {
        glDeleteTextures(1, &m_texNoise.id);
        m_texNoise.id = 0;
    }
}

void NightVisionRenderStage::dispose() {
    IRenderStage::dispose();
    if (m_texNoise.id) {
        glDeleteTextures(1, &m_texNoise.id);
        m_texNoise.id = 0;
    }
}

void NightVisionRenderStage::render() {
    m_et += NIGHT_VISION_DEFAULT_NOISE_TIME_STEP;

    //_visionColorHSL.r = fmod(_visionColorHSL.r = 0.005f, 6.28f);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_texNoise.id);

    if (!m_program.isCreated()) {
        m_program = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert",
                                                             "Shaders/PostProcessing/NightVision.frag");
        m_program.use();
        glUniform1i(m_program.getUniform("unTexColor"), NIGHT_VISION_TEXTURE_SLOT_COLOR);
        glUniform1i(m_program.getUniform("unTexNoise"), NIGHT_VISION_TEXTURE_SLOT_NOISE);
        setParams(NightVisionRenderParams::createDefault());
    } else {
        m_program.use();
    }
    m_program.enableVertexAttribArrays();

    glUniform1f(m_program.getUniform("unTime"), m_et);

    glDisable(GL_DEPTH_TEST);
    m_quad->draw();
    glEnable(GL_DEPTH_TEST);

    m_program.disableVertexAttribArrays();
    vg::GLProgram::unuse();
}
