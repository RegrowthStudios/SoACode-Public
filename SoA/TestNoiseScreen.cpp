#include "stdafx.h"
#include "TestNoiseScreen.h"

#include <Vorb/ui/InputDispatcher.h>

#include "Errors.h"
#include "ShaderLoader.h"

#define MS_AVARAGE_FRAMES 32

std::chrono::system_clock::time_point lastTime;
double delta;

double ms = 0;
unsigned int frameCount = 0;

i32 TestNoiseScreen::getNextScreen() const
{
    return SCREEN_INDEX_NO_SCREEN;
}

i32 TestNoiseScreen::getPreviousScreen() const
{
    return SCREEN_INDEX_NO_SCREEN;
}

void TestNoiseScreen::build()
{

}
void TestNoiseScreen::destroy(const vui::GameTime& gameTime)
{

}

void TestNoiseScreen::onEntry(const vui::GameTime& gameTime)
{
    lastTime = std::chrono::high_resolution_clock::now();
    
    m_noiseTypes.emplace_back();
    m_noiseTypes.back().vertexShader = R"(
    out vec2 fPosition;

    const vec2 vertices[4] = vec2[](vec2(-1.0, 1.0), vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(1.0, 1.0));

    void main()
    {
        vec2 vertex = vertices[gl_VertexID];
        fPosition = vertex;
        gl_Position = vec4(vertex, 0.0, 1.0);
    }
    )";

    m_noiseTypes.back().fragmentShader = R"(
    uniform float unTime;
    uniform float unAspectRatio;

    in vec2 fPosition;

    out vec4 pColor;

    #include "Shaders/Noise/snoise3.glsl"

    void main()
    {
        vec3 noisePosition = vec3(fPosition * 4.0, unTime);
        noisePosition.x *= unAspectRatio;
        pColor = vec4(snoise(noisePosition) * 0.5 + 0.5);
    }
    )";
    
    m_currentNoise = 0;
    onNoiseChange();

    m_hooks.addAutoHook(vui::InputDispatcher::key.onKeyDown, [&](Sender s, const vui::KeyEvent& e) {
        if (e.keyCode == VKEY_LEFT) m_currentNoise--;
        if (e.keyCode == VKEY_RIGHT) m_currentNoise++;
        if (m_currentNoise < 0) m_currentNoise = m_noiseTypes.size() - 1;
        if (m_currentNoise >= m_noiseTypes.size()) m_currentNoise = 0;
        onNoiseChange();
    });
}

void TestNoiseScreen::onExit(const vui::GameTime& gameTime)
{
    delete m_program;
}

void TestNoiseScreen::update(const vui::GameTime& gameTime)
{
    std::chrono::system_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
    delta = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - lastTime).count() / 1000000.0;
    ms += delta;
    lastTime = currentTime;

    frameCount++;
    if (frameCount >= MS_AVARAGE_FRAMES) {
        printf("%fms\n", ms / (double)frameCount);
        frameCount = 0;
        ms = 0.0;
    }
}

void TestNoiseScreen::draw(const vui::GameTime& gameTime)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static float time = 0.0f;
    time += (float)delta * 0.0002f;
    m_program->use();
    glUniform1f(m_program->getUniform("unTime"), time);
    glUniform1f(m_program->getUniform("unAspectRatio"), m_game->getWindow().getAspectRatio());

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    m_program->unuse();
    checkGlError("TestNoiseScreen::draw");
}

void TestNoiseScreen::onNoiseChange()
{
    if (m_program != nullptr) delete m_program;
    m_program = ShaderLoader::createProgram("Noise", m_noiseTypes[m_currentNoise].vertexShader, m_noiseTypes[m_currentNoise].fragmentShader);
}